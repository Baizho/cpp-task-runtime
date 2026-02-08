#ifndef CONFIG_H
#define CONFIG_H

#include <thread>

namespace runtime {
namespace config {

// Default number of worker threads
inline unsigned default_thead_count() {
    unsigned n = std::thread::hardware_concurrency();
    return n == 0 ? 1 : n;
}

// Number of victims to try to steal from before sleeping
constexpr int steal_attempts = 4;

// How many tasks per chunk for parallel_for
constexpr int parallel_chunk_size = 1024;

// Maximum tasks per queue
constexpr int max_queue_size = 1 << 16;

} // namespace config
} // namespace runtime

#endif // CONFIG_H