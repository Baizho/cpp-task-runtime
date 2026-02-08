// Thread pool with work stealing
#ifndef THREAD_POOL_H
#define THREAD_POOL_H

#include <runtime/config.h>
#include <runtime/work_stealing_queue.h>
#include <thread>
#include <vector>
#include <random>
#include <atomic>
#include <chrono>
#include <condition_variable>

namespace runtime { 

class ThreadPool {
    public:
        // Constructor with options
        explicit ThreadPool(const config::ThreadPoolOptions& options = {});
        ~ThreadPool() noexcept;

        void submit(Task task);
        void wait(); 
    private:
        void execute_task(Task& task);
        void worker(size_t idx);
        size_t get_random_thread();
        size_t get_next_victim(size_t i, size_t attempt);

        size_t thread_count_;
        int steal_attempts_;
        std::chrono::milliseconds idle_sleep_;
        size_t max_queue_tasks_;
        config::StealPolicy steal_policy_;

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

        // indicate when it should stop
        std::atomic<bool> stop_;
        
        std::vector<std::thread> threads_;
        std::vector<WorkStealingQueue> work_queues_;
        WorkStealingQueue global_queue_;  // Add unbounded overflow queue

        std::atomic<size_t> active_tasks_{0}; // track running tasks
        std::condition_variable cv_completion_; 
        std::mutex completion_mutex_;
};

} // namespace runtime

#endif // THREAD_POOL_H
