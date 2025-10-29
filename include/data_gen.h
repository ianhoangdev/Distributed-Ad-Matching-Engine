#pragma once
#include <vector>
#include <string>
#include <random>
#include <memory>
#include <sstream>
#include "ad.h"
#include "ad_index.h"

struct DataGenerationConfig {
    size_t num_ads = 10000;
    unsigned int seed = 12345u;
    int min_keywords_per_ad = 1;
    int max_keywords_per_ad = 3;
    double min_bid = 0.5;
    double max_bid = 5.0;
};

inline const std::vector<std::string>& interest_vocab() {
    static std::vector<std::string> vocab = {
        "fitness","sports","soccer","basketball","tennis","running","yoga","wellness",
        "travel","flights","hotels","adventure","beach","hiking","finance","invest",
        "credit","loans","banking","crypto","tech","gadgets","smartphone","laptop",
        "gaming","food","cooking","restaurants","coffee","vegan", "fashion","beauty",
        "health","cars","movies","music","dating","shopping","education","real estate"
    };
    return vocab;
}

inline const std::vector<std::string>& region_vocab() {
    static std::vector<std::string> regions = {"US","UK","CA","DE","FR","IN","JP","AU", "BR", "SG", "VN"};
    return regions;
}

inline std::string uuid_v4(std::mt19937 &rng) {
    std::uniform_int_distribution<uint32_t> dist32(0, 0xFFFFFFFFu);
    uint32_t d0 = dist32(rng);
    uint32_t d1 = dist32(rng);
    uint32_t d2 = dist32(rng);
    uint32_t d3 = dist32(rng);

    // Set version (4) and variant (10)
    d1 = (d1 & 0xFFFF0FFFu) | 0x00004000u; // version 4 in time_hi_and_version
    d2 = (d2 & 0x3FFFFFFFu) | 0x80000000u; // variant 10 in clock_seq_hi_and_reserved

    std::ostringstream oss;
    oss.setf(std::ios::hex, std::ios::basefield);
    oss.fill('0');
    oss.width(8); oss << (d0);
    oss << "-";
    oss.width(4); oss << ((d1 >> 16) & 0xFFFFu);
    oss << "-";
    oss.width(4); oss << (d1 & 0xFFFFu);
    oss << "-";
    oss.width(4); oss << ((d2 >> 16) & 0xFFFFu);
    oss << "-";
    oss.width(4); oss << (d2 & 0xFFFFu);
    oss.width(8); oss << (d3);
    return oss.str();
}

inline std::shared_ptr<Ad> generate_random_ad(std::mt19937 &rng, const DataGenerationConfig &cfg = {}) {
    std::uniform_real_distribution<double> bid_dist(cfg.min_bid, cfg.max_bid);
    std::uniform_int_distribution<int> kw_count_dist(cfg.min_keywords_per_ad, cfg.max_keywords_per_ad);
    const auto &iv = interest_vocab();
    const auto &rv = region_vocab();
    std::uniform_int_distribution<size_t> kw_pick(0, iv.size() - 1);
    std::uniform_int_distribution<size_t> reg_pick(0, rv.size() - 1);

    Ad ad;
    ad.id = uuid_v4(rng);
    ad.bid = bid_dist(rng);
    ad.region = rv[reg_pick(rng)];

    int k = kw_count_dist(rng);
    ad.keywords.reserve(static_cast<size_t>(k));
    for (int i = 0; i < k; ++i) {
        ad.keywords.push_back(iv[kw_pick(rng)]);
    }

    return std::make_shared<Ad>(ad);
}

inline std::shared_ptr<AdIndex> build_sample_index(const DataGenerationConfig &cfg) {
    auto idx = std::make_shared<AdIndex>();
    std::mt19937 rng(cfg.seed);
    idx->ads_storage.reserve(cfg.num_ads);
    for (size_t i = 0; i < cfg.num_ads; ++i) {
        idx->ads_storage.push_back(generate_random_ad(rng, cfg));
    }
    idx->build_indexes();
    return idx;
}