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
        void worker(size_t idx);
        int get_random_thread();
        int get_next_victim(size_t i, size_t attempt);

        size_t thread_count_;
        int steal_attempts_;
        std::chrono::milliseconds idle_sleep_;
        size_t max_queue_tasks_;
        config::StealPolicy steal_policy_;

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
