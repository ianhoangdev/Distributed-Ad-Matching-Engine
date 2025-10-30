# Distributed Concurrent Ad Matching Engine

This project is a C++ simulation of a high-performance, in-memory ad matching service. I want to demonstrate a high-throughput, multi-threaded, and locking-free architecture designed to serve ad requests at high QPS with low latency, complete with live performance monitoring.

The core of the project is the concurrency model: matching threads (readers) operate on an immutable, atomic snapshot of the ad index, allowing them to work completely lock-free. A background thread (writer) periodically builds a new index and atomically swaps it in, ensuring reads are never blocked by writes.

## Key Features 
* **Lock-Free Concurrency:** Readers (matchers) and the writer (updater) are fully decoupled. Readers operate on an atomic `std::shared_ptr` snapshot of the ad index, ensuring zero read contention.
* **High-Performance Queuing:** Uses `moodycamel::ConcurrentQueue`, a fast, lock-free MPMC queue, to manage the flow of user requests from producers to consumer (matching) threads.
* **AdRank Logic:** Implements a non-trivial matching algorithm similar to Google's that finds candidate ads by region and interest, then scores them using a weighted AdRank formula: `rank = bid * quality`, where `quality = f(ctr, relevance, landing_score)`.

## Core Architecture: The Atomic Snapshot
**High Level Design**:
![high_level_design](https://github.com/user-attachments/assets/b5ed2590-0341-4ede-bde6-8ca569790ea3)
The system uses RCU-style (Read-Copy-Update) concurrency model, managed by a single global: `std::atomic<std::shared_ptr<AdIndex>> current_index;`

**Readers (Matching Workers):**
1.  A worker thread fetches its own `shared_ptr` to the current index:
    `auto snap = std::atomic_load(&current_index);`
2.  It then operates on this **immutable snapshot** (`snap`) for the entire duration of the request.
3.  This operation is lock-free and scales horizontally with the number of reader threads.

**Writer (Ad Updater):**
1.  The updater thread wakes up and gets a pointer to the *current* index, just like a reader.
2.  It creates a **deep copy** of this index:
    `auto new_snapshot = std::make_shared<AdIndex>(*old_snapshot);`
3.  It modifies this **private copy** by adding new ads and rebuilding the internal inverted indexes.
4.  Once the `new_snapshot` is fully ready, it atomically publishes it:
    `std::atomic_store(&current_index, new_snapshot);`

This pattern guarantees that readers are never blocked and always operate on a 100% consistent version of the index.


## How to Build and Run
```bash
# Build the project
make
# Run the project
./ad_matching_engine
```
The application will start, build the initial index, and begin printing metrics to the console:
<img width="589" height="152" alt="Screenshot 2025-10-29 at 11 20 02 PM" src="https://github.com/user-attachments/assets/1693b700-1ee9-4fb2-af75-74993e6bd82c" />

