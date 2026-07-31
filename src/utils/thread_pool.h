#pragma once

#include <vector>
#include <thread>
#include <queue>
#include <mutex>
#include <condition_variable>
#include <functional>
#include <atomic>

namespace revres {
namespace utils {

class ThreadPool {
public:
    // Create a thread pool with the specified number of threads
    explicit ThreadPool(size_t num_threads);
    
    // Disable copy/move
    ThreadPool(const ThreadPool&) = delete;
    ThreadPool& operator=(const ThreadPool&) = delete;
    
    // Destructor stops all threads
    ~ThreadPool();

    // Enqueue a task (a function with no arguments returning void)
    void enqueue(std::function<void()> task);

    // Stop the thread pool, wait for all threads to finish current tasks
    void shutdown();

private:
    std::vector<std::thread> workers_;
    std::queue<std::function<void()>> tasks_;
    
    std::mutex queue_mutex_;
    std::condition_variable condition_;
    std::atomic<bool> stop_;
};

} // namespace utils
} // namespace revres
