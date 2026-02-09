#include <runtime/thread_pool.h>
#include <iostream>
#include <chrono>
#include <iomanip>
#include <atomic>

// Benchmark for many small, fast tasks (tests overhead)
void benchmark_tiny_tasks() {
    std::cout << "=== Tiny Tasks Benchmark ===\n";
    std::cout << "Measures overhead for very small tasks\n\n";
    
    const std::vector<size_t> task_counts = {1000, 10000, 100000, 1000000};
    
    std::cout << std::left << std::setw(15) << "Task Count" 
              << std::setw(15) << "Time (ms)"
              << std::setw(20) << "Tasks/sec"
              << std::setw(20) << "Avg Time/Task (μs)"
              << "\n";
    std::cout << std::string(70, '-') << "\n";
    
    for (size_t num_tasks : task_counts) {
        runtime::ThreadPool pool;
        std::atomic<size_t> counter{0};
        
        auto start = std::chrono::high_resolution_clock::now();
        
        for (size_t i = 0; i < num_tasks; ++i) {
            pool.submit([&counter]() {
                counter++;  // Minimal work
            });
        }
        
        pool.wait();
        
        auto end = std::chrono::high_resolution_clock::now();
        auto duration_ms = std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count();
        auto duration_us = std::chrono::duration_cast<std::chrono::microseconds>(end - start).count();
        
        double tasks_per_sec = (num_tasks * 1000.0) / duration_ms;
        double avg_time_per_task = static_cast<double>(duration_us) / num_tasks;
        
        std::cout << std::setw(15) << num_tasks
                  << std::setw(15) << duration_ms
                  << std::setw(20) << std::fixed << std::setprecision(0) << tasks_per_sec
                  << std::setw(20) << std::fixed << std::setprecision(3) << avg_time_per_task
                  << "\n";
    }
    
    std::cout << "\n";
}

// Benchmark with incrementally increasing work per task
void benchmark_varying_workload() {
    std::cout << "=== Varying Workload Benchmark ===\n";
    std::cout << "Tasks with different amounts of work\n\n";
    
    const size_t num_tasks = 10000;
    const std::vector<int> work_amounts = {10, 100, 1000, 10000};
    
    std::cout << std::left << std::setw(15) << "Work Amount" 
              << std::setw(15) << "Time (ms)"
              << std::setw(20) << "Tasks/sec"
              << "\n";
    std::cout << std::string(50, '-') << "\n";
    
    for (int work : work_amounts) {
        runtime::ThreadPool pool;
        
        auto start = std::chrono::high_resolution_clock::now();
        
        for (size_t i = 0; i < num_tasks; ++i) {
            pool.submit([work]() {
                volatile double result = 0.0;
                for (int j = 0; j < work; ++j) {
                    result += j * 0.001;
                }
            });
        }
        
        pool.wait();
        
        auto end = std::chrono::high_resolution_clock::now();
        auto duration_ms = std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count();
        
        double tasks_per_sec = (num_tasks * 1000.0) / duration_ms;
        
        std::cout << std::setw(15) << work
                  << std::setw(15) << duration_ms
                  << std::setw(20) << std::fixed << std::setprecision(0) << tasks_per_sec
                  << "\n";
    }
    
    std::cout << "\n";
}

// Benchmark submission rate
void benchmark_submission_rate() {
    std::cout << "=== Submission Rate Benchmark ===\n";
    std::cout << "How fast can we submit tasks?\n\n";
    
    runtime::ThreadPool pool;
    const size_t num_tasks = 1000000;
    
    auto start = std::chrono::high_resolution_clock::now();
    
    for (size_t i = 0; i < num_tasks; ++i) {
        pool.submit([]() {
            // Empty task - just measuring submission overhead
        });
    }
    
    auto end = std::chrono::high_resolution_clock::now();
    auto duration_us = std::chrono::duration_cast<std::chrono::microseconds>(end - start).count();
    
    pool.wait();
    
    double submissions_per_sec = (num_tasks * 1000000.0) / duration_us;
    double avg_submission_time = static_cast<double>(duration_us) / num_tasks;
    
    std::cout << "Tasks submitted: " << num_tasks << "\n";
    std::cout << "Total time: " << duration_us / 1000.0 << " ms\n";
    std::cout << "Submissions/sec: " << std::fixed << std::setprecision(0) << submissions_per_sec << "\n";
    std::cout << "Avg time/submission: " << std::fixed << std::setprecision(3) << avg_submission_time << " μs\n\n";
}

int main() {
    std::cout << "\n╔════════════════════════════════════════════════════════╗\n";
    std::cout << "║          Small Tasks Benchmark Suite                  ║\n";
    std::cout << "╚════════════════════════════════════════════════════════╝\n\n";
    
    benchmark_tiny_tasks();
    benchmark_varying_workload();
    benchmark_submission_rate();
    
    return 0;
}