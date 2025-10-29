#pragma once
#include <thread>
#include <chrono>
#include <memory>
#include <atomic>
#include <iostream>
#include "ad_index.h"
#include "data_gen.h"

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
    std::mt19937 rng{987654u};
    std::uniform_int_distribution<int> kw_count_dist{1, 3};
    std::uniform_real_distribution<double> bid_dist{0.5, 5.0};

    void run() {
        while (true) {
            std::this_thread::sleep_for(std::chrono::seconds(interval));

            auto old_snapshot = std::atomic_load(&current_index);
            auto new_snapshot = std::make_shared<AdIndex>(*old_snapshot);

            // Generate a varied new ad
            Ad new_ad;
            new_ad.id = next_ad_id++;
            new_ad.bid = bid_dist(rng);
            const auto &iv = interest_vocab();
            const auto &rv = region_vocab();
            std::uniform_int_distribution<size_t> kw_pick(0, iv.size() - 1);
            std::uniform_int_distribution<size_t> reg_pick(0, rv.size() - 1);

            int k = kw_count_dist(rng);
            new_ad.keywords.reserve(static_cast<size_t>(k));
            for (int i = 0; i < k; ++i) new_ad.keywords.push_back(iv[kw_pick(rng)]);
            new_ad.region = rv[reg_pick(rng)];

            new_snapshot->ads_storage.push_back(std::make_shared<Ad>(new_ad));

            new_snapshot->build_indexes();
            std::atomic_store(&current_index, new_snapshot);

            std::cout << "[Updater] Published new snapshot with Ad ID=" << next_ad_id-1 << "\n";
        }
    }
};
