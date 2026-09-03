/**
 * @file ThreadPool.cpp
 * @author Soumyajit C
 * @brief Implementation of ThreadPool for concurrent task execution.
 * @date 2026-09-03
 *
 * The ThreadPool manages a fixed number of worker threads that continuously
 * process tasks from a shared queue. It ensures safe task submission,
 * synchronization, and graceful shutdown.
 */

#include "ThreadPool.h"

/**
 * @brief Constructs a ThreadPool with the given number of threads.
 * @param threads [in] Number of worker threads.
 *
 * Each worker thread waits on the condition variable until tasks are available
 * or the pool is stopped. Tasks are executed in FIFO order.
 */
ThreadPool::ThreadPool(size_t threads) 
{
    m_workers.reserve(threads);

    for (size_t i = 0; i < threads; ++i) 
    {
        m_workers.emplace_back([this]() 
        {
            while (true) 
            {
                std::function<void()> task;
                {
                    std::unique_lock<std::mutex> lock(m_queueMutex);

                    // Wait until either stop flag is set or tasks are available
                    m_cv.wait(lock, [this]() 
                    {
                        return m_stop.load() || !m_tasks.empty();
                    });

                    // Exit if pool is stopped and no tasks remain
                    if (m_stop.load() && m_tasks.empty()) return;

                    task = std::move(m_tasks.front());
                    m_tasks.pop();
                }

                // Execute the retrieved task
                task();
            }
        });
    }
}

/**
 * @brief Destructor. Stops the pool and joins all worker threads.
 *
 * Sets the stop flag, notifies all threads, and ensures they are joined
 * before destruction to prevent dangling threads.
 */
ThreadPool::~ThreadPool() 
{
    m_stop.store(true);
    m_cv.notify_all();

    for (std::thread& worker : m_workers) 
    {
        if (worker.joinable()) 
        {
            worker.join();
        }
    }
}

/**
 * @brief Enqueues a task for execution by the thread pool.
 * @param task [in] Callable object (std::function<void()>).
 *
 * The task is added to the queue and one worker thread is notified.
 */
void ThreadPool::Enqueue(std::function<void()> task) 
{
    {
        std::unique_lock<std::mutex> lock(m_queueMutex);
        m_tasks.push(std::move(task));
    }
    
    m_cv.notify_one();
}