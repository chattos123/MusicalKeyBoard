/**
 * @file ThreadPool.h
 * @author Soumyajit C
 * @brief A lightweight thread pool for concurrent task execution.
 * @date 2026-09-03
 *
 * The ThreadPool class manages a fixed number of worker threads that
 * execute tasks submitted to a shared queue. It provides a simple
 * interface for enqueuing tasks and ensures proper cleanup on destruction.
 */

#pragma once

#include <vector>
#include <queue>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <atomic>
#include <functional>

/**
 * @class ThreadPool
 * @brief A fixed-size pool of worker threads for executing tasks concurrently.
 *
 * Responsibilities:
 * - Maintain a queue of tasks (std::function<void()>).
 * - Run worker threads that continuously process tasks from the queue.
 * - Provide an interface to enqueue tasks.
 * - Ensure graceful shutdown and cleanup of threads.
 *
 * Usage:
 * - Construct with a desired number of threads.
 * - Call Enqueue(task) to submit work.
 * - Destructor ensures all threads are joined and resources released.
 */
class ThreadPool 
{
private:
    std::vector<std::thread> m_workers;              ///< Worker threads
    std::queue<std::function<void()>> m_tasks;       ///< Task queue
    std::mutex m_queueMutex;                         ///< Protects task queue
    std::condition_variable m_cv;                    ///< Condition variable for task notification
    std::atomic<bool> m_stop{ false };               ///< Stop flag for shutdown

public:
    /**
     * @brief Constructs a ThreadPool with the given number of threads.
     * @param threads [in] Number of worker threads (default: 8).
     */
    explicit ThreadPool(size_t threads = 8);

    /**
     * @brief Destructor. Stops the pool and joins all threads.
     */
    ~ThreadPool();

    /**
     * @brief Enqueues a task for execution by the thread pool.
     * @param task [in] Callable object (std::function<void()>).
     *
     * The task will be executed by one of the worker threads
     * when resources become available.
     */
    void Enqueue(std::function<void()> task);
};