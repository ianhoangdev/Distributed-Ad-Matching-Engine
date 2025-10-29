#include "../include/matcher.h"
#include "../include/data_gen.h"
#include <iostream>
#include <thread>
#include <chrono>
#include <random>

moodycamel::ConcurrentQueue<UserRequest> request_queue;

void matching_worker(int worker_id) {
    UserRequest req;

    while (true) {
        if (request_queue.try_dequeue(req)) {
            Ad* winner = match_simple(req.interests, req.region);

            if (winner) {
                std::cout << "[Worker " << worker_id << "] Request " << req.id
                          << " matched Ad id=" << winner->id << " bid=" << winner->bid << "\n";
            } else {
                std::cout << "[Worker " << worker_id << "] Request " << req.id << " no match\n";
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

        int num_interests = dist_num_interests(rng);
        std::unordered_set<std::string> unique_interests;
        while (unique_interests.size() < static_cast<size_t>(num_interests)) {
            unique_interests.insert(interests[dist_interest(rng)]);
        }
        
        req.interests.assign(unique_interests.begin(), unique_interests.end());
        
        request_queue.enqueue(req);
    }
}
