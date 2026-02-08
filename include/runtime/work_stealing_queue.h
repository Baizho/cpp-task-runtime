#ifndef WORK_STEALING_QUEUE_H
#define WORK_STEALING_QUEUE_H

#include <runtime/task.h>
#include <mutex>
#include <deque>

namespace runtime {

class WorkStealingQueue {
    public:
        // Thread-safety:
        // All operations are protected by mutex_.
        // Owner thread calls push/try_pop.
        // Other threads call try_steal.
        WorkStealingQueue() noexcept = default;

        // Remove copy and move capabilities
        WorkStealingQueue(const WorkStealingQueue&) noexcept = delete;
        WorkStealingQueue& operator=(const WorkStealingQueue&) noexcept = delete;
        WorkStealingQueue(WorkStealingQueue&&) noexcept = delete;
        WorkStealingQueue& operator=(WorkStealingQueue&&) noexcept = delete;

        void push(Task task);           // Owner: push back
        bool try_push(Task& task, size_t max_queue_size);       // Owner: try push back
        bool try_pop(Task& task);        // Owner: pop back
        bool try_steal(Task& task);      // Thief: pop front
        bool empty() const; 
        size_t size() const; 
    private:
        std::deque<Task> deque_;
        mutable std::mutex mutex_;
};

} // namespace runtime

#endif // WORK_STEALING_QUEUE_H