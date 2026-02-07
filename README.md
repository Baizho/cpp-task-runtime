# cpp-task-runtime

A high-performance **work-stealing task runtime** written in modern C++.

This project implements a lightweight execution engine capable of efficiently running large numbers of asynchronous tasks across multiple CPU cores. It includes futures, parallel algorithms, and performance instrumentation, and is designed to resemble the core scheduling infrastructure used in real-world systems.

---

## Overview

Modern applications frequently need to execute millions of small, independent tasks efficiently. Examples include:

* Financial risk and pricing computations
* Market data processing pipelines
* Game engines and simulation systems
* Databases and query execution engines
* Machine learning workloads

Naive thread pools often suffer from:

* Lock contention on a global queue
* Poor CPU utilization
* Load imbalance between threads

This project implements a **work-stealing scheduler**, a widely used industry solution (similar to Intel TBB, Cilk, Java ForkJoinPool, and Rust Rayon).

Each worker thread maintains its own task queue and executes tasks locally. When a worker becomes idle, it dynamically steals work from other threads, providing efficient load balancing with minimal contention.

---

## Architecture

### High-Level Design

```
User Code
    │
    ▼
Parallel Algorithms Layer
(parallel_for, parallel_reduce)
    │
    ▼
ThreadPool API
(submit, futures)
    │
    ▼
Scheduler
(work distribution)
    │
    ▼
Work-Stealing Queues
(per worker)
    │
    ▼
Worker Threads
(execution)
```

---

### Execution Flow

1. User submits a task to the `ThreadPool`
2. Task is placed into a worker's local queue
3. Worker executes tasks from its own queue (fast path)
4. If the worker becomes idle:

   * It attempts to **steal** work from other workers
5. This strategy:

   * Minimizes locking
   * Balances uneven workloads
   * Scales well with core count

---

### Work-Stealing Strategy

* Local operations: push/pop from **front**
* Remote stealing: steal from **back**
* Each queue protected by a lightweight mutex
* Stealing uses `try_lock` to avoid blocking

This approach reduces contention while maintaining correctness.

---

## Features

### Core Runtime

* Fixed-size thread pool
* Per-thread work-stealing queues
* Graceful shutdown (drains remaining tasks)
* Low-contention scheduling

### Asynchronous Execution

* `submit()` returning `std::future`
* `std::packaged_task` integration
* Exception propagation through futures

### Parallel Algorithms

* `parallel_for` — divide-and-conquer parallel loops
* `parallel_reduce` — parallel aggregation

### Performance Instrumentation

Runtime statistics:

* Tasks submitted
* Tasks executed
* Work-steal count
* Thread utilization insights

### Benchmarks

* Scaling across multiple thread counts
* Small-task overhead analysis
* Compute-heavy workload performance
* Latency measurements

---

## Project Structure

```
cpp-task-runtime/
│
├── include/runtime/
│   ├── thread_pool.h          # Public ThreadPool API
│   ├── work_stealing_queue.h  # Per-thread deque implementation
│   ├── task.h                 # Task abstraction
│   ├── parallel_for.h         # Parallel loop implementation
│   ├── parallel_reduce.h      # Parallel reduction
│   ├── stats.h                # Runtime metrics
│   └── config.h               # Tuning parameters
│
├── src/
│   ├── thread_pool.cpp
│   ├── worker.cpp
│   ├── work_stealing_queue.cpp
│   └── stats.cpp
│
├── benchmarks/
│   ├── scaling_benchmark.cpp
│   ├── small_tasks.cpp
│   ├── heavy_tasks.cpp
│   └── latency_benchmark.cpp
│
├── examples/
│   ├── basic_usage.cpp
│   ├── future_example.cpp
│   ├── parallel_for_example.cpp
│   └── parallel_reduce_example.cpp
│
├── tests/
│   ├── thread_pool_test.cpp
│   ├── future_test.cpp
│   ├── shutdown_test.cpp
│   └── stress_test.cpp
│
└── CMakeLists.txt
```

---

## Build Instructions

### Requirements

* C++17 or newer
* CMake 3.10+

### Build

```bash
git clone https://github.com/<your-username>/cpp-task-runtime.git
cd cpp-task-runtime

mkdir build
cd build
cmake ..
make
```

---

## Example Usage

```cpp
#include <runtime/thread_pool.h>
#include <vector>
#include <iostream>

int main() {
    runtime::ThreadPool pool(8);

    auto future = pool.submit([] {
        return 42;
    });

    std::cout << future.get() << std::endl;
}
```

---

### Parallel For

```cpp
runtime::parallel_for(pool, 0, n, [&](int i) {
    data[i] = compute(i);
});
```

---

### Parallel Reduce

```cpp
int sum = runtime::parallel_reduce(
    pool,
    data.begin(),
    data.end(),
    0,
    std::plus<>()
);
```

---

## Benchmarking

Run scaling benchmark:

```bash
./scaling_benchmark
```

Example output:

```
Threads: 1  -> 3.21s
Threads: 2  -> 1.74s
Threads: 4  -> 0.95s
Threads: 8  -> 0.52s
Speedup: 6.17x
Steals: 12,450
```

---

## Design Goals

* Efficient execution of fine-grained tasks
* Minimal synchronization overhead
* Scalable performance across cores
* Clear separation between API and scheduler internals
* Observability for performance analysis

---

## Real-World Relevance

Work-stealing runtimes are used in:

* High-frequency trading systems
* Risk and pricing engines
* Game engines
* Database query schedulers
* Parallel computing frameworks

This project demonstrates core concepts used in production systems:

* Task scheduling
* Load balancing
* Lock contention reduction
* Concurrent data structures
* Performance measurement

---

## Future Improvements

* Lock-free Chase–Lev deque
* Priority scheduling
* Task affinity
* NUMA-aware scheduling
* Adaptive worker parking strategies

---

## License

MIT License
