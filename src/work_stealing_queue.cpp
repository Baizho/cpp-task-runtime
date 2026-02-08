// Thread-safe Work Stealing Queue implementation

#include <runtime/work_stealing_queue.h>
#include <mutex>

namespace runtime {

bool WorkStealingQueue::empty() const {
    std::lock_guard<std::mutex> lock(mutex_);
    return deque_.empty();
}

size_t WorkStealingQueue::size() const {
    std::lock_guard<std::mutex> lock(mutex_);
    return deque_.size();
}

void WorkStealingQueue::push(Task task) {
    std::lock_guard<std::mutex> lock(mutex_);
    deque_.push_back(std::move(task));
}

bool WorkStealingQueue::try_push(Task task, size_t max_queue_size) {
    std::lock_guard<std::mutex> lock(mutex_);

    if (deque_.size() >= max_queue_size) return false;

    deque_.push_back(std::move(task));
    return true;
}

bool WorkStealingQueue::try_pop(Task& task) {
    std::lock_guard<std::mutex> lock(mutex_);

    if (deque_.empty()) return false;

    task = std::move(deque_.back());
    deque_.pop_back();

    return true;
}

bool WorkStealingQueue::try_steal(Task& task) {
    std::lock_guard<std::mutex> lock(mutex_);

    if (deque_.empty()) return false;

    task = std::move(deque_.front());
    deque_.pop_front();

    return true;
}

} // namespace runtime
