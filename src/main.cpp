#include <iostream>
#include <thread>
#include "../include/global.h"
#include "../include/ad_index.h"
#include "../include/ad_updater.h"
#include "../include/matcher.h"
#include "../include/data_gen.h"


extern std::shared_ptr<AdIndex> current_index;

static DataGenerationConfig kDefaultGenCfg{10000, 12345u, 1, 3, 0.5, 5.0};

int main() {

    auto initial = build_sample_index(kDefaultGenCfg);
    std::atomic_store(&current_index, initial);
    std::cout << "[Main] Initial snapshot published.\n";

    AdUpdater updater;
    updater.start();

    const int num_workers = 4;
    std::vector<std::thread> workers;
    for (int i = 0; i < num_workers; ++i)
        workers.emplace_back(matching_worker, i);

    std::thread producer(user_generator);

    std::this_thread::sleep_for(std::chrono::seconds(30));

    producer.detach();
    for (auto &w : workers) w.detach();

    return 0;
}
