cmake_minimum_required(VERSION 3.16)

project(cpp_task_runtime
    VERSION 1.0
    LANGUAGES CXX
)

# C++ standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# ==============================
# Runtime Library
# ==============================

add_library(runtime
    src/thread_pool.cpp
    src/work_stealing_queue.cpp
)

# Public include directory
target_include_directories(runtime
    PUBLIC
        ${PROJECT_SOURCE_DIR}/include
)

# ==============================
# Examples
# ==============================

add_executable(basic_usage
    examples/basic_usage.cpp
)

target_link_libraries(basic_usage
    PRIVATE runtime
)

# ==============================

add_executable(future_example
    examples/future_example.cpp
)

target_link_libraries(future_example
    PRIVATE runtime
)

# ==============================

add_executable(parallel_for_example
    examples/parallel_for_example.cpp
)

target_link_libraries(parallel_for_example
    PRIVATE runtime
)

# ==============================

add_executable(parallel_reduce_example
    examples/parallel_reduce_example.cpp
)

target_link_libraries(parallel_reduce_example
    PRIVATE runtime
)

# ==============================
# Tests
# ==============================

add_executable(thread_pool_test
    tests/thread_pool_test.cpp
)

target_link_libraries(thread_pool_test
    PRIVATE runtime
)

# ==============================

add_executable(work_stealing_queue_test
    tests/work_stealing_queue_test.cpp
)

target_link_libraries(work_stealing_queue_test
    PRIVATE runtime
)

# ==============================

add_executable(shutdown_test
    tests/shutdown_test.cpp
)

target_link_libraries(shutdown_test
    PRIVATE runtime
)

# ==============================
# Benchmarks
# ==============================

add_executable(heavy_tasks
    benchmarks/heavy_tasks.cpp
)

target_link_libraries(heavy_tasks
    PRIVATE runtime
)

# ==============================

add_executable(latency_benchmark
    benchmarks/latency_benchmark.cpp
)

target_link_libraries(latency_benchmark
    PRIVATE runtime
)

# ==============================

add_executable(scaling_benchmark
    benchmarks/scaling_benchmark.cpp
)

target_link_libraries(scaling_benchmark
    PRIVATE runtime
)

# ==============================

add_executable(small_tasks
    benchmarks/small_tasks.cpp
)

target_link_libraries(small_tasks
    PRIVATE runtime
)

# ==============================