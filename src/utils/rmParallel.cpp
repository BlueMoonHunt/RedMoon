#include "rmParallel.h"
#include <thread>
#include <future>
#include <vector>

void rm::parExecution(size_t totalWork, std::function<void(size_t,size_t)> function) {
    // Determine the number of threads to use 
    std::size_t num_threads = std::thread::hardware_concurrency();
    std::vector<std::future<void>> futures;
    std::size_t work_per_thread = totalWork / num_threads;
    // Spawn threads for parallel processing 

    for (std::size_t t = 0; t < num_threads; ++t) {
        std::size_t start = t * work_per_thread;
        std::size_t end = (t == num_threads - 1) ? totalWork : start + work_per_thread;
        futures.push_back(std::async(std::launch::async, function, start, end));
    }
    
    // Wait for all threads to complete
    for (auto& future : futures) {
        future.get();
    }
}