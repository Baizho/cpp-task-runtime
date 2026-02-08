#ifndef STATS_H
#define STATS_H

#include <atomic>
#include <cstdint>

namespace runtime {

struct RuntimeStats {
    std::atomic<uint64_t> tasks_submitted{0};
    std::atomic<uint64_t> tasks_executed{0};
    std::atomic<uint64_t> tasks_stolen{0};
    std::atomic<uint64_t> steal_attempts{0};
    std::atomic<uint64_t> failed_steals{0};
};

} // namespace runtime

#endif // STATS_H
