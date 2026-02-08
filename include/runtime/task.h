#ifndef TASK_H
#define TASK_H

#include <functional>
#include <atomic>
#include <condition_variable>

namespace runtime {

using Task = std::function<void()>;

} // namespace runtime

#endif // TASK_H
