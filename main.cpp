#include <atomic>
#include <algorithm>
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
    vector<unique_ptr<Ad> > ads_storage;

    unordered_map<string, vector<Ad*> > keyword_to_ads;
    unordered_map<string, vector<Ad*> > region_to_ads;
};