#include <runtime/thread_pool.h>
#include <iostream>
#include <vector>
#include <atomic>

int main() {
    std::cout << "=== Basic ThreadPool Usage ===\n\n";
    
    // 1. Create thread pool with default settings
    std::cout << "1. Creating thread pool...\n";
    runtime::ThreadPool pool;
    
    // 2. Submit simple tasks
    std::cout << "2. Submitting fire-and-forget tasks...\n";
    std::atomic<int> counter{0};
    
    for (int i = 0; i < 10; ++i) {
        pool.submit([&counter, i]() {
            counter++;
            std::cout << "  Task " << i << " executed\n";
        });
    }
    
    pool.wait();
    std::cout << "   Completed " << counter << " tasks\n\n";
    
    // 3. Submit tasks with return values
    std::cout << "3. Submitting tasks with futures...\n";
    auto future1 = pool.submit_task([]() {
        return 42;
    });
    
    auto future2 = pool.submit_task([](int a, int b) {
        return a + b;
    }, 10, 20);
    
    std::cout << "   Future 1 result: " << future1.get() << "\n";
    std::cout << "   Future 2 result: " << future2.get() << "\n\n";
    
    // 4. View statistics
    std::cout << "4. Runtime statistics:\n";
    const auto& stats = pool.stats();
    std::cout << "   Tasks submitted: " << stats.tasks_submitted << "\n";
    std::cout << "   Tasks executed: " << stats.tasks_executed << "\n";
    std::cout << "   Tasks stolen: " << stats.tasks_stolen << "\n";
    std::cout << "   Steal attempts: " << stats.steal_attempts << "\n";
    std::cout << "   Failed steals: " << stats.failed_steals << "\n";
    
    return 0;
}