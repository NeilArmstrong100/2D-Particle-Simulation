#pragma once

#include <functional>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <vector>
#include <queue>

class ThreadPool
{
private:
	void thread_loop();

	bool should_terminate = false;
	std::mutex queue_mutex;
	std::condition_variable mutex_condition;
	std::vector<std::thread> threads;
	std::queue<std::function<void()>> jobs;
public:
	void start(size_t);
	void queue_job(const std::function<void()>& job);
	void stop();
	bool busy();
};