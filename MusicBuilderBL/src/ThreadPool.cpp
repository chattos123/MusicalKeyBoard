#include "ThreadPool.h"

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

                    m_cv.wait(lock, [this]() 
                    {
                        return m_stop.load() || !m_tasks.empty();
                    });

                    if (m_stop.load() && m_tasks.empty()) return;

                    task = std::move(m_tasks.front());
                    m_tasks.pop();
                }

                task();
            }
        });
    }
}

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

void ThreadPool::Enqueue(std::function<void()> task) 
{
    {
        std::unique_lock<std::mutex> lock(m_queueMutex);
        m_tasks.push(std::move(task));
    }
    
    m_cv.notify_one();
}