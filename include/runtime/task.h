#ifndef TASK_H
#define TASK_H

#include <functional>
#include <atomic>
#include <condition_variable>

namespace runtime {

using Task = std::function<void()>;

struct TaskGuard {
    std::atomic<size_t>& counter;
    std::condition_variable& cv;
    
    TaskGuard(std::atomic<size_t>& c, std::condition_variable& v) 
        : counter(c), cv(v) {}
    
    ~TaskGuard() noexcept { 
        try {
            size_t prev = counter.fetch_sub(1, std::memory_order_release);
            if (prev == 1) {
                cv.notify_all();
            }
        } catch (...) {
            // Suppress - destructor must not throw
        }
    }
};

} // namespace runtime

#endif // TASK_H
