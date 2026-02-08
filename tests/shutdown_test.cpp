#include <runtime/thread_pool.h>
#include <iostream>
#include <thread>
#include <atomic>
#include <cassert>

void test_graceful_shutdown() {
    std::cout << "Test 1: Graceful shutdown with pending tasks\n";
    std::atomic<int> completed{0};
    
    {
        runtime::ThreadPool pool;
        for (int i = 0; i < 100; ++i) {
            pool.submit([&completed]() {
                std::this_thread::sleep_for(std::chrono::milliseconds(10));
                completed++;
            });
        }
        // Destructor should wait for all tasks
    }
    
    assert(completed == 100);
    std::cout << "  ✓ All 100 tasks completed\n\n";
}

void test_explicit_shutdown() {
    std::cout << "Test 2: Explicit shutdown call\n";
    runtime::ThreadPool pool;
    std::atomic<int> count{0};
    
    for (int i = 0; i < 50; ++i) {
        pool.submit([&count]() {
            count++;
        });
    }
    
    pool.shutdown();
    
    // Should throw after shutdown
    bool caught = false;
    try {
        pool.submit([]() {});
    } catch (const std::runtime_error&) {
        caught = true;
    }
    
    assert(caught);
    assert(count == 50);
    std::cout << "  ✓ Shutdown prevents new submissions\n";
    std::cout << "  ✓ All tasks completed before shutdown\n\n";
}

void test_double_shutdown() {
    std::cout << "Test 3: Double shutdown safety\n";
    runtime::ThreadPool pool;
    
    pool.shutdown();
    pool.shutdown();  // Should be safe
    
    std::cout << "  ✓ Double shutdown is safe\n\n";
}

void test_wait_then_shutdown() {
    std::cout << "Test 4: Wait then shutdown\n";
    runtime::ThreadPool pool;
    std::atomic<int> count{0};
    
    for (int i = 0; i < 100; ++i) {
        pool.submit([&count]() {
            std::this_thread::sleep_for(std::chrono::milliseconds(1));
            count++;
        });
    }
    
    pool.wait();
    assert(count == 100);
    
    pool.shutdown();
    
    std::cout << "  ✓ Wait followed by shutdown works correctly\n\n";
}

int main() {
    std::cout << "=== ThreadPool Shutdown Tests ===\n\n";
    
    test_graceful_shutdown();
    test_explicit_shutdown();
    test_double_shutdown();
    test_wait_then_shutdown();
    
    std::cout << "All shutdown tests passed!\n";
    return 0;
}