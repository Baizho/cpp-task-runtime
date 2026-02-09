#include <runtime/thread_pool.h>
#include <iostream>
#include <chrono>
#include <iomanip>
#include <vector>
#include <algorithm>
#include <numeric>
#include <cmath>

// Measure time from submission to execution
void benchmark_submission_latency() {
    std::cout << "=== Task Submission Latency Benchmark ===\n";
    std::cout << "Time from submit() to task execution start\n\n";
    
    runtime::ThreadPool pool;
    const size_t num_samples = 10000;
    std::vector<long long> latencies;
    latencies.reserve(num_samples);
    
    std::mutex latencies_mutex;
    
    for (size_t i = 0; i < num_samples; ++i) {
        auto submit_time = std::chrono::high_resolution_clock::now();
        
        pool.submit([submit_time, &latencies, &latencies_mutex]() {
            auto exec_time = std::chrono::high_resolution_clock::now();
            auto latency = std::chrono::duration_cast<std::chrono::microseconds>(
                exec_time - submit_time).count();
            
            std::lock_guard<std::mutex> lock(latencies_mutex);
            latencies.push_back(latency);
        });
        
        // Small delay to avoid overwhelming the queue
        std::this_thread::sleep_for(std::chrono::microseconds(10));
    }
    
    pool.wait();
    
    // Calculate statistics
    std::sort(latencies.begin(), latencies.end());
    
    double mean = std::accumulate(latencies.begin(), latencies.end(), 0.0) / latencies.size();
    double median = latencies[latencies.size() / 2];
    double p95 = latencies[static_cast<size_t>(latencies.size() * 0.95)];
    double p99 = latencies[static_cast<size_t>(latencies.size() * 0.99)];
    double min = latencies.front();
    double max = latencies.back();
    
    std::cout << "Samples: " << num_samples << "\n";
    std::cout << std::fixed << std::setprecision(2);
    std::cout << "Mean:    " << mean << " μs\n";
    std::cout << "Median:  " << median << " μs\n";
    std::cout << "P95:     " << p95 << " μs\n";
    std::cout << "P99:     " << p99 << " μs\n";
    std::cout << "Min:     " << min << " μs\n";
    std::cout << "Max:     " << max << " μs\n\n";
}

// Measure wait() latency
void benchmark_wait_latency() {
    std::cout << "=== Wait Latency Benchmark ===\n";
    std::cout << "Time for wait() to return after last task completes\n\n";
    
    const std::vector<size_t> task_counts = {10, 100, 1000, 10000};
    
    std::cout << std::left << std::setw(15) << "Tasks" 
              << std::setw(20) << "Wait Latency (μs)"
              << "\n";
    std::cout << std::string(35, '-') << "\n";
    
    for (size_t num_tasks : task_counts) {
        runtime::ThreadPool pool;
        std::atomic<std::chrono::high_resolution_clock::time_point*> last_completion{nullptr};
        
        for (size_t i = 0; i < num_tasks; ++i) {
            pool.submit([&last_completion]() {
                auto* t = new std::chrono::high_resolution_clock::time_point(
                    std::chrono::high_resolution_clock::now());
                last_completion.store(t, std::memory_order_release);
            });
        }
        
        auto wait_start = std::chrono::high_resolution_clock::now();
        pool.wait();
        auto wait_end = std::chrono::high_resolution_clock::now();
        
        auto* last = last_completion.load(std::memory_order_acquire);
        
        auto latency = std::chrono::duration_cast<std::chrono::microseconds>(
            wait_end - (last ? *last : wait_start)).count();
        
        delete last;
        
        std::cout << std::setw(15) << num_tasks
                  << std::setw(20) << latency
                  << "\n";
    }
    
    std::cout << "\n";
}

// Measure future.get() latency
void benchmark_future_latency() {
    std::cout << "=== Future Get Latency Benchmark ===\n";
    std::cout << "Time from task completion to future.get() return\n\n";
    
    runtime::ThreadPool pool;
    const size_t num_samples = 1000;
    std::vector<long long> latencies;
    
    for (size_t i = 0; i < num_samples; ++i) {
        auto future = pool.submit_task([]() {
            return std::chrono::high_resolution_clock::now();
        });
        
        auto completion_time = future.get();
        auto get_time = std::chrono::high_resolution_clock::now();
        
        auto latency = std::chrono::duration_cast<std::chrono::microseconds>(
            get_time - completion_time).count();
        
        latencies.push_back(latency);
    }
    
    std::sort(latencies.begin(), latencies.end());
    
    double mean = std::accumulate(latencies.begin(), latencies.end(), 0.0) / latencies.size();
    double median = latencies[latencies.size() / 2];
    double p95 = latencies[static_cast<size_t>(latencies.size() * 0.95)];
    
    std::cout << "Samples: " << num_samples << "\n";
    std::cout << std::fixed << std::setprecision(2);
    std::cout << "Mean:    " << mean << " μs\n";
    std::cout << "Median:  " << median << " μs\n";
    std::cout << "P95:     " << p95 << " μs\n\n";
}

// Measure work stealing latency
void benchmark_work_stealing_latency() {
    std::cout << "=== Work Stealing Latency Benchmark ===\n";
    std::cout << "Response time when one thread is overloaded\n\n";
    
    runtime::config::ThreadPoolOptions options;
    options.threads = 4;
    runtime::ThreadPool pool(options);
    
    const size_t tasks_per_test = 100;
    std::atomic<size_t> slow_thread_tasks{0};
    std::vector<long long> response_times;
    
    // Submit many slow tasks to likely overload one queue
    for (size_t i = 0; i < tasks_per_test; ++i) {
        auto submit_time = std::chrono::high_resolution_clock::now();
        
        pool.submit([submit_time, &response_times, &slow_thread_tasks]() {
            slow_thread_tasks++;
            std::this_thread::sleep_for(std::chrono::milliseconds(10));
            
            auto complete_time = std::chrono::high_resolution_clock::now();
            auto response = std::chrono::duration_cast<std::chrono::milliseconds>(
                complete_time - submit_time).count();
            
            response_times.push_back(response);
        });
    }
    
    pool.wait();
    
    std::sort(response_times.begin(), response_times.end());
    
    double mean = std::accumulate(response_times.begin(), response_times.end(), 0.0) / response_times.size();
    double median = response_times[response_times.size() / 2];
    double p95 = response_times[static_cast<size_t>(response_times.size() * 0.95)];
    
    std::cout << "Total tasks: " << tasks_per_test << "\n";
    std::cout << std::fixed << std::setprecision(2);
    std::cout << "Mean response time:   " << mean << " ms\n";
    std::cout << "Median response time: " << median << " ms\n";
    std::cout << "P95 response time:    " << p95 << " ms\n";
    
    const auto& stats = pool.stats();
    std::cout << "\nWork stealing stats:\n";
    std::cout << "  Tasks stolen: " << stats.tasks_stolen << "\n";
    std::cout << "  Steal attempts: " << stats.steal_attempts << "\n";
    std::cout << "  Success rate: " << std::fixed << std::setprecision(1)
              << (100.0 * stats.tasks_stolen / std::max(1ULL, stats.steal_attempts.load())) << "%\n\n";
}

int main() {
    std::cout << "\n╔════════════════════════════════════════════════════════╗\n";
    std::cout << "║          Latency Benchmark Suite                      ║\n";
    std::cout << "╚════════════════════════════════════════════════════════╝\n\n";
    
    benchmark_submission_latency();
    benchmark_wait_latency();
    benchmark_future_latency();
    benchmark_work_stealing_latency();
    
    return 0;
}