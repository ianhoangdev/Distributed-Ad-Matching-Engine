#include "../include/matcher.h"
#include <iostream>
#include <thread>
#include <chrono>
#include <random>

moodycamel::ConcurrentQueue<UserRequest> request_queue;

void matching_worker(int worker_id) {
    UserRequest req;

    while (true) {
        if (request_queue.try_dequeue(req)) {
            Ad* winner = match_simple(req.interest, req.region);

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
    std::vector<std::string> interests = {"fitness","sports","travel"};
    std::vector<std::string> regions = {"US","UK"};

    // Deterministic RNG for reproducible workloads
    static std::mt19937 rng(123456);
    std::uniform_int_distribution<size_t> dist_interest(0, interests.size() - 1);
    std::uniform_int_distribution<size_t> dist_region(0, regions.size() - 1);

    while (true) {
        UserRequest req;
        req.id = request_id++;
        req.interest = interests[dist_interest(rng)];
        req.region = regions[dist_region(rng)];
        request_queue.enqueue(req);

        std::this_thread::sleep_for(std::chrono::milliseconds(50));
    }
}
