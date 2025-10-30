#include <memory>
#include "../include/ad_index.h"
#include "../include/global.h"

// Single definition of the global snapshot pointer
std::shared_ptr<AdIndex> current_index;
std::atomic<size_t> g_requests_processed{0};
std::atomic<size_t> g_ads_matched{0};
std::atomic<size_t> g_no_match_found{0};
std::atomic<uint64_t> g_total_latency_us{0};

