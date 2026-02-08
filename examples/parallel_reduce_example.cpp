#include <runtime/thread_pool.h>
#include <runtime/parallel_reduce.h>
#include <iostream>
#include <vector>
#include <chrono>
#include <cmath>
#include <numeric>

int main() {
    std::cout << "=== Parallel Reduce Examples ===\n\n";
    
    runtime::ThreadPool pool;
    
    // Example 1: Sum of squares
    std::cout << "1. Sum of squares from 1 to 10,000,000:\n";
    
    auto start = std::chrono::high_resolution_clock::now();
    auto sum = runtime::parallel_reduce(
        pool,
        0, 10000000,
        0LL,  // Initial value
        [](int i) { return static_cast<long long>(i) * i; },  // Map function
        std::plus<long long>()  // Reduce operation
    );
    auto end = std::chrono::high_resolution_clock::now();
    
    std::cout << "   Result: " << sum << "\n";
    std::cout << "   Time: " << std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count() 
              << " ms\n\n";
    
    // Example 2: Find maximum
    std::cout << "2. Find maximum value:\n";
    std::vector<int> data(1000000);
    for (size_t i = 0; i < data.size(); ++i) {
        data[i] = std::sin(i) * 1000;
    }
    
    auto max_val = runtime::parallel_reduce(
        pool,
        size_t(0), data.size(),
        std::numeric_limits<int>::min(),
        [&data](size_t i) { return data[i]; },
        [](int a, int b) { return std::max(a, b); }
    );
    
    std::cout << "   Maximum value: " << max_val << "\n\n";
    
    // Example 3: Dot product
    std::cout << "3. Vector dot product:\n";
    std::vector<double> vec1(10000000);
    std::vector<double> vec2(10000000);
    
    for (size_t i = 0; i < vec1.size(); ++i) {
        vec1[i] = i * 0.001;
        vec2[i] = (vec1.size() - i) * 0.001;
    }
    
    start = std::chrono::high_resolution_clock::now();
    double dot_product = runtime::parallel_reduce(
        pool,
        size_t(0), vec1.size(),
        0.0,
        [&](size_t i) { return vec1[i] * vec2[i]; },
        std::plus<double>()
    );
    end = std::chrono::high_resolution_clock::now();
    
    std::cout << "   Dot product: " << dot_product << "\n";
    std::cout << "   Time: " << std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count() 
              << " ms\n\n";
    
    // Example 4: Count elements matching condition
    std::cout << "4. Count even numbers:\n";
    int count = runtime::parallel_reduce(
        pool,
        0, 10000000,
        0,
        [](int i) { return (i % 2 == 0) ? 1 : 0; },
        std::plus<int>()
    );
    
    std::cout << "   Even numbers from 0 to 9,999,999: " << count << "\n";
    
    return 0;
}