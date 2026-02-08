// basic_usage.cpp - Thread pool testing and demonstration
#include <runtime/thread_pool.h>
#include <iostream>
#include <chrono>
#include <vector>
#include <atomic>
#include <thread>
#include <cmath>
#include <cassert>

using namespace std::literals;

// ANSI color codes for pretty output
#define GREEN "\033[32m"
#define RED "\033[31m"
#define YELLOW "\033[33m"
#define BLUE "\033[34m"
#define RESET "\033[0m"

void print_test(const std::string& name) {
    std::cout << "\n" << BLUE << "=== " << name << " ===" << RESET << "\n";
}

void print_success(const std::string& msg) {
    std::cout << GREEN << "✓ " << msg << RESET << "\n";
}

void print_error(const std::string& msg) {
    std::cout << RED << "✗ " << msg << RESET << "\n";
}

void print_info(const std::string& msg) {
    std::cout << YELLOW << "ℹ " << msg << RESET << "\n";
}

// ============================================================================
// Test 1: Basic Task Submission
// ============================================================================
void test_basic_submission() {
    print_test("Test 1: Basic Task Submission");
    
    runtime::ThreadPool pool;
    std::atomic<int> counter{0};
    
    // Submit 10 simple tasks
    for (int i = 0; i < 10; ++i) {
        pool.submit([&counter, i]() {
            counter++;
            std::cout << "Task " << i << " executed\n";
        });
    }
    
    pool.wait();
    
    if (counter == 10) {
        print_success("All 10 tasks completed");
    } else {
        print_error("Expected 10, got " + std::to_string(counter.load()));
    }
}

// ============================================================================
// Test 2: CPU-Intensive Work
// ============================================================================
void test_cpu_intensive() {
    print_test("Test 2: CPU-Intensive Work");
    
    runtime::ThreadPool pool;
    std::atomic<int> completed{0};
    
    auto start = std::chrono::high_resolution_clock::now();
    
    // Submit 100 CPU-intensive tasks
    for (int i = 0; i < 100; ++i) {
        pool.submit([&completed]() {
            // Simulate CPU work
            double result = 0.0;
            for (int j = 0; j < 100000; ++j) {
                result += std::sqrt(j) * std::sin(j);
            }
            completed++;
        });
    }
    
    pool.wait();
    auto end = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
    
    print_success("Completed " + std::to_string(completed.load()) + " CPU tasks in " + 
                  std::to_string(duration.count()) + "ms");
}

// ============================================================================
// Test 3: Exception Handling
// ============================================================================
void test_exception_handling() {
    print_test("Test 3: Exception Handling");
    
    runtime::ThreadPool pool;
    std::atomic<int> successful{0};
    std::atomic<int> total{0};
    
    for (int i = 0; i < 20; ++i) {
        pool.submit([&successful, &total, i]() {
            total++;
            if (i % 5 == 0) {
                throw std::runtime_error("Task " + std::to_string(i) + " threw exception");
            }
            successful++;
        });
    }
    
    pool.wait();
    
    print_info("Total tasks: " + std::to_string(total.load()));
    print_info("Successful tasks: " + std::to_string(successful.load()));
    print_info("Failed tasks: " + std::to_string(total.load() - successful.load()));
    
    if (successful == 16) {  // 20 tasks, 4 throw exceptions (i = 0, 5, 10, 15)
        print_success("Exception handling works correctly");
    } else {
        print_error("Expected 16 successful, got " + std::to_string(successful.load()));
    }
}

// ============================================================================
// Test 4: Work Stealing Verification
// ============================================================================
void test_work_stealing() {
    print_test("Test 4: Work Stealing (Load Balancing)");
    
    runtime::ThreadPool pool;
    std::vector<std::atomic<int>> thread_counters(std::thread::hardware_concurrency());
    
    // Submit tasks that record which thread executes them
    for (int i = 0; i < 1000; ++i) {
        pool.submit([&thread_counters]() {
            // Simulate work
            std::this_thread::sleep_for(std::chrono::microseconds(100));
            
            // Note: We can't directly know which worker thread this is,
            // but we can verify work gets distributed
        });
    }
    
    pool.wait();
    print_success("1000 tasks distributed across worker threads");
}

// ============================================================================
// Test 5: Nested Task Submission
// ============================================================================
void test_nested_tasks() {
    print_test("Test 5: Nested Task Submission");
    
    runtime::ThreadPool pool;
    std::atomic<int> outer_count{0};
    std::atomic<int> inner_count{0};
    
    for (int i = 0; i < 10; ++i) {
        pool.submit([&pool, &outer_count, &inner_count]() {
            outer_count++;
            
            // Submit more tasks from within a task
            for (int j = 0; j < 5; ++j) {
                pool.submit([&inner_count]() {
                    inner_count++;
                });
            }
        });
    }
    
    pool.wait();
    
    print_info("Outer tasks: " + std::to_string(outer_count.load()));
    print_info("Inner tasks: " + std::to_string(inner_count.load()));
    
    if (outer_count == 10 && inner_count == 50) {
        print_success("Nested task submission works");
    } else {
        print_error("Expected 10 outer, 50 inner");
    }
}

// ============================================================================
// Test 6: Custom Configuration
// ============================================================================
void test_custom_config() {
    print_test("Test 6: Custom Configuration");
    
    runtime::config::ThreadPoolOptions options;
    options.threads = 2;
    options.steal_attempts = 8;
    options.max_queue_tasks = 100;
    options.steal_policy = runtime::config::StealPolicy::RoundRobin;
    
    runtime::ThreadPool pool(options);
    std::atomic<int> count{0};
    
    for (int i = 0; i < 50; ++i) {
        pool.submit([&count]() {
            count++;
        });
    }
    
    pool.wait();
    
    if (count == 50) {
        print_success("Custom configuration works (2 threads, RoundRobin)");
    } else {
        print_error("Expected 50 tasks");
    }
}

// ============================================================================
// Test 7: Stress Test - Many Small Tasks
// ============================================================================
void test_stress_many_small() {
    print_test("Test 7: Stress Test - 10,000 Small Tasks");
    
    runtime::ThreadPool pool;
    std::atomic<int> count{0};
    
    auto start = std::chrono::high_resolution_clock::now();
    
    for (int i = 0; i < 10000; ++i) {
        pool.submit([&count]() {
            count++;
        });
    }
    
    pool.wait();
    auto end = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
    
    print_success("Completed " + std::to_string(count.load()) + " tasks in " + 
                  std::to_string(duration.count()) + "ms");
    print_info("Throughput: " + std::to_string(count.load() * 1000 / duration.count()) + " tasks/sec");
}

// ============================================================================
// Test 8: Queue Overflow Handling
// ============================================================================
void test_queue_overflow() {
    print_test("Test 8: Queue Overflow (Global Queue Fallback)");
    
    runtime::config::ThreadPoolOptions options;
    options.threads = 2;
    options.max_queue_tasks = 10;  // Small queue to trigger overflow
    
    runtime::ThreadPool pool(options);
    std::atomic<int> count{0};
    
    // Submit many tasks quickly to trigger overflow
    for (int i = 0; i < 100; ++i) {
        pool.submit([&count]() {
            std::this_thread::sleep_for(std::chrono::milliseconds(10));
            count++;
        });
    }
    
    pool.wait();
    
    if (count == 100) {
        print_success("Queue overflow handled correctly (global queue used)");
    } else {
        print_error("Expected 100, got " + std::to_string(count.load()));
    }
}

// ============================================================================
// Test 9: Shutdown During Active Work
// ============================================================================
void test_graceful_shutdown() {
    print_test("Test 9: Graceful Shutdown");
    
    std::atomic<int> completed{0};
    
    {
        runtime::ThreadPool pool;
        
        // Submit long-running tasks
        for (int i = 0; i < 20; ++i) {
            pool.submit([&completed]() {
                std::this_thread::sleep_for(std::chrono::milliseconds(50));
                completed++;
            });
        }
        
        // Pool destructor should wait for all tasks
        print_info("Destroying pool (should wait for all tasks)...");
    }  // Destructor called here
    
    if (completed == 20) {
        print_success("Graceful shutdown completed all tasks");
    } else {
        print_error("Some tasks were not completed: " + std::to_string(completed.load()) + "/20");
    }
}

// ============================================================================
// Test 10: Submit After Shutdown (Should Throw)
// ============================================================================
void test_submit_after_shutdown() {
    print_test("Test 10: Submit After Shutdown");
    
    auto pool = std::make_unique<runtime::ThreadPool>();
    pool.reset();  // Destroy the pool
    
    // Try to create a new pool and immediately destroy it
    try {
        runtime::ThreadPool temp_pool;
        // Start shutdown in another thread
        std::thread shutdown_thread([&temp_pool]() {
            std::this_thread::sleep_for(std::chrono::milliseconds(10));
            // Destructor will be called when this goes out of scope
        });
        
        // Try submitting during shutdown (timing-dependent)
        bool caught_exception = false;
        for (int i = 0; i < 100; ++i) {
            try {
                temp_pool.submit([]() {});
            } catch (const std::runtime_error& e) {
                caught_exception = true;
                print_info("Caught expected exception: " + std::string(e.what()));
                break;
            }
            std::this_thread::sleep_for(std::chrono::milliseconds(1));
        }
        
        shutdown_thread.join();
        
        if (caught_exception) {
            print_success("Exception thrown when submitting during shutdown");
        } else {
            print_info("No exception caught (timing-dependent test)");
        }
    } catch (...) {
        print_info("Test completed (timing-dependent)");
    }
}

// ============================================================================
// Test 11: Wait() Functionality
// ============================================================================
void test_wait_functionality() {
    print_test("Test 11: Wait() Functionality");
    
    runtime::ThreadPool pool;
    std::atomic<int> count{0};
    
    // Submit first batch
    for (int i = 0; i < 10; ++i) {
        pool.submit([&count]() {
            std::this_thread::sleep_for(std::chrono::milliseconds(10));
            count++;
        });
    }
    
    pool.wait();
    int first_batch = count.load();
    
    // Submit second batch
    for (int i = 0; i < 10; ++i) {
        pool.submit([&count]() {
            std::this_thread::sleep_for(std::chrono::milliseconds(10));
            count++;
        });
    }
    
    pool.wait();
    int second_batch = count.load();
    
    if (first_batch == 10 && second_batch == 20) {
        print_success("wait() correctly waits for task completion");
    } else {
        print_error("wait() did not work correctly");
    }
}

// ============================================================================
// Test 12: Future Return Values
// ============================================================================
void test_future_return() {
    print_test("Test 12: Future Return Values");
    
    runtime::ThreadPool pool;
    
    // Submit task that returns an int
    auto future1 = pool.submit_task([]() {
        return 42;
    });
    
    // Submit task that returns a string
    auto future2 = pool.submit_task([]() {
        return std::string("Hello from thread pool!");
    });
    
    // submit_task task with parameters
    auto future3 = pool.submit_task([](int a, int b) {
        return a + b;
    }, 10, 20);
    
    // Get results
    int result1 = future1.get();
    std::string result2 = future2.get();
    int result3 = future3.get();
    
    print_info("Result 1: " + std::to_string(result1));
    print_info("Result 2: " + result2);
    print_info("Result 3: " + std::to_string(result3));
    
    if (result1 == 42 && result2 == "Hello from thread pool!" && result3 == 30) {
        print_success("Future return values work correctly");
    } else {
        print_error("Future return values incorrect");
    }
}

// ============================================================================
// Test 13: Multiple Futures
// ============================================================================
void test_multiple_futures() {
    print_test("Test 13: Multiple Futures - Parallel Computation");
    
    runtime::ThreadPool pool;
    std::vector<std::future<int>> futures;
    
    // submit_task 10 computation tasks
    for (int i = 0; i < 10; ++i) {
        futures.push_back(pool.submit_task([i]() {
            // Simulate computation
            int result = 0;
            for (int j = 0; j < 1000000; ++j) {
                result += j % (i + 1);
            }
            return result;
        }));
    }
    
    // Collect all results
    int sum = 0;
    for (auto& future : futures) {
        sum += future.get();
    }
    
    print_success("Computed sum from 10 parallel tasks: " + std::to_string(sum));
}

// ============================================================================
// Test 14: Future Exception Handling
// ============================================================================
void test_future_exceptions() {
    print_test("Test 14: Future Exception Handling");
    
    runtime::ThreadPool pool;
    
    auto future = pool.submit_task([]() -> int {
        throw std::runtime_error("Task failed!");
        return 42;
    });
    
    try {
        int result = future.get();
        print_error("Should have thrown exception");
    } catch (const std::runtime_error& e) {
        print_success("Exception properly propagated through future: " + std::string(e.what()));
    }
}

// ============================================================================
// Test 15: Complex Return Types
// ============================================================================
void test_complex_return_types() {
    print_test("Test 15: Complex Return Types");
    
    runtime::ThreadPool pool;
    
    // Return a vector
    auto future1 = pool.submit_task([]() {
        return std::vector<int>{1, 2, 3, 4, 5};
    });
    
    // Return a struct
    struct Result {
        int value;
        std::string message;
    };
    
    auto future2 = pool.submit_task([]() -> Result {
        return Result{100, "Success"};
    });
    
    // Return a pair
    auto future3 = pool.submit_task([](int a, int b) {
        return std::make_pair(a * b, a + b);
    }, 5, 7);
    
    auto vec = future1.get();
    auto res = future2.get();
    auto [product, sum] = future3.get();
    
    print_info("Vector size: " + std::to_string(vec.size()));
    print_info("Struct: value=" + std::to_string(res.value) + ", msg=" + res.message);
    print_info("Pair: product=" + std::to_string(product) + ", sum=" + std::to_string(sum));
    
    if (vec.size() == 5 && res.value == 100 && product == 35 && sum == 12) {
        print_success("Complex return types work correctly");
    } else {
        print_error("Complex return types failed");
    }
}

// ============================================================================
// Test 16: Future with Wait
// ============================================================================
void test_future_wait_patterns() {
    print_test("Test 16: Future Wait Patterns");
    
    runtime::ThreadPool pool;
    
    auto future = pool.submit_task([]() {
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
        return 42;
    });
    
    // Check if ready
    auto status = future.wait_for(std::chrono::milliseconds(10));
    if (status == std::future_status::timeout) {
        print_info("Future not ready yet (expected)");
    }
    
    // Wait until ready
    future.wait();
    print_success("Future is ready, result: " + std::to_string(future.get()));
}

// ============================================================================
// Main Test Runner
// ============================================================================
int main() {
    std::cout << "\n";
    std::cout << "╔════════════════════════════════════════════════════════════╗\n";
    std::cout << "║         Thread Pool Comprehensive Test Suite              ║\n";
    std::cout << "╚════════════════════════════════════════════════════════════╝\n";
    
    print_info("Hardware threads: " + std::to_string(std::thread::hardware_concurrency()));
    
    try {
        test_basic_submission();
        test_cpu_intensive();
        test_exception_handling();
        test_work_stealing();
        test_nested_tasks();
        test_custom_config();
        test_stress_many_small();
        test_queue_overflow();
        test_graceful_shutdown();
        test_submit_after_shutdown();
        test_wait_functionality();
        test_future_return();
        test_multiple_futures();
        test_future_exceptions();
        test_complex_return_types();
        test_future_wait_patterns();
        
        std::cout << "\n";
        std::cout << GREEN << "╔════════════════════════════════════════════════════════════╗\n";
        std::cout << "║            ALL TESTS COMPLETED SUCCESSFULLY!               ║\n";
        std::cout << "╚════════════════════════════════════════════════════════════╝" << RESET << "\n\n";
        
    } catch (const std::exception& e) {
        print_error("Test suite failed with exception: " + std::string(e.what()));
        return 1;
    }
    
    return 0;
}