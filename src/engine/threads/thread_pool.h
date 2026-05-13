#pragma once

#include <functional>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <vector>
#include <queue>

// Source - https://stackoverflow.com/a/32593825
// Posted by PhD AP EcE, modified by community. See post 'Timeline' for change history
// Retrieved 2026-05-13, License - CC BY-SA 4.0 : https://creativecommons.org/licenses/by-sa/4.0/legalcode.txt

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