#include <atomic>
#include <algorithm>
#include <chrono>
#include <cstddef>
#include <iostream>
#include <memory>
#include <string>
#include <thread>
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
    vector<shared_ptr<Ad>> ads_storage;

    unordered_map<string, vector<Ad*>> keyword_to_ads;
    unordered_map<string, vector<Ad*>> region_to_ads;

    void build_indexes() {
        keyword_to_ads.clear();
        region_to_ads.clear();
        for (shared_ptr<Ad> &ad_ptr : ads_storage) {
            Ad* a = ad_ptr.get();
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
    
    idx->ads_storage.push_back(std::make_shared<Ad>(Ad{1, 1.20, {"sports","fitness"}, "US"}));
    idx->ads_storage.push_back(std::make_shared<Ad>(Ad{2, 2.50, {"finance","invest"}, "US"}));
    idx->ads_storage.push_back(std::make_shared<Ad>(Ad{3, 0.75, {"sports","soccer"}, "UK"}));
    idx->ads_storage.push_back(std::make_shared<Ad>(Ad{4, 3.10, {"fitness","wellness"}, "US"}));
    idx->ads_storage.push_back(std::make_shared<Ad>(Ad{5, 1.80, {"travel","adventure"}, "UK"}));

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

void periodic_update() {
    int counter = 100;
    while (true) {
        this_thread::sleep_for(chrono::seconds(5));

        shared_ptr<AdIndex> old_snapshot = atomic_load(&current_index);
        shared_ptr<AdIndex> new_snapshot = make_shared<AdIndex>(*old_snapshot);

        new_snapshot->ads_storage.push_back(
            make_shared<Ad>(Ad{counter++, 2.0, {"fitness","health"}, "US"})
        );

        new_snapshot->build_indexes();
        atomic_store(&current_index, new_snapshot);

        cout << "Published updated snapshot with new ad id=" << counter-1 << "\n";
    }
}

int main() {
    shared_ptr<AdIndex> initial = build_sample_index();

    atomic_store(&current_index, initial);
    cout << "Published initial AdIndex snapshot.\n";

    thread updater(periodic_update);

    for (int i = 0; i < 5; ++i) {
        this_thread::sleep_for(std::chrono::seconds(3));
        string interest = "fitness";
        string region = "US";
        Ad* winner = match_simple(interest, region);
        if (winner) {
            cout << "Matched Ad id=" << winner->id << " bid=" << winner->bid << "\n";
        } else {
            cout << "No matching ad found.\n";
        }
    }

    updater.detach(); // let it run in background
    return 0;
}