#pragma once
#include <string>
#include "ad_index.h"
#include "global.h"
#include "../third_party/ConcurrentQueue.h"

struct UserRequest {
    int id;
    std::string interest;
    std::string region;
};

extern moodycamel::ConcurrentQueue<UserRequest> request_queue;

void matching_worker(int worker_id);
void user_generator();

inline Ad* match_simple(const std::string &interest, const std::string &region) {
    auto snap = std::atomic_load(&current_index);
    if (!snap) return nullptr;

    std::vector<Ad*> candidates;
    auto it_kw = snap->keyword_to_ads.find(interest);
    if (it_kw != snap->keyword_to_ads.end())
        candidates.insert(candidates.end(), it_kw->second.begin(), it_kw->second.end());

    auto it_reg = snap->region_to_ads.find(region);
    if (it_reg != snap->region_to_ads.end())
        candidates.insert(candidates.end(), it_reg->second.begin(), it_reg->second.end());

    if (candidates.empty()) return nullptr;

    return *std::max_element(candidates.begin(), candidates.end(),
                             [](Ad* a, Ad* b){ return a->bid < b->bid; });
}
