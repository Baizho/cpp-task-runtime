#ifndef PARALLEL_REDUCE_H
#define PARALLEL_REDUCE_H

#include <runtime/thread_pool.h>
#include <runtime/config.h>
#include <vector>
#include <future>
#include <functional>

namespace runtime {

// Parallel reduction - combines elements in range [start, end) using binary operation
template<typename IndexType, typename T, typename Func, typename ReduceOp>
T parallel_reduce(ThreadPool& pool, IndexType start, IndexType end, T init, 
                  Func&& map_func, ReduceOp&& reduce_op,
                  size_t chunk_size = config::parallel_alg::chunk_size) {
    if (start >= end) return init;
    
    IndexType range = end - start;
    if (range <= static_cast<IndexType>(chunk_size)) {
        // Range too small, execute sequentially
        T result = init;
        for (IndexType i = start; i < end; ++i) {
            result = reduce_op(result, map_func(i));
        }
        return result;
    }
    
    // Calculate number of chunks
    size_t num_chunks = (range + chunk_size - 1) / chunk_size;
    std::vector<std::future<T>> futures;
    futures.reserve(num_chunks);
    
    // Submit chunks - each chunk produces a partial result
    for (size_t chunk = 0; chunk < num_chunks; ++chunk) {
        IndexType chunk_start = start + chunk * chunk_size;
        IndexType chunk_end = std::min(chunk_start + static_cast<IndexType>(chunk_size), end);
        
        futures.push_back(pool.submit_task([chunk_start, chunk_end, init, &map_func, &reduce_op]() -> T {
            T partial = init;
            for (IndexType i = chunk_start; i < chunk_end; ++i) {
                partial = reduce_op(partial, map_func(i));
            }
            return partial;
        }));
    }
    
    // Combine all partial results
    T final_result = init;
    for (auto& future : futures) {
        final_result = reduce_op(final_result, future.get());
    }
    
    return final_result;
}

// Overload that creates its own thread pool
template<typename IndexType, typename T, typename Func, typename ReduceOp>
T parallel_reduce(IndexType start, IndexType end, T init, 
                  Func&& map_func, ReduceOp&& reduce_op,
                  size_t chunk_size = config::parallel_alg::chunk_size) {
    ThreadPool pool;
    return parallel_reduce(pool, start, end, init, 
                          std::forward<Func>(map_func), 
                          std::forward<ReduceOp>(reduce_op), 
                          chunk_size);
}

// Simplified version - just sum the results of map_func
template<typename IndexType, typename T, typename Func>
T parallel_map_reduce(ThreadPool& pool, IndexType start, IndexType end, T init, Func&& map_func,
                      size_t chunk_size = config::parallel_alg::chunk_size) {
    return parallel_reduce(pool, start, end, init, 
                          std::forward<Func>(map_func),
                          std::plus<T>(),
                          chunk_size);
}

} // namespace runtime

#endif // PARALLEL_REDUCE_H