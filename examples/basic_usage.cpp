// Showing the basic usage of the whole project

#include <runtime/work_stealing_queue.h>
#include <runtime/task.h>
#include <thread>
#include <iostream>
#include <vector>
#include <chrono>

int main() {
    runtime::WorkStealingQueue queue;

    // Push 10 tasks
    for (int i = 0; i < 5; ++i) {
        queue.push([i]() {
            std::cout << "Task " << i << " executed by thread " << std::this_thread::get_id() << "\n";
        });
    }

    // Owner thread pops tasks
    std::thread owner([&queue]() {
        runtime::Task task;
        while (queue.try_pop(task)) {
            task();
            std::this_thread::sleep_for(std::chrono::milliseconds(50));
        }
        std::cout << "Owner done\n";
    });

    // Another thread tries to steal tasks
    std::thread thief([&queue]() {
        runtime::Task task;
        while (queue.try_steal(task)) {
            task();
            std::this_thread::sleep_for(std::chrono::milliseconds(50));
        }
        std::cout << "Thief done\n";
    });

    owner.join();
    thief.join();

    // Final check
    if (queue.empty()) {
        std::cout << "Queue is empty\n";
    } else {
        std::cout << "Queue still has tasks\n";
    }

    return 0;
}
