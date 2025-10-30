#pragma once
#include <memory>
#include <atomic>
#include <cstdint>

extern std::atomic<size_t> g_requests_processed;
extern std::atomic<size_t> g_ads_matched;
extern std::atomic<size_t> g_no_match_found;
extern std::atomic<uint64_t> g_total_latency_us;

struct AdIndex;

// Atomic snapshot handle declared here, defined in a single .cpp
extern std::shared_ptr<AdIndex> current_index;