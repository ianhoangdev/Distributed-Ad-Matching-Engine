#include <iostream>
#include <thread>
#include "../include/global.h"
#include "../include/ad_index.h"
#include "../include/ad_updater.h"
#include "../include/matcher.h"
#include "../include/data_gen.h"


extern std::shared_ptr<AdIndex> current_index;

static DataGenerationConfig kDefaultGenCfg{10000, 12345u, 1, 3, 0.5, 5.0};

void metrics_printer() {
    size_t last_reqs = 0;
    while (true) {
        std::this_thread::sleep_for(std::chrono::seconds(2));
        
        size_t current_reqs = g_requests_processed.load(std::memory_order_relaxed);
        size_t reqs_per_sec = (current_reqs - last_reqs) / 2;
        last_reqs = current_reqs;
        
        std::cout << "\n--- METRICS (2s) ---\n"
                  << "QPS (approx):   " << reqs_per_sec << "\n"
                  << "Total Requests: " << current_reqs << "\n"
                  << "Total Matched:  " << g_ads_matched.load(std::memory_order_relaxed) << "\n"
                  << "Total No Match: " << g_no_match_found.load(std::memory_order_relaxed) << "\n---\n\n";
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
