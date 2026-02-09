#include <runtime/thread_pool.h>
#include <iostream>
#include <chrono>
#include <iomanip>
#include <vector>
#include <cmath>
#include <random>

// Heavy CPU-bound computation
double compute_intensive_task(int iterations) {
    double result = 0.0;
    for (int i = 0; i < iterations; ++i) {
        result += std::sqrt(i) * std::sin(i) * std::cos(i) + std::log(i + 1);
    }
    return result;
}

// Matrix multiplication (heavy computation)
std::vector<std::vector<double>> matrix_multiply(
    const std::vector<std::vector<double>>& A,
    const std::vector<std::vector<double>>& B) {
    
    size_t n = A.size();
    size_t m = B[0].size();
    size_t p = B.size();
    
    std::vector<std::vector<double>> C(n, std::vector<double>(m, 0.0));
    
    for (size_t i = 0; i < n; ++i) {
        for (size_t j = 0; j < m; ++j) {
            for (size_t k = 0; k < p; ++k) {
                C[i][j] += A[i][k] * B[k][j];
            }
        }
    }
    
    return C;
}

void benchmark_cpu_intensive() {
    std::cout << "=== CPU-Intensive Tasks Benchmark ===\n";
    std::cout << "Heavy mathematical computations\n\n";
    
    const std::vector<int> task_counts = {10, 50, 100, 200};
    const int iterations = 10000000;
    
    std::cout << std::left << std::setw(15) << "Tasks" 
              << std::setw(15) << "Time (ms)"
              << std::setw(20) << "Throughput (tasks/s)"
              << "\n";
    std::cout << std::string(50, '-') << "\n";
    
    for (int num_tasks : task_counts) {
        runtime::ThreadPool pool;
        std::vector<std::future<double>> futures;
        
        auto start = std::chrono::high_resolution_clock::now();
        
        for (int i = 0; i < num_tasks; ++i) {
            futures.push_back(pool.submit_task(compute_intensive_task, iterations));
        }
        
        // Wait for all and collect results
        double sum = 0.0;
        for (auto& f : futures) {
            sum += f.get();
        }
        
        auto end = std::chrono::high_resolution_clock::now();
        auto duration_ms = std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count();
        
        double throughput = (num_tasks * 1000.0) / duration_ms;
        
        std::cout << std::setw(15) << num_tasks
                  << std::setw(15) << duration_ms
                  << std::setw(20) << std::fixed << std::setprecision(2) << throughput
                  << "\n";
    }
    
    std::cout << "\n";
}

void benchmark_parallel_matrix_multiply() {
    std::cout << "=== Parallel Matrix Multiplication Benchmark ===\n";
    std::cout << "Multiple matrix multiplications in parallel\n\n";
    
    const size_t matrix_size = 200;
    const std::vector<int> num_multiplications = {1, 5, 10, 20};
    
    // Generate random matrices
    std::mt19937 rng(42);
    std::uniform_real_distribution<double> dist(0.0, 1.0);
    
    auto generate_matrix = [&]() {
        std::vector<std::vector<double>> m(matrix_size, std::vector<double>(matrix_size));
        for (auto& row : m) {
            for (auto& val : row) {
                val = dist(rng);
            }
        }
        return m;
    };
    
    std::cout << std::left << std::setw(20) << "Multiplications" 
              << std::setw(15) << "Time (ms)"
              << std::setw(20) << "Matrices/sec"
              << "\n";
    std::cout << std::string(55, '-') << "\n";
    
    for (int num_mults : num_multiplications) {
        runtime::ThreadPool pool;
        std::vector<std::future<std::vector<std::vector<double>>>> futures;
        
        // Pre-generate matrices
        std::vector<std::vector<std::vector<double>>> matrices_A;
        std::vector<std::vector<std::vector<double>>> matrices_B;
        for (int i = 0; i < num_mults; ++i) {
            matrices_A.push_back(generate_matrix());
            matrices_B.push_back(generate_matrix());
        }
        
        auto start = std::chrono::high_resolution_clock::now();
        
        for (int i = 0; i < num_mults; ++i) {
            futures.push_back(pool.submit_task(matrix_multiply, 
                                               std::ref(matrices_A[i]), 
                                               std::ref(matrices_B[i])));
        }
        
        // Wait for all
        for (auto& f : futures) {
            f.get();
        }
        
        auto end = std::chrono::high_resolution_clock::now();
        auto duration_ms = std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count();
        
        double throughput = (num_mults * 1000.0) / duration_ms;
        
        std::cout << std::setw(20) << num_mults
                  << std::setw(15) << duration_ms
                  << std::setw(20) << std::fixed << std::setprecision(2) << throughput
                  << "\n";
    }
    
    std::cout << "\n";
}

void benchmark_mixed_workload() {
    std::cout << "=== Mixed Heavy Workload Benchmark ===\n";
    std::cout << "Combination of different heavy tasks\n\n";
    
    runtime::ThreadPool pool;
    const int num_tasks = 100;
    
    auto start = std::chrono::high_resolution_clock::now();
    
    std::vector<std::future<double>> futures;
    
    for (int i = 0; i < num_tasks; ++i) {
        if (i % 3 == 0) {
            // Heavy math
            futures.push_back(pool.submit_task(compute_intensive_task, 5000000));
        } else if (i % 3 == 1) {
            // Medium math with different operations
            futures.push_back(pool.submit_task([](int n) {
                double sum = 0.0;
                for (int j = 0; j < n; ++j) {
                    sum += std::pow(j, 1.5) / (j + 1.0);
                }
                return sum;
            }, 1000000));
        } else {
            // Trigonometric operations
            futures.push_back(pool.submit_task([](int n) {
                double result = 0.0;
                for (int j = 0; j < n; ++j) {
                    result += std::tan(j * 0.001) + std::atan(j * 0.001);
                }
                return result;
            }, 500000));
        }
    }
    
    double total = 0.0;
    for (auto& f : futures) {
        total += f.get();
    }
    
    auto end = std::chrono::high_resolution_clock::now();
    auto duration_ms = std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count();
    
    std::cout << "Total tasks: " << num_tasks << "\n";
    std::cout << "Total time: " << duration_ms << " ms\n";
    std::cout << "Throughput: " << std::fixed << std::setprecision(2) 
              << (num_tasks * 1000.0) / duration_ms << " tasks/sec\n\n";
}

int main() {
    std::cout << "\n╔════════════════════════════════════════════════════════╗\n";
    std::cout << "║          Heavy Tasks Benchmark Suite                  ║\n";
    std::cout << "╚════════════════════════════════════════════════════════╝\n\n";
    std::cout << "Hardware threads: " << std::thread::hardware_concurrency() << "\n\n";
    
    benchmark_cpu_intensive();
    benchmark_parallel_matrix_multiply();
    benchmark_mixed_workload();
    
    return 0;
}