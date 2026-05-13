#include "thread_pool.h"

void ThreadPool::start(const size_t thread_count)
{
	for (size_t i = 0; i < thread_count; ++i)
		threads.emplace_back(&ThreadPool::thread_loop, this);
}

void ThreadPool::thread_loop()
{
	while (true)
	{
        std::function<void()> job;
        {
            std::unique_lock lock(queue_mutex);
            mutex_condition.wait(lock, [this] {
                return !jobs.empty() || should_terminate_;
                });
            if (should_terminate_)
                return;
            job = jobs.front();
            jobs.pop();
        }
        job();
	}
}

void ThreadPool::queue_job(const std::function<void()>& job)
{
	{
		std::unique_lock lock(queue_mutex);
		jobs.push(job);
	}
	mutex_condition.notify_one();
}

bool ThreadPool::busy() 
{
    bool pool_busy;
    {
        std::unique_lock lock(queue_mutex);
		pool_busy = !jobs.empty();
    }
    return pool_busy;
}

void ThreadPool::stop() {
    {
        std::unique_lock lock(queue_mutex);
        should_terminate_ = true;
    }
    mutex_condition.notify_all();
    for (std::thread& active_thread : threads)
        active_thread.join();
    threads.clear();
}

