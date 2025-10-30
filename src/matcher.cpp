#include "../include/matcher.h"
#include "../include/data_gen.h"
#include <iostream>
#include <thread>
#include <chrono>
#include <random>

moodycamel::ConcurrentQueue<UserRequest> request_queue;

void matching_worker() {
    UserRequest req;

    while (true) {
        if (request_queue.try_dequeue(req)) {
            auto end_time = std::chrono::high_resolution_clock::now();
            auto duration = end_time - req.start_time;
            auto us = std::chrono::duration_cast<std::chrono::microseconds>(duration).count();
            g_total_latency_us.fetch_add(us, std::memory_order_relaxed);

            Ad* winner = match_simple(req.interests, req.region);
            g_requests_processed.fetch_add(1, std::memory_order_relaxed);
            if (winner) {
                g_ads_matched.fetch_add(1, std::memory_order_relaxed);
            } else {
                g_no_match_found.fetch_add(1, std::memory_order_relaxed);
            }
        } else {
            std::this_thread::sleep_for(std::chrono::milliseconds(1)); // avoid busy loop
        }
    }
}

void user_generator() {
    int request_id = 0;
    const auto &interests = interest_vocab();
    const auto &regions = region_vocab();

    static std::mt19937 rng(123456);
    std::uniform_int_distribution<size_t> dist_interest(0, interests.size() - 1);
    std::uniform_int_distribution<size_t> dist_region(0, regions.size() - 1);
    std::uniform_int_distribution<int> dist_num_interests(1, 3);

    while (true) {
        UserRequest req;
        req.id = request_id++;
        req.region = regions[dist_region(rng)];
        req.start_time = std::chrono::high_resolution_clock::now();

        int num_interests = dist_num_interests(rng);
        std::unordered_set<std::string> unique_interests;
        while (unique_interests.size() < static_cast<size_t>(num_interests)) {
            unique_interests.insert(interests[dist_interest(rng)]);
        }
        
        req.interests.assign(unique_interests.begin(), unique_interests.end());
        
        request_queue.enqueue(req);
    }
}