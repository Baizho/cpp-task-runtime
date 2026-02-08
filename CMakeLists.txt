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

add_executable(thread_pool_test
    test/thread_pool_test
)

add_executable(work_stealing_queue_test
    test/work_stealing_queue_test
)

target_link_libraries(thread_pool_test
    PRIVATE runtime
)

target_link_libraries(work_stealing_queue_test
    PRIVATE runtime
)
