#ifndef CONFIG_H
#define CONFIG_H

#include <thread>
#include <chrono>

namespace runtime {
namespace config {

// ==============================
// Worker Thread Configuration
// ==============================
namespace worker {

// Default number of worker threads (falls back to 1 if hardware_concurrency cannot detect)
inline size_t default_threads() {
    size_t n = std::thread::hardware_concurrency();
    return n == 0 ? 1 : n;
}

// Maximum number of victims to try stealing from before sleeping
inline constexpr int steal_attempts = 4;

// Idle sleep duration when no tasks are available
inline constexpr std::chrono::milliseconds idle_sleep{1};

} // namespace worker

// ==============================
// Per-thread Queue Configuration
// ==============================
namespace queue {

// Maximum number of tasks per thread queue to prevent memory blow-up
inline constexpr size_t max_tasks = 1 << 16;

} // namespace queue

// ==============================
// Parallel Algorithm Configuration
// ==============================
namespace parallel_alg {

// Number of tasks per chunk for parallel_for or parallel_reduce
inline constexpr int chunk_size = 1024;

} // namespace parallel_alg

// ==============================
// Enum for Steal Policy
// ==============================
enum class StealPolicy {
    Random,
    RoundRobin
};

// Default stealing policy
inline constexpr StealPolicy default_steal_policy = StealPolicy::Random;

// ==============================
// Runtime Override Struct
// ==============================
struct ThreadPoolOptions {
    size_t threads = worker::default_threads();
    int steal_attempts = worker::steal_attempts;
    std::chrono::milliseconds idle_sleep = worker::idle_sleep;
    size_t max_queue_tasks = queue::max_tasks;
    StealPolicy steal_policy = default_steal_policy;
};

} // namespace config
} // namespace runtime

#endif // CONFIG_H
