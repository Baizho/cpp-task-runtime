#ifndef PARALLEL_FOR_H
#define PARALLEL_FOR_H

#include <runtime/thread_pool.h>
#include <runtime/config.h>
#include <vector>
#include <future>

namespace runtime {

// Parallel for loop - splits range [start, end) into chunks and processes in parallel
template<typename IndexType, typename Func>
void parallel_for(ThreadPool& pool, IndexType start, IndexType end, Func&& func, size_t chunk_size = config::parallel_alg::chunk_size) {
    if (start >= end) return;
    
    IndexType range = end - start;
    if (range <= static_cast<IndexType>(chunk_size)) {
        // Range too small, just execute sequentially
        for (IndexType i = start; i < end; ++i) {
            func(i);
        }
        return;
    }
    
    // Calculate number of chunks
    size_t num_chunks = (range + chunk_size - 1) / chunk_size;
    std::vector<std::future<void>> futures;
    futures.reserve(num_chunks);
    
    // Submit chunks
    for (size_t chunk = 0; chunk < num_chunks; ++chunk) {
        IndexType chunk_start = start + chunk * chunk_size;
        IndexType chunk_end = std::min(chunk_start + static_cast<IndexType>(chunk_size), end);
        
        futures.push_back(pool.submit_task([chunk_start, chunk_end, &func]() {
            for (IndexType i = chunk_start; i < chunk_end; ++i) {
                func(i);
            }
        }));
    }
    
    // Wait for all chunks to complete
    for (auto& future : futures) {
        future.get();
    }
}

// Overload that creates its own thread pool
template<typename IndexType, typename Func>
void parallel_for(IndexType start, IndexType end, Func&& func, 
                  size_t chunk_size = config::parallel_alg::chunk_size) {
    ThreadPool pool;
    parallel_for(pool, start, end, std::forward<Func>(func), chunk_size);
}

} // namespace runtime

#endif // PARALLEL_FOR_H