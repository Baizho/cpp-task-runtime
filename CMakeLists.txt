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
)

# Public include directory
target_include_directories(runtime
    PUBLIC
        ${PROJECT_SOURCE_DIR}/include
)

# Threads library (needed later anyway)
find_package(Threads REQUIRED)
target_link_libraries(runtime
    PUBLIC Threads::Threads
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
