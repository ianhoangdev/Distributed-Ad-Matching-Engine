#pragma once
#include <memory>
#include <atomic>
// ...
extern std::atomic<size_t> g_requests_processed;
extern std::atomic<size_t> g_ads_matched;
extern std::atomic<size_t> g_no_match_found;
struct AdIndex;

// Atomic snapshot handle declared here, defined in a single .cpp
extern std::shared_ptr<AdIndex> current_index;