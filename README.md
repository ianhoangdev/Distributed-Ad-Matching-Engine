# Distributed Concurrent Ad Matching Engine

This project is a C++ simulation of a high-performance, in-memory ad matching service. It demonstrates a high-throughput, multi-threaded, and locking-free architecture designed to serve ad requests at high QPS with low latency, complete with live performance monitoring.

The core of the project is its concurrency model: matching threads (readers) operate on an immutable, atomic snapshot of the ad index, allowing them to work completely lock-free. A background thread (writer) periodically builds a new index and atomically swaps it in, ensuring reads are never blocked by writes.
