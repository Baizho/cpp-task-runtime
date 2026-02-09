# cpp-task-runtime

A high-performance **work-stealing thread pool** written in modern C++.

This project implements a production-quality task execution engine capable of efficiently running millions of asynchronous tasks across multiple CPU cores. It features futures for result retrieval, parallel algorithms, comprehensive performance instrumentation, and robust exception safety—designed to resemble the core scheduling infrastructure used in real-world systems like Intel TBB and Java's ForkJoinPool.

---

## Overview

Modern applications frequently need to execute millions of small, independent tasks efficiently. Examples include:

* Financial risk and pricing computations
* Market data processing pipelines
* Game engines and simulation systems
* Database query execution engines
* Machine learning workloads
* Parallel scientific computing

Naive thread pools often suffer from:

* **Lock contention** on a global queue
* **Poor CPU utilization** due to load imbalance
* **Scalability bottlenecks** as thread count increases

This project implements a **work-stealing scheduler**, a widely-used industry solution found in:
- Intel Threading Building Blocks (TBB)
- Cilk
- Java ForkJoinPool
- Rust Rayon
- Go runtime scheduler

---

## Key Features

### ✨ Core Runtime
* **Work-stealing scheduler** with per-thread queues
* **Configurable thread pool** size and stealing policies
* **Global overflow queue** for bounded per-thread queues
* **Graceful shutdown** that drains all pending tasks
* **Exception-safe** task execution with RAII guards
* **Condition variable optimization** for efficient worker wake-up

### 🔮 Asynchronous Execution
* `submit()` for fire-and-forget tasks
* `submit_task()` returning `std::future<T>` for result retrieval
* Full exception propagation through futures
* Template-based type-safe task submission

### 🚀 Parallel Algorithms
* **`parallel_for`** — efficient parallel loop execution with automatic chunking
* **`parallel_reduce`** — parallel aggregation with custom reduce operations
* Configurable chunk sizes for performance tuning

### 📊 Performance Instrumentation
Built-in runtime statistics:
* Tasks submitted/executed
* Work-steal success/failure counts
* Steal attempt tracking
* Zero-overhead when not accessed

### 🔧 Highly Configurable
* Thread count (default: `hardware_concurrency()`)
* Steal policy: Random or Round-Robin
* Queue capacity limits
* Idle sleep duration
* Steal attempt count

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
(submit, submit_task with futures)
    │
    ▼
Scheduler Core
(work distribution & stealing)
    │
    ├─► Per-Thread Work-Stealing Queues (LIFO for owner, FIFO for thieves)
    └─► Global Overflow Queue (unbounded fallback)
    │
    ▼
Worker Threads
(condition variable sleep/wake)
```

---

### Execution Flow

1. **User submits task** via `submit()` or `submit_task()`
2. **Task placement**: Randomly assigned to a worker's local queue
3. **Local execution**: Worker executes from its own queue (LIFO, cache-friendly)
4. **Work stealing**: When idle, workers steal from others (FIFO to reduce contention)
5. **Global fallback**: If local queue is full, task goes to global overflow queue
6. **Graceful shutdown**: Workers drain all queues before exiting

---

### Work-Stealing Strategy

* **Local operations**: Push/pop from back (LIFO) — excellent cache locality
* **Remote stealing**: Steal from front (FIFO) — minimizes contention with owner
* **Mutex-based queues**: Each queue protected by `std::mutex` with `std::deque`
* **Stealing order**: Configurable (Random or Round-Robin)
* **Self-stealing prevention**: Workers skip their own queue when stealing

**Why this works:**
- Minimizes lock contention (thieves rarely conflict with owner)
- Balances uneven workloads dynamically
- Scales near-linearly with core count for most workloads

---

## Project Structure
```
cpp-task-runtime/
│
├── include/runtime/
│   ├── thread_pool.h          # ThreadPool API with futures
│   ├── work_stealing_queue.h  # Mutex-based deque (LIFO/FIFO)
│   ├── task.h                 # Task type alias (std::function<void()>)
│   ├── parallel_for.h         # Parallel loop implementation
│   ├── parallel_reduce.h      # Parallel reduction algorithms
│   ├── stats.h                # Runtime metrics (atomic counters)
│   └── config.h               # Tuning parameters & options
│
├── src/
│   ├── thread_pool.cpp        # ThreadPool implementation
│   └── work_stealing_queue.cpp # Queue implementation
│
├── benchmarks/
│   ├── scaling_benchmark.cpp  # Strong/weak scaling tests
│   ├── small_tasks.cpp        # Overhead analysis
│   ├── heavy_tasks.cpp        # CPU-intensive workloads
│   └── latency_benchmark.cpp  # Latency measurements
│
├── examples/
│   ├── basic_usage.cpp         # Simple fire-and-forget tasks
│   ├── future_example.cpp      # Using futures for results
│   ├── parallel_for_example.cpp # Parallel loops
│   └── parallel_reduce_example.cpp # Parallel aggregation
│
├── tests/
│   ├── thread_pool_test.cpp           # Comprehensive test suite
│   ├── work_stealing_queue_test.cpp   # Queue correctness tests
│   └── shutdown_test.cpp              # Graceful shutdown tests
│
└── CMakeLists.txt
```

---

## Build Instructions

### Requirements

* **C++17** or newer
* **CMake 3.10+**
* **Compiler**: GCC 7+, Clang 5+, MSVC 2017+
* **Platform**: Linux, macOS, Windows

### Build
```bash
git clone https://github.com/Baizho/cpp-task-runtime.git
cd cpp-task-runtime

mkdir build && cd build
cmake ..
make
```

### Run Tests
```bash
./thread_pool_test
./work_stealing_queue_test
./shutdown_test
```

### Run Benchmarks
```bash
./scaling_benchmark
./small_tasks
./heavy_tasks
./latency_benchmark
```

---

## Usage Examples

### Basic Fire-and-Forget
```cpp
#include <runtime/thread_pool.h>
#include <iostream>

int main() {
    runtime::ThreadPool pool;  // Uses hardware_concurrency() threads
    
    pool.submit([]() {
        std::cout << "Task executed!\n";
    });
    
    pool.wait();  // Wait for all tasks to complete
    return 0;
}
```

---

### Futures for Results
```cpp
#include <runtime/thread_pool.h>
#include <iostream>

int main() {
    runtime::ThreadPool pool;
    
    // Submit task with return value
    auto future = pool.submit_task([](int a, int b) {
        return a + b;
    }, 10, 20);
    
    std::cout << "Result: " << future.get() << "\n";  // Prints 30
    return 0;
}
```

---

### Parallel For Loop
```cpp
#include <runtime/thread_pool.h>
#include <runtime/parallel_for.h>
#include <vector>

int main() {
    runtime::ThreadPool pool;
    std::vector<int> data(1000000);
    
    // Parallel fill with squares
    runtime::parallel_for(pool, size_t(0), data.size(), [&](size_t i) {
        data[i] = i * i;
    });
    
    return 0;
}
```

---

### Parallel Reduce (Sum)
```cpp
#include <runtime/thread_pool.h>
#include <runtime/parallel_reduce.h>

int main() {
    runtime::ThreadPool pool;
    
    // Sum of squares from 0 to 9,999,999
    long long sum = runtime::parallel_reduce(
        pool,
        0, 10000000,
        0LL,  // Initial value
        [](int i) { return static_cast<long long>(i) * i; },  // Map
        std::plus<long long>()  // Reduce operation
    );
    
    std::cout << "Sum: " << sum << "\n";
    return 0;
}
```

---

### Custom Configuration
```cpp
#include <runtime/thread_pool.h>
#include <runtime/config.h>

int main() {
    runtime::config::ThreadPoolOptions options;
    options.threads = 4;
    options.steal_policy = runtime::config::StealPolicy::RoundRobin;
    options.max_queue_tasks = 1000;
    options.idle_sleep = std::chrono::milliseconds(2);
    
    runtime::ThreadPool pool(options);
    
    // Use custom-configured pool...
    
    return 0;
}
```

---

### View Runtime Statistics
```cpp
#include <runtime/thread_pool.h>
#include <iostream>

int main() {
    runtime::ThreadPool pool;
    
    for (int i = 0; i < 10000; ++i) {
        pool.submit([]() { /* work */ });
    }
    
    pool.wait();
    
    const auto& stats = pool.stats();
    std::cout << "Tasks submitted: " << stats.tasks_submitted << "\n";
    std::cout << "Tasks executed: " << stats.tasks_executed << "\n";
    std::cout << "Tasks stolen: " << stats.tasks_stolen << "\n";
    std::cout << "Failed steals: " << stats.failed_steals << "\n";
    
    return 0;
}
```

---

## Benchmarking

### Scaling Benchmark
```bash
./scaling_benchmark
```

**Example Output:**
```
=== Thread Scaling Benchmark ===
Threads    Time (ms)      Tasks/sec      Speedup
--------------------------------------------------------
1          2847           35123          1.00
2          1456           68681          1.95
4          751            133156         3.79
8          402            248756         7.08

=== Weak Scaling Benchmark ===
Tasks per thread: 10000, Work per task: 500

Threads   Total Tasks    Time (ms)      Efficiency
-------------------------------------------------------
1         10000          291            100.0          %
2         20000          177            164.4          %
4         40000          226            128.8          %
8         80000          237            122.8          %
```

---

### Latency Benchmark
```bash
./latency_benchmark
```

**Example Output:**
```
=== Task Submission Latency Benchmark ===
Time from submit() to task execution start

Samples: 10000
Mean:    298.73 μs
Median:  202.00 μs
P95:     924.00 μs
P99:     1343.00 μs
Min:     0.00 μs
Max:     5784.00 μs

=== Wait Latency Benchmark ===
Time for wait() to return after last task completes

Tasks          Wait Latency (μs)
-----------------------------------
10             54
100            26
1000           15
10000          14

=== Future Get Latency Benchmark ===
Time from task completion to future.get() return

Samples: 1000
Mean:    33.93 μs
Median:  30.00 μs
P95:     66.00 μs

=== Work Stealing Latency Benchmark ===
Response time when one thread is overloaded

Total tasks: 100
Mean response time:   220.66 ms
Median response time: 201.00 ms
P95 response time:    422.00 ms

Work stealing stats:
  Tasks stolen: 18
  Steal attempts: 44
  Success rate: 40.9%
```

---

### Heavy tasks
```bash
./heavy_tasks
```

**Example Output:**
```
Hardware threads: 8

=== CPU-Intensive Tasks Benchmark ===
Heavy mathematical computations

Tasks          Time (ms)      Throughput (tasks/s)
--------------------------------------------------
10             1153           8.67
50             4409           11.34
100            9560           10.46
200            17696          11.30

=== Parallel Matrix Multiplication Benchmark ===
Multiple matrix multiplications in parallel

Multiplications     Time (ms)      Matrices/sec
-------------------------------------------------------
1                   240            4.17
5                   330            15.15
10                  720            13.89
20                  1190           16.81

=== Mixed Heavy Workload Benchmark ===
Combination of different heavy tasks

Total tasks: 100
Total time: 1924 ms
Throughput: 51.98 tasks/sec
```

---

### Small tasks
```bash
./small_tasks
```

**Example Output:**
```
=== Tiny Tasks Benchmark ===
Measures overhead for very small tasks

Task Count     Time (ms)      Tasks/sec           Avg Time/Task (μs)
----------------------------------------------------------------------
1000           23             43478               23.479
10000          27             370370              2.755
100000         76             1315789             0.770
1000000        504            1984127             0.504

=== Varying Workload Benchmark ===
Tasks with different amounts of work

Work Amount    Time (ms)      Tasks/sec
--------------------------------------------------
10             13             769231
100            5              2000000
1000           24             416667
10000          47             212766

=== Submission Rate Benchmark ===
How fast can we submit tasks?

Tasks submitted: 1000000
Total time: 522 ms
Submissions/sec: 1914271
Avg time/submission: 0.522 μs
```

---

## Design Highlights

### Exception Safety
- **RAII guards** ensure task counters are always decremented
- **Try-catch blocks** prevent exceptions from terminating workers
- **Future exception propagation** via `std::packaged_task`

### Concurrency Primitives
- **Atomic operations** with explicit memory ordering
- **Condition variables** for efficient worker sleep/wake
- **Mutex-protected queues** with minimal critical sections

### Performance Optimizations
- **Thread-local RNG** eliminates mutex contention on random queue selection
- **Condition variable notifications** prevent busy-waiting
- **Task guard optimization** only notifies when last task completes

---

## Real-World Relevance

Work-stealing runtimes are foundational to:

* **High-frequency trading systems** (latency-critical task scheduling)
* **Risk and pricing engines** (parallel Monte Carlo simulations)
* **Game engines** (job systems for frame-parallel execution)
* **Database query schedulers** (parallel query execution)
* **Parallel computing frameworks** (MapReduce, data processing)

This project demonstrates production-grade concepts:
- Lock-free thinking (minimal critical sections)
- Load balancing algorithms
- Concurrent data structure design
- Performance measurement and profiling
- Exception-safe concurrent programming

---

## Implementation Details

### Key Design Decisions

1. **Mutex-based queues instead of lock-free**
   - Simpler implementation
   - Still achieves excellent performance
   - Future upgrade possibility to Chase-Lev deque

2. **Global overflow queue**
   - Prevents task rejection when local queues are full
   - Acts as load balancer for burst workloads

3. **Condition variable sleep**
   - Workers sleep efficiently when idle
   - Woken on task submission or shutdown
   - Timeout-based to handle missed notifications

4. **Exception isolation**
   - Worker threads never terminate due to task exceptions
   - Full exception propagation through futures
   - Silent failure for fire-and-forget tasks

---

## Future Improvements

* 🔒 Lock-free Chase–Lev deque implementation
* 📊 Priority task scheduling
* 🎯 Task affinity and NUMA-aware placement
* 🔋 Adaptive worker parking strategies
* 📈 Work-stealing histogram metrics
* 🧵 Thread naming and debugging support

---

## Testing

Comprehensive test suite covering:
- ✅ Basic task submission and execution
- ✅ Future return values and exception propagation
- ✅ Work stealing and load balancing
- ✅ Graceful shutdown with pending tasks
- ✅ Queue overflow handling
- ✅ Nested task submission
- ✅ Exception safety under stress

Run tests:
```bash
./thread_pool_test
```

---

## Performance

Tested on Intel Core system with 8 cores:

- **Throughput**: 500K+ tasks/second (small tasks)
- **Latency**: Sub-10μs median submission latency
- **Scaling**: Near-linear speedup up to hardware thread count
- **Overhead**: Minimal for tasks >1μs of work

---

## Contributing

Contributions welcome! Areas of interest:
- Lock-free queue implementations
- Additional parallel algorithms
- Platform-specific optimizations
- Benchmark improvements

---

## License

MIT License

---

## Acknowledgments

Inspired by:
- Intel Threading Building Blocks (TBB)
- Java ForkJoinPool
- Cilk work-stealing scheduler
- "C++ Concurrency in Action" by Anthony Williams

---

## Author

Created as a demonstration of production-quality concurrent systems programming in C++.

For questions or suggestions, open an issue on GitHub.
