#include <iostream>
#include <thread>
#include "../include/global.h"
#include "../include/ad.h"
#include "../include/ad_index.h"
#include "../include/ad_updater.h"

extern std::shared_ptr<AdIndex> current_index;

std::shared_ptr<AdIndex> build_sample_index() {
    auto idx = std::make_shared<AdIndex>();
    idx->ads_storage.push_back(std::make_shared<Ad>(Ad{1, 1.2, {"sports","fitness"}, "US"}));
    idx->ads_storage.push_back(std::make_shared<Ad>(Ad{2, 2.5, {"finance","invest"}, "US"}));
    idx->ads_storage.push_back(std::make_shared<Ad>(Ad{3, 0.75, {"sports","soccer"}, "UK"}));
    idx->ads_storage.push_back(std::make_shared<Ad>(Ad{4, 3.1, {"fitness","wellness"}, "US"}));
    idx->ads_storage.push_back(std::make_shared<Ad>(Ad{5, 1.8, {"travel","adventure"}, "UK"}));
    idx->build_indexes();
    return idx;
}

int main() {
    auto initial = build_sample_index();
    std::atomic_store(&current_index, initial);
    std::cout << "[Main] Initial snapshot published.\n";

    AdUpdater updater;
    updater.start();

    std::this_thread::sleep_for(std::chrono::seconds(30));
    return 0;
}