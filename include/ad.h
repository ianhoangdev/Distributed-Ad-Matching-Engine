#pragma once
#include <string>
#include <vector>

struct Ad {
    std::string id;
    double bid;
    double ctr;
    double landing_score;
    std::vector<std::string> keywords;
    std::string region;
};