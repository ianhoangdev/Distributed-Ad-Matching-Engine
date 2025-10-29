#pragma once
#include <string>
#include <vector>

struct Ad {
    std::string id;
    double bid;
    std::vector<std::string> keywords;
    std::string region;
};