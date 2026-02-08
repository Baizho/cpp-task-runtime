#include <runtime/thread_pool.h>
#include <iostream>
#include <vector>
#include <numeric>
#include <cmath>

// Example: Parallel computation with futures
int main() {
    std::cout << "=== Future-based Parallel Computation ===\n\n";
    
    runtime::ThreadPool pool;
    
    // Example 1: Compute multiple values in parallel
    std::cout << "1. Computing squares of numbers 1-10 in parallel:\n";
    std::vector<std::future<int>> futures;
    
    for (int i = 1; i <= 10; ++i) {
        futures.push_back(pool.submit_task([i]() {
            return i * i;
        }));
    }
    
    std::cout << "   Results: ";
    for (auto& f : futures) {
        std::cout << f.get() << " ";
    }
    std::cout << "\n\n";
    
    // Example 2: Complex computation
    std::cout << "2. Computing expensive operations:\n";
    auto compute_pi_chunk = [](int iterations) -> double {
        double sum = 0.0;
        for (int i = 0; i < iterations; ++i) {
            double x = (i + 0.5) / iterations;
            sum += 4.0 / (1.0 + x * x);
        }
        return sum / iterations;
    };
    
    const int chunks = 8;
    const int iterations_per_chunk = 10000000;
    std::vector<std::future<double>> pi_futures;
    
    for (int i = 0; i < chunks; ++i) {
        pi_futures.push_back(pool.submit_task(compute_pi_chunk, iterations_per_chunk));
    }
    
    double pi_estimate = 0.0;
    for (auto& f : pi_futures) {
        pi_estimate += f.get();
    }
    pi_estimate /= chunks;
    
    std::cout << "   Estimated π: " << pi_estimate << "\n";
    std::cout << "   Actual π:    " << M_PI << "\n";
    std::cout << "   Error:       " << std::abs(pi_estimate - M_PI) << "\n\n";
    
    // Example 3: Exception handling
    std::cout << "3. Exception handling with futures:\n";
    auto future_with_exception = pool.submit_task([]() -> int {
        throw std::runtime_error("Something went wrong!");
        return 42;
    });
    
    try {
        future_with_exception.get();
    } catch (const std::exception& e) {
        std::cout << "   Caught exception: " << e.what() << "\n";
    }
    
    return 0;
}