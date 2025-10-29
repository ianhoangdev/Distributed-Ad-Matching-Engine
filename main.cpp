#include <atomic>
#include <algorithm>
#include <cstddef>
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

Ad* match_simple(const string &interest, const string &region) {
    shared_ptr<AdIndex> snap = atomic_load(&current_index);
    if (!snap) return nullptr;
    
    vector<Ad*> candidates;

    auto it_kw = snap->keyword_to_ads.find(interest);
    if (it_kw != snap->keyword_to_ads.end()) {
        candidates.insert(candidates.end(), it_kw->second.begin(), it_kw->second.end());
    }
    auto it_reg = snap->region_to_ads.find(region);
    if (it_reg != snap->region_to_ads.end()) {
        candidates.insert(candidates.end(), it_reg->second.begin(), it_reg->second.end());
    }

    if (candidates.empty()) return nullptr;

    Ad* best_ad = *max_element(candidates.begin(), candidates.end(), [](Ad* a, Ad* b){
        return a->bid < b->bid;
    });
    
    return best_ad;
}