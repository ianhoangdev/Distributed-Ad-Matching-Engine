#pragma once
#include <string>
#include <vector>

struct Ad {
    int id;
    double bid;
    std::vector<std::string> keywords;
    std::string region;
};