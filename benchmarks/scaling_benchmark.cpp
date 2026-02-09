#include <runtime/thread_pool.h>
#include <iostream>
#include <chrono>
#include <vector>
#include <iomanip>
#include <cmath>

// Benchmark that tests how performance scales with number of threads
void benchmark_thread_scaling() {
    const size_t num_tasks = 100000;
    const int work_per_task = 1000;
    
    std::cout << "=== Thread Scaling Benchmark ===\n";
    std::cout << "Tasks: " << num_tasks << ", Work per task: " << work_per_task << " iterations\n\n";
    std::cout << std::left << std::setw(10) << "Threads" 
              << std::setw(15) << "Time (ms)" 
              << std::setw(15) << "Tasks/sec"
              << std::setw(15) << "Speedup"
              << "\n";
    std::cout << std::string(55, '-') << "\n";
    
    double baseline_time = 0.0;
    
    for (size_t num_threads = 1; num_threads <= std::thread::hardware_concurrency(); num_threads *= 2) {
        runtime::config::ThreadPoolOptions options;
        options.threads = num_threads;
        
        runtime::ThreadPool pool(options);
        std::atomic<size_t> completed{0};
        
        auto start = std::chrono::high_resolution_clock::now();
        
        for (size_t i = 0; i < num_tasks; ++i) {
            pool.submit([&completed, work_per_task]() {
                // Simulate CPU work
                volatile double result = 0.0;
                for (int j = 0; j < work_per_task; ++j) {
                    result += std::sqrt(j) * std::sin(j);
                }
                completed++;
            });
        }
        
        pool.wait();
        
        auto end = std::chrono::high_resolution_clock::now();
        auto duration_ms = std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count();
        
        if (num_threads == 1) {
            baseline_time = duration_ms;
        }
        
        double tasks_per_sec = (num_tasks * 1000.0) / duration_ms;
        double speedup = baseline_time / duration_ms;
        
        std::cout << std::setw(10) << num_threads 
                  << std::setw(15) << duration_ms
                  << std::setw(15) << std::fixed << std::setprecision(0) << tasks_per_sec
                  << std::setw(15) << std::fixed << std::setprecision(2) << speedup
                  << "\n";
    }
    
    std::cout << "\n";
}

// Benchmark weak scaling (constant work per thread)
void benchmark_weak_scaling() {
    const size_t tasks_per_thread = 10000;
    const int work_per_task = 500;
    
    std::cout << "=== Weak Scaling Benchmark ===\n";
    std::cout << "Tasks per thread: " << tasks_per_thread << ", Work per task: " << work_per_task << "\n\n";
    std::cout << std::left << std::setw(10) << "Threads" 
              << std::setw(15) << "Total Tasks"
              << std::setw(15) << "Time (ms)" 
              << std::setw(15) << "Efficiency"
              << "\n";
    std::cout << std::string(55, '-') << "\n";
    
    double baseline_time = 0.0;
    
    for (size_t num_threads = 1; num_threads <= std::thread::hardware_concurrency(); num_threads *= 2) {
        runtime::config::ThreadPoolOptions options;
        options.threads = num_threads;
        
        runtime::ThreadPool pool(options);
        size_t total_tasks = tasks_per_thread * num_threads;
        
        auto start = std::chrono::high_resolution_clock::now();
        
        for (size_t i = 0; i < total_tasks; ++i) {
            pool.submit([work_per_task]() {
                volatile double result = 0.0;
                for (int j = 0; j < work_per_task; ++j) {
                    result += std::sqrt(j) * std::sin(j);
                }
            });
        }
        
        pool.wait();
        
        auto end = std::chrono::high_resolution_clock::now();
        auto duration_ms = std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count();
        
        if (num_threads == 1) {
            baseline_time = duration_ms;
        }
        
        // Ideal weak scaling: time stays constant as threads/work increases
        double efficiency = (baseline_time / duration_ms) * 100.0;
        
        std::cout << std::setw(10) << num_threads 
                  << std::setw(15) << total_tasks
                  << std::setw(15) << duration_ms
                  << std::setw(15) << std::fixed << std::setprecision(1) << efficiency << "%"
                  << "\n";
    }
    
    std::cout << "\n";
}

int main() {
    std::cout << "\n╔════════════════════════════════════════════════════════╗\n";
    std::cout << "║          Thread Pool Scaling Benchmarks                ║\n";
    std::cout << "╚════════════════════════════════════════════════════════╝\n\n";
    std::cout << "Hardware threads: " << std::thread::hardware_concurrency() << "\n\n";
    
    benchmark_thread_scaling();
    benchmark_weak_scaling();
    
    return 0;
}