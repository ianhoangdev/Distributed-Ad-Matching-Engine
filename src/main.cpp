#include <iostream>
#include <thread>
#include <iomanip>
#include <chrono>
#include <atomic>
#include "../include/global.h"
#include "../include/ad_index.h"
#include "../include/ad_updater.h"
#include "../include/matcher.h"
#include "../include/data_gen.h"

extern std::shared_ptr<AdIndex> current_index;
extern moodycamel::ConcurrentQueue<UserRequest> request_queue;

static DataGenerationConfig kDefaultGenCfg{10000, 12345u, 1, 3, 0.5, 5.0};


struct comma_numpunct : std::numpunct<char> {
protected:
    char do_thousands_sep() const override { return ','; }
    std::string do_grouping() const override { return "\3"; }
};

void metrics_printer() {
    size_t last_reqs = 0;
    std::cout.imbue(std::locale(std::cout.getloc(), new comma_numpunct));

    while (true) {
        std::this_thread::sleep_for(std::chrono::seconds(3));

        size_t current_reqs = g_requests_processed.load(std::memory_order_relaxed);
        size_t reqs_per_sec = (current_reqs - last_reqs) / 2;
        last_reqs = current_reqs;

        auto snap = std::atomic_load(&current_index);
        size_t total_ads = snap ? snap->ads_storage.size() : 0;
        size_t q_size = request_queue.size_approx();
        uint64_t total_lat = g_total_latency_us.load(std::memory_order_relaxed);
        uint64_t avg_lat_us = (current_reqs == 0) ? 0 : (total_lat / current_reqs);

        // Clear screen
        std::cout << "\033[2J\033[H";
        std::cout << "\n";
        std::cout << "==================== METRICS ====================\n";
        std::cout << std::setw(25) << std::left << "QPS (approx):"       << reqs_per_sec << "\n";
        std::cout << std::setw(25) << std::left << "Avg Latency (us):"  << avg_lat_us << "\n";
        std::cout << std::setw(25) << std::left << "Queue Size:"         << q_size << "\n";
        std::cout << std::setw(25) << std::left << "Total Ads:"          << total_ads << "\n";
        std::cout << std::setw(25) << std::left << "Total Requests:"     << current_reqs << "\n";
        std::cout << std::setw(25) << std::left << "Total Matched:"      << g_ads_matched.load(std::memory_order_relaxed) << "\n";
        std::cout << std::setw(25) << std::left << "Total No Match:"     << g_no_match_found.load(std::memory_order_relaxed) << "\n";
        std::cout << "=================================================\n";
    }
}

int main() {
    auto initial = build_sample_index(kDefaultGenCfg);
    std::atomic_store(&current_index, initial);
    std::cout << "[Main] Initial snapshot published.\n";

    AdUpdater updater;
    updater.start();

    const int num_workers = 4;
    std::vector<std::thread> workers;
    for (int i = 0; i < num_workers; ++i)
        workers.emplace_back(matching_worker);

    const int num_producers = 4;
    std::vector<std::thread> producers;

    for (int i = 0; i < num_producers; ++i)
        producers.emplace_back(user_generator);

    std::thread metrics_thread(metrics_printer);

    for (auto &w : workers) w.join();
    for (auto &p : producers) p.join();

    return 0;
}
