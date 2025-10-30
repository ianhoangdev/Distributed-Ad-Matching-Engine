# Distributed Concurrent Ad Matching Engine

This project is a C++ simulation of a high-performance, in-memory ad matching service. I want to demonstrate a high-throughput, multi-threaded, and locking-free architecture designed to serve ad requests at high QPS with low latency, complete with live performance monitoring.

The core of the project is the concurrency model: matching threads (readers) operate on an immutable, atomic snapshot of the ad index, allowing them to work completely lock-free. A background thread (writer) periodically builds a new index and atomically swaps it in, ensuring reads are never blocked by writes.

## Key Features 
* **Lock-Free Concurrency:** Readers (matchers) and the writer (updater) are fully decoupled. Readers operate on an atomic `std::shared_ptr` snapshot of the ad index, ensuring zero read contention.
* **High-Performance Queuing:** Uses `moodycamel::ConcurrentQueue`, a fast, lock-free MPMC queue, to manage the flow of user requests from producers to consumer (matching) threads.
* **Realistic AdRank Logic:** Implements a non-trivial matching algorithm similar to Google's that finds candidate ads by region and interest, then scores them using a weighted AdRank formula: `rank = bid * quality`, where `quality = f(ctr, relevance, landing_score)`.
