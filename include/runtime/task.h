#ifndef TASK_H
#define TASK_H

#include <functional>

namespace runtime {

using Task = std::function<void()>;

} // namespace runtime

#endif // TASK_H
