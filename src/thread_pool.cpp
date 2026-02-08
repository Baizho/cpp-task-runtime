// Thread pool with work stealing
#include <runtime/thread_pool.h>
#include <stdexcept>
// #include <iostream>

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
        // std::cout << "Creating ThreadPool " << "\n";
        // Validate configuration
        if (thread_count_ == 0) {
            throw std::invalid_argument("Thread count must be > 0");
        }
        if (steal_attempts_ <= 0) {
            throw std::invalid_argument("Steal attempts must be > 0");
        }
        work_queues_.reserve(thread_count_);
        for (size_t i = 0; i < thread_count_; ++i) {
            work_queues_.emplace_back(std::make_unique<WorkStealingQueue>());
        }

        threads_.reserve(thread_count_);
        for (size_t i = 0; i < thread_count_; ++i) {
            // std::cout << "Starting thread " << i << "\n";
            threads_.emplace_back(&ThreadPool::worker, this, i);
        }

        // std::cout << "ThreadPool created " << "\n";
    } 

// Destructor
ThreadPool::~ThreadPool() noexcept {
    // std::cout << "Called destructor \n";
    stop_.store(true, std::memory_order_release);
	// Join all worker threads (blocking until all tasks complete)
	for (auto& thr: threads_) {
		thr.join();
	}
}

// Choose a thread's queue and add a task to it
void ThreadPool::submit(Task task) {
    // std::cout << "Submitting a task " << "\n";
    // Check if shutting down
    if (stop_.load(std::memory_order_acquire)) {
        throw std::runtime_error("ThreadPool is shutting down");
    }
    
    // RAII guard for exception safety
    struct SubmitGuard {
        std::atomic<size_t>& counter;
        bool committed = false;
        ~SubmitGuard() noexcept {
            if (!committed) {
                counter.fetch_sub(1, std::memory_order_release);
            }
        }
    };
    
    active_tasks_.fetch_add(1, std::memory_order_release);
    SubmitGuard guard{active_tasks_};
    
    
    
    size_t idx = get_random_thread();
    if (work_queues_[idx]->try_push(std::move(task), max_queue_tasks_)) {
        guard.committed = true;
        // std::cout << "Submitted a task " << "\n";
        return;
    }
    
    global_queue_.push(std::move(task));  // Protected now!
    guard.committed = true;
    // std::cout << "Submitted a task " << "\n";
}

void ThreadPool::worker(size_t idx) {
    // std::cout << "Worker " << std::this_thread::get_id() << " is here\n";
    while(true) {
        Task task;

        if (work_queues_[idx]->try_pop(task)) {
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
            if (work_queues_[i]->try_steal(task)) {
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
            // std::cout << "Steal from global queue\n";
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

// Execute task with exception handling
void ThreadPool::execute_task(Task& task) {
    try {
        // std::cout << active_tasks_.load(std::memory_order_relaxed) << "\n";
        task();
    } catch (const std::exception& e) {
        // Optional: 
        // std::cerr << "Task exception: " << e.what() << '\n';
    } catch (...) {
        // Optional: 
        // std::cerr << "Task unknown exception\n";
    }
}

void ThreadPool::wait() {
    // WARNING: Do not call wait() from within a task, as it will deadlock
    std::unique_lock<std::mutex> lock(completion_mutex_);
    // std::cout << "Waiting " << "\n";
    cv_completion_.wait(lock, [this] {
        return active_tasks_.load(std::memory_order_acquire) == 0;
    });
    // std::cout << "Done waiting " << "\n";
}

} // namespace runtime