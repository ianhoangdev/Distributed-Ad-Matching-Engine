#pragma once
#include <thread>
#include <chrono>
#include <memory>
#include <atomic>
#include <iostream>
#include "ad_index.h"

extern std::shared_ptr<AdIndex> current_index;

class AdUpdater {
public:
    AdUpdater(int start_id = 100, int update_interval_sec = 5)
        : next_ad_id(start_id), interval(update_interval_sec) {}

    void start() {
        updater_thread = std::thread(&AdUpdater::run, this);
        updater_thread.detach();
    }

private:
    int next_ad_id;
    int interval;
    std::thread updater_thread;

    void run() {
        while (true) {
            std::this_thread::sleep_for(std::chrono::seconds(interval));

            auto old_snapshot = std::atomic_load(&current_index);
            auto new_snapshot = std::make_shared<AdIndex>(*old_snapshot);

            new_snapshot->ads_storage.push_back(
                std::make_shared<Ad>(
                    Ad{next_ad_id++, 2.0, {"fitness","health"}, "US"}
                )
            );

            new_snapshot->build_indexes();
            std::atomic_store(&current_index, new_snapshot);

            std::cout << "[Updater] Published new snapshot with Ad ID=" << next_ad_id-1 << "\n";
        }
    }
};
