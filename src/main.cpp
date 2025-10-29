#include <iostream>
#include <thread>
#include "../include/global.h"
#include "../include/ad.h"
#include "../include/ad_index.h"
#include "../include/ad_updater.h"
#include "../include/matcher.h"


extern std::shared_ptr<AdIndex> current_index;

std::shared_ptr<AdIndex> build_sample_index() {
    auto idx = std::make_shared<AdIndex>();
    idx->ads_storage.push_back(std::make_shared<Ad>(Ad{1, 1.2, {"sports","fitness"}, "US"}));
    idx->ads_storage.push_back(std::make_shared<Ad>(Ad{2, 2.5, {"finance","invest"}, "US"}));
    idx->ads_storage.push_back(std::make_shared<Ad>(Ad{3, 0.75, {"sports","soccer"}, "UK"}));
    idx->ads_storage.push_back(std::make_shared<Ad>(Ad{4, 3.1, {"fitness","wellness"}, "US"}));
    idx->ads_storage.push_back(std::make_shared<Ad>(Ad{5, 1.8, {"travel","adventure"}, "UK"}));
    // extra categories/regions
    idx->ads_storage.push_back(std::make_shared<Ad>(Ad{6, 2.2, {"tech","gadgets"}, "US"}));
    idx->ads_storage.push_back(std::make_shared<Ad>(Ad{7, 1.1, {"food","cooking"}, "UK"}));
    idx->ads_storage.push_back(std::make_shared<Ad>(Ad{8, 2.9, {"travel","flights"}, "CA"}));
    idx->ads_storage.push_back(std::make_shared<Ad>(Ad{9, 1.6, {"finance","credit"}, "CA"}));
    idx->ads_storage.push_back(std::make_shared<Ad>(Ad{10, 3.4, {"sports","basketball"}, "US"}));
    idx->build_indexes();
    return idx;
}

int main() {

    auto initial = build_sample_index();
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