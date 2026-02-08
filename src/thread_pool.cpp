// Thread pool with work stealing
#include <runtime/thread_pool.h>

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
    active_tasks_.fetch_add(1, std::memory_order_relaxed);  // ADD THIS LINE
    int i = get_random_thread();

    // Enforce capacity limit
    if (work_queues_[i].size() >= max_queue_tasks_) {
        // we will do something
    }

    work_queues_[i].push(std::move(task));
}

// get random thread function
int ThreadPool::get_random_thread() {
    thread_local std::mt19937 rng(std::random_device{}());
    std::uniform_int_distribution<int> dist(0, thread_count_ - 1);
    return dist(rng);
}

// get next victim according to the steal policy
int ThreadPool::get_next_victim(size_t i, size_t attempt) {
    if (steal_policy_ == config::StealPolicy::Random) return get_random_thread();
    return (i + attempt) % thread_count_;
}

void ThreadPool::wait() {
    std::unique_lock<std::mutex> lock(completion_mutex_);
    cv_completion_.wait(lock, [this] {
        return active_tasks_.load(std::memory_order_relaxed) == 0;
    });
}

} // namespace runtime