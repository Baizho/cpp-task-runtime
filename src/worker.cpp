// Worker implementation
#include <runtime/thread_pool.h>

namespace runtime {

void ThreadPool::worker(size_t idx) {
    while(true) {
        Task task;

        if (work_queues_[idx].try_pop(task)) {
            task();
            active_tasks_.fetch_sub(1, std::memory_order_relaxed);
            cv_completion_.notify_all();
            continue;
        }

        bool found_work = false;

        for (size_t attempt = 1; attempt <= steal_attempts_; ++attempt) {
            size_t i = get_next_victim(idx, attempt);
            if (work_queues_[i].try_steal(task)) {
                found_work = true;
                task();
                active_tasks_.fetch_sub(1, std::memory_order_relaxed);
                cv_completion_.notify_all();
                break;
            }
        }

        if (found_work) continue;
        if (stop_.load(std::memory_order_acquire) && 
            active_tasks_.load(std::memory_order_relaxed) == 0) {
            break;
        }

        std::this_thread::sleep_for(idle_sleep_); 
    }
}

} // namespace runtime