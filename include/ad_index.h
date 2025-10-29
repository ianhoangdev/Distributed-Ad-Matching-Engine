#pragma once
#include <vector>
#include <unordered_map>
#include <memory>
#include <algorithm>
#include "ad.h"

struct AdIndex {
    std::vector<std::shared_ptr<Ad>> ads_storage;
    
    std::unordered_map<std::string, std::vector<Ad*>> keyword_to_ads;
    std::unordered_map<std::string, std::vector<Ad*>> region_to_ads;

    void build_indexes() {
        keyword_to_ads.clear();
        region_to_ads.clear();

        for (auto &ad_ptr : ads_storage) {
            Ad* a = ad_ptr.get();
            for (auto &kw : a->keywords)
                keyword_to_ads[kw].push_back(a);
            region_to_ads[a->region].push_back(a);
        }

        for (auto &p : keyword_to_ads)
            std::sort(p.second.begin(), p.second.end(),
                      [](Ad* a, Ad* b){ return a->bid > b->bid; });
        for (auto &p : region_to_ads)
            std::sort(p.second.begin(), p.second.end(),
                      [](Ad* a, Ad* b){ return a->bid > b->bid; });
    }
};
