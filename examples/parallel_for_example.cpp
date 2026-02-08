#include <runtime/thread_pool.h>
#include <runtime/parallel_for.h>
#include <iostream>
#include <vector>
#include <chrono>
#include <cmath>

int main() {
    std::cout << "=== Parallel For Loop Examples ===\n\n";
    
    runtime::ThreadPool pool;
    
    // Example 1: Simple parallel iteration
    std::cout << "1. Filling array with squares:\n";
    const size_t n = 1000;
    std::vector<int> data(n);
    
    auto start = std::chrono::high_resolution_clock::now();
    runtime::parallel_for(pool, size_t(0), n, [&data](size_t i) {
        data[i] = i * i;
    });
    auto end = std::chrono::high_resolution_clock::now();
    
    std::cout << "   First 10 values: ";
    for (size_t i = 0; i < 10; ++i) {
        std::cout << data[i] << " ";
    }
    std::cout << "\n";
    std::cout << "   Time: " << std::chrono::duration_cast<std::chrono::microseconds>(end - start).count() 
              << " μs\n\n";
    
    // Example 2: Image processing simulation
    std::cout << "2. Image processing simulation (1000x1000 pixels):\n";
    const size_t width = 1000;
    const size_t height = 1000;
    std::vector<double> image(width * height);
    
    // Initialize with some pattern
    for (size_t i = 0; i < width * height; ++i) {
        image[i] = std::sin(i * 0.01);
    }
    
    start = std::chrono::high_resolution_clock::now();
    
    // Apply filter in parallel
    runtime::parallel_for(pool, size_t(0), height, [&](size_t y) {
        for (size_t x = 0; x < width; ++x) {
            size_t idx = y * width + x;
            // Simple blur-like operation
            image[idx] = std::sqrt(std::abs(image[idx])) * 0.5;
        }
    }, 10);  // Custom chunk size
    
    end = std::chrono::high_resolution_clock::now();
    std::cout << "   Processing time: " 
              << std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count() 
              << " ms\n\n";
    
    // Example 3: Compare with sequential
    std::cout << "3. Sequential vs Parallel comparison:\n";
    std::vector<double> seq_data(10000000);
    std::vector<double> par_data(10000000);
    
    // Sequential
    start = std::chrono::high_resolution_clock::now();
    for (size_t i = 0; i < seq_data.size(); ++i) {
        seq_data[i] = std::sin(i) * std::cos(i);
    }
    end = std::chrono::high_resolution_clock::now();
    auto seq_time = std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count();
    
    // Parallel
    start = std::chrono::high_resolution_clock::now();
    runtime::parallel_for(pool, size_t(0), par_data.size(), [&par_data](size_t i) {
        par_data[i] = std::sin(i) * std::cos(i);
    });
    end = std::chrono::high_resolution_clock::now();
    auto par_time = std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count();
    
    std::cout << "   Sequential: " << seq_time << " ms\n";
    std::cout << "   Parallel:   " << par_time << " ms\n";
    std::cout << "   Speedup:    " << (double)seq_time / par_time << "x\n";
    
    return 0;
}