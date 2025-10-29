#pragma once
#include <thread>
#include <chrono>
#include <memory>
#include <atomic>
#include <iostream>
#include <unordered_set>
#include <limits>
#include "ad_index.h"
#include "data_gen.h"

extern std::shared_ptr<AdIndex> current_index;

class AdUpdater {
public:
    AdUpdater(int /*start_id*/ = 100, int update_interval_sec = 5)
        : interval(update_interval_sec) {}

    void start() {
        updater_thread = std::thread(&AdUpdater::run, this);
        updater_thread.detach();
    }

private:
    int interval;
    std::thread updater_thread;
    std::mt19937 rng{987654u};
    DataGenerationConfig cfg{};
    std::unordered_set<std::string> used_ids; // tracks IDs to avoid collisions (paranoia)

    void run() {
        if (auto snap = std::atomic_load(&current_index)) {
            for (const auto &ad_ptr : snap->ads_storage) {
                if (ad_ptr) used_ids.insert(ad_ptr->id);
            }
        }

        while (true) {
            std::this_thread::sleep_for(std::chrono::seconds(interval));

            auto old_snapshot = std::atomic_load(&current_index);
            auto new_snapshot = std::make_shared<AdIndex>(*old_snapshot);

            // Generate UUIDv4; in the extremely unlikely event of collision, retry
            std::shared_ptr<Ad> new_ad;
            do { new_ad = generate_random_ad(rng, cfg); } while (used_ids.find(new_ad->id) != used_ids.end());
            used_ids.insert(new_ad->id);
            new_snapshot->ads_storage.push_back(new_ad);

            new_snapshot->build_indexes();
            std::atomic_store(&current_index, new_snapshot);

            std::cout << "[Updater] Published new snapshot with Ad ID=" << new_ad->id << "\n";
        }
    }
};
