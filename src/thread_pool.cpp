// Thread pool with work stealing
#include <runtime/thread_pool.h>
#include <stdexcept>

namespace runtime {

// Constructor with options
ThreadPool::ThreadPool(const config::ThreadPoolOptions& options)
    : thread_count_(options.threads),
      steal_attempts_(options.steal_attempts),
      idle_sleep_(options.idle_sleep),
      max_queue_tasks_(options.max_queue_tasks),
      steal_policy_(options.steal_policy),
      stop_(false)
    {
        // Validate configuration
        if (thread_count_ == 0) {
            throw std::invalid_argument("Thread count must be > 0");
        }
        if (steal_attempts_ <= 0) {
            throw std::invalid_argument("Steal attempts must be > 0");
        }
        work_queues_.resize(thread_count_);
        for (size_t i = 0; i < thread_count_; ++i) {
            threads_.emplace_back(&ThreadPool::worker, this, i);
        }
    } 

// Destructor
ThreadPool::~ThreadPool() noexcept {
    stop_.store(true, std::memory_order_release);
	// Wait for the threads to finish
	for (auto& thr: threads_) {
		thr.join();
	}
}

// Choose a thread's queue and add a task to it
void ThreadPool::submit(Task task) {
    // Check if shutting down
    if (stop_.load(std::memory_order_acquire)) {
        throw std::runtime_error("ThreadPool is shutting down");
    }

    active_tasks_.fetch_add(1, std::memory_order_release);
    
    int idx = get_random_thread();
    // Try local queue first
    if (work_queues_[idx].try_push(std::move(task), max_queue_tasks_)) {
        return;
    }
    
    // Fallback to global queue (unbounded)
    global_queue_.push(std::move(task));
}

// get random thread function
size_t ThreadPool::get_random_thread() {
    thread_local std::mt19937 rng(std::random_device{}());
    std::uniform_int_distribution<size_t> dist(0, thread_count_ - 1);
    return dist(rng);
}

// get next victim according to the steal policy
size_t ThreadPool::get_next_victim(size_t i, size_t attempt) {
    if (steal_policy_ == config::StealPolicy::Random) return get_random_thread();
    return (i + attempt) % thread_count_;
}

void ThreadPool::wait() {
    //If a submitted task calls wait(), it deadlocks (task is counted, waits for count to reach 0, but can't complete while waiting).
    // just a precaution
    std::unique_lock<std::mutex> lock(completion_mutex_);
    cv_completion_.wait(lock, [this] {
        return active_tasks_.load(std::memory_order_acquire) == 0;
    });
}

} // namespace runtime