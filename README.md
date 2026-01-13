# Lock-Free Telemetry Pipeline (Linux, C)
A high-performance lock-free telemetry system implemented in C using C11 atomics.
The project focuses on low latency, high throughput, and tail-latency analysis, with a
direct comparison against a mutex-based queue.
---
## Features
- Lock-free SPSC (Single Producer Single Consumer) ring buffer
- Zero dynamic allocations in the hot path
- Explicit C11 atomic memory ordering
- CPU-pinned producer and consumer threads
- Average, P99, and max latency measurement
- Fair comparison with mutex-based queue
- Linux-focused (tested on WSL2)
---
## Design Overview
### Telemetry Event Format
```c
typedef struct {
 uint64_t timestamp_ns;
 uint32_t producer_id;
 uint16_t type;
 uint16_t reserved;
 uint64_t value;
} telemetry_event_t;
```
### Lock-Free Ring Buffer Design
- Circular buffer with power-of-two size
- Atomic head and tail indices
- Memory ordering:
 - relaxed for local index updates
 - acquire for cross-thread reads
 - release for publishing updates
- No locks, no syscalls, no blocking
---
## Benchmark Methodology
- Single producer, single consumer
- Threads pinned to separate CPUs
- Nanosecond-resolution timestamps using CLOCK_MONOTONIC_RAW
- Identical workload for lock-free and mutex implementations
- Metrics collected:
 - Average latency
 - P99 latency
 - Maximum latency
- Throughput (events/sec)
---
## Benchmark Results (WSL2, CPU-Pinned)

### Lock-Free SPSC Ring Buffer
- Average latency: ~47 µs
- P99 latency: ~91 µs
- Max latency: ~392 µs
- Throughput: ~20–27 million events/sec

### Mutex-Based Queue
- Average latency: ~64 µs (best case)
- P99 latency: ~388 µs
- Max latency: ~667 µs
- Throughput: ~4.2–4.9 million events/sec

### Key Observations
- Lock-free achieved ~5× higher throughput
- Lock-free showed ~4× better P99 latency
- Mutex average latency can appear acceptable
- Mutex tail latency remains significantly worse due to kernel scheduling and lock contention
---
## Build & Run

### Lock-Free Benchmark
```bash
gcc -O2 -std=c11 -pthread -Iinclude src/ring_buffer.c benchmarks/bench_spsc.c -o bench_spsc
./bench_spsc
```
### Mutex Benchmark
```bash
gcc -O2 -std=c11 -pthread -Iinclude src/mutex_ring_buffer.c benchmarks/bench_mutex.c -o bench_mu./bench_mutex
```
---
## Why This Project Matters
This project demonstrates:
- Correct lock-free programming
- Understanding of memory models
- Tail-latency awareness (P99 > average)
- OS scheduling effects on performance
- Data-driven comparison of synchronization strategies
---
## Future Work
- MPMC lock-free ring buffer
- eBPF-based kernel telemetry producer
- CPU isolation and real-time scheduling
- CSV export and plotting
## Resume Bullet
> Built a lock-free telemetry pipeline in C using C11 atomics; benchmarked avg/P99 latency and achieved ~5× higher throughput and ~4× better P99 latency compared to a mutex-based queue (Linux/WSL).
