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

    // Consider only ads in the same region for fairness
    std::vector<Ad*> candidates;
    auto it_reg = snap->region_to_ads.find(region);
    if (it_reg != snap->region_to_ads.end())
        candidates.insert(candidates.end(), it_reg->second.begin(), it_reg->second.end());
    if (candidates.empty()) return nullptr;

    // QualityScore = f(CTR, Relevance, LandingScore)
    // Use a simple weighted blend in [0,1]
    const double w_ctr = 0.5;
    const double w_rel = 0.4;
    const double w_land = 0.1;

    auto relevance_of = [&](Ad* ad) -> double {
        // Binary relevance for now: 1 if interest keyword appears, else 0
        for (const auto &kw : ad->keywords) if (kw == interest) return 1.0;
        return 0.0;
    };

    Ad* best = nullptr;
    double best_rank = -1.0;
    for (Ad* ad : candidates) {
        const double relevance = relevance_of(ad);
        const double quality = std::clamp(w_ctr*ad->ctr + w_rel*relevance + w_land*ad->landing_score, 0.0, 1.0);
        const double adrank = ad->bid * quality;
        if (adrank > best_rank) { best_rank = adrank; best = ad; }
    }
    return best;
}
