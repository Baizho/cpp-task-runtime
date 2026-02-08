// Worker implementation
#include <runtime/thread_pool.h>
#include <iostream>
#include <stdexcept>

namespace runtime {

void ThreadPool::worker(size_t idx) {
    while(true) {
        Task task;

        if (work_queues_[idx].try_pop(task)) {
            {
                TaskGuard guard(active_tasks_, cv_completion_);
                execute_task(task);
            }
            continue;
        }

        // stealing from other threads

        bool found_work = false;

        for (size_t attempt = 1; attempt <= steal_attempts_; ++attempt) {
            size_t i = get_next_victim(idx, attempt);
            if (work_queues_[i].try_steal(task)) {
                found_work = true;
                {
                    TaskGuard guard(active_tasks_, cv_completion_);
                    execute_task(task);
                }
                break;
            }
        }

        if (found_work) continue;

        // try global queue
        if (global_queue_.try_steal(task)) {  // Use try_steal (FIFO from global)
            {
                TaskGuard guard(active_tasks_, cv_completion_);
                execute_task(task);
            }
            continue;
        }
        
        if (stop_.load(std::memory_order_acquire) && active_tasks_.load(std::memory_order_acquire) == 0) {
            break;
        }

        std::this_thread::sleep_for(idle_sleep_); 
    }
}

} // namespace runtime