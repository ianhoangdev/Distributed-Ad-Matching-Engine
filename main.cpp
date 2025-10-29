#include <atomic>
#include <algorithm>
#include <iostream>
#include <memory>
#include <string>
#include <unordered_map>
#include <vector>

using namespace std;

struct Ad {
    int id;
    double bid;
    vector<string> keywords;
    string region;
};

struct AdIndex{
    vector<unique_ptr<Ad>> ads_storage;

    unordered_map<string, vector<Ad*>> keyword_to_ads;
    unordered_map<string, vector<Ad*>> region_to_ads;

    void build_indexes() {
        keyword_to_ads.clear();
        region_to_ads.clear();
        for (unique_ptr<Ad> &up : ads_storage) {
            Ad* a = up.get();
            for (string &kw : a->keywords) {
                keyword_to_ads[kw].push_back(a);
            }
            region_to_ads[a->region].push_back(a);
        }

        for (auto &pair : keyword_to_ads) {
            sort(pair.second.begin(), pair.second.end(), [](Ad* a, Ad* b){ return a->bid > b->bid; });
        }

        for (auto &pair : region_to_ads) {
            sort(pair.second.begin(), pair.second.end(), [](Ad* a, Ad* b){ return a->bid > b->bid; });
        }
    }
};

shared_ptr<AdIndex> current_index;

shared_ptr<AdIndex> build_sample_index() {
    shared_ptr<AdIndex> idx = make_shared<AdIndex>();
    
    idx->ads_storage.push_back(std::make_unique<Ad>(Ad{1, 1.20, {"sports","fitness"}, "US"}));
    idx->ads_storage.push_back(std::make_unique<Ad>(Ad{2, 2.50, {"finance","invest"}, "US"}));
    idx->ads_storage.push_back(std::make_unique<Ad>(Ad{3, 0.75, {"sports","soccer"}, "UK"}));
    idx->ads_storage.push_back(std::make_unique<Ad>(Ad{4, 3.10, {"fitness","wellness"}, "US"}));
    idx->ads_storage.push_back(std::make_unique<Ad>(Ad{5, 1.80, {"travel","adventure"}, "UK"}));

    idx->build_indexes();
    return idx;
}