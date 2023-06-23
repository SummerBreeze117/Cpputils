//
// Created by co2ma on 2023/6/21.
//

#ifndef THREADPOOL_H
#define THREADPOOL_H

#include <vector>
#include <queue>
#include <memory>
#include <thread>
#include <future>
#include <mutex>
#include <condition_variable>

class ThreadPool {
	struct Task{
		std::function<void()> func_;
	};
	using TaskPtr = std::shared_ptr<Task>;

public:
	explicit ThreadPool(std::string nameArg = std::string("ThreadPool"),
						size_t maxTaskNum = 1024);
	~ThreadPool();

	ThreadPool(const ThreadPool&) = delete;
	ThreadPool(ThreadPool &&) = delete;
	ThreadPool& operator = (const ThreadPool&) = delete;
	ThreadPool& operator = (ThreadPool &&) = delete;

	size_t getMaxTaskNum();
	size_t getRunningTaskNum();
	void waitForAllDone();

	void start(int numThread);
	void stop();

	template<typename F, typename... Args>
	auto exec(F&& f, Args&& ...args) -> std::future<decltype(f(args...))>;
private:
	bool get(Task& task);

	// 线程运行态
	void run();

	std::mutex mutex_;
	std::condition_variable notEmpty_;

	std::string name_;
	// Task threadInitCallback_;
	std::vector<std::unique_ptr<std::thread>> threads_;
	std::queue<Task> queue_;
	size_t maxTaskNum_;

	// std::atomic<size_t> runningTaskNum_;
	bool running_;
};

#endif //THREADPOOL_H
