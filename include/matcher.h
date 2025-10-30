#pragma once
#include <string>
#include "ad_index.h"
#include "global.h"
#include <chrono>
#include <algorithm>
#include "../third_party/ConcurrentQueue.h"
#include <unordered_set>

struct UserRequest {
    int id;
    std::vector<std::string> interests;
    std::string region;
    std::chrono::high_resolution_clock::time_point start_time;
};

extern moodycamel::ConcurrentQueue<UserRequest> request_queue;

void matching_worker();
void user_generator();

inline Ad* match_simple(const std::vector<std::string> &interests, const std::string &region) {
    auto snap = std::atomic_load(&current_index);
    if (!snap) return nullptr;

    // Consider only ads in the same region for fairness
    auto it_reg = snap->region_to_ads.find(region);
    if (it_reg == snap->region_to_ads.end()) {
        return nullptr;
    }

    std::unordered_set<Ad*> region_matches(it_reg->second.begin(), it_reg->second.end());
    if (region_matches.empty()) {
        return nullptr;
    }

    std::unordered_set<Ad*> keyword_matches;
    for (const auto& interest : interests) {
        auto it_kw = snap->keyword_to_ads.find(interest);
        if (it_kw != snap->keyword_to_ads.end()) {
            keyword_matches.insert(it_kw->second.begin(), it_kw->second.end());
        }
    }

    if (keyword_matches.empty()) {
        return nullptr;
    }

    std::vector<Ad*> final_candidates;

    if (region_matches.size() < keyword_matches.size()) {
        for (Ad* ad : region_matches) {
            if (keyword_matches.count(ad)) {
                final_candidates.push_back(ad);
            }
        }
    } else {
        for (Ad* ad : keyword_matches) {
            if (region_matches.count(ad)) {
                final_candidates.push_back(ad);
            }
        }
    }

    if (final_candidates.empty()) {
        return nullptr;
    }

    // QualityScore = f(CTR, Relevance, LandingScore)
    const double w_ctr = 0.5;
    const double w_rel = 0.4;
    const double w_land = 0.1;

    std::unordered_set<std::string> user_interest_set(interests.begin(), interests.end());

    auto relevance_of = [&](Ad* ad) -> double {
        if (ad->keywords.empty()) {
            return 0.0;
        }
        double matches = 0.0;
        for (const auto &ad_kw : ad->keywords) {
            if (user_interest_set.count(ad_kw)) {
                matches += 1.0;
            }
        }
        return matches / static_cast<double>(ad->keywords.size());
    };

    Ad* best = nullptr;
    double best_rank = -1.0;
    
    for (Ad* ad : final_candidates) {
        const double relevance = relevance_of(ad);
        const double quality = std::clamp(w_ctr*ad->ctr + w_rel*relevance + w_land*ad->landing_score, 0.0, 1.0);
        const double adrank = ad->bid * quality;
        if (adrank > best_rank) {
            best_rank = adrank;
            best = ad;
        }
    }
    return best;
}
