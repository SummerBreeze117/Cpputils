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
#include <functional>

class ThreadPool {
	struct Task {
		std::function<void()> func_;
		// TODO: task info
	};
	using TaskPtr = std::shared_ptr<Task>;

public:
	explicit ThreadPool(std::string nameArg = std::string("ThreadPool"), size_t maxTaskNum = 1024)
				: name_(std::move(nameArg)), maxTaskNum_(maxTaskNum) {}

	~ThreadPool()
	{
		if (running_) {
			stop();
		}
	}

	ThreadPool(const ThreadPool&) = delete;
	ThreadPool(ThreadPool &&) = delete;
	ThreadPool& operator = (const ThreadPool&) = delete;
	ThreadPool& operator = (ThreadPool &&) = delete;

	size_t getMaxTaskNum() const { return maxTaskNum_; }
	size_t getRunningTaskNum() const { return runningTaskNum_; }

	void waitForAllDone() {
		std::unique_lock<std::mutex> lc(mutex_);
		// TODO: timeout
		cv_.wait(lc, [&]{ return runningTaskNum_ == 0 && queue_.empty(); });
	}

	void start(int numThread)
	{
		if (running_) return;

		running_ = true;
		threads_.reserve(numThread);
		for (int i = 0; i < numThread; i ++) {
			threads_.emplace_back(std::make_unique<std::thread>([this] { ThreadPool::run(); }));
		}
	}

	void stop()
	{
		{
			std::lock_guard<std::mutex> lc(mutex_);
			running_ = false;
			cv_.notify_all();
		}

		for (auto &thread : threads_) {
			thread->join();
		}
	}

	template<typename F, typename... Args>
	auto exec(F&& f, Args&& ...args) -> std::future<decltype(f(args...))>
	{
		using RetType = decltype(f(args...));
		auto taskPkg = std::make_shared<std::packaged_task<RetType()>>
			(std::bind(std::forward<F>(f), std::forward<Args>(args)...));

		TaskPtr taskPtr = std::make_shared<Task>();
		taskPtr->func_ = [taskPkg] {
			// do task
			(*taskPkg)();
		};
		// insert task
		std::lock_guard<std::mutex> lc(mutex_);
		// TODO: maxTaskNum_
		queue_.push(taskPtr);
		cv_.notify_one();

		return taskPkg->get_future();
	}

private:
	bool get(TaskPtr& task)
	{
		std::unique_lock<std::mutex> lc(mutex_);
		cv_.wait(lc, [&]{ return !queue_.empty() || !running_; });

		if (!running_) {
			return false;
		}

		task = std::move(queue_.front());
		queue_.pop();
		return true;
	}

	// running state for threads in ThreadPool
	void run()
	{
		while (running_) {
			TaskPtr task;
			bool isValidTask = get(task);
			if (isValidTask) {
				runningTaskNum_ ++;
				// fixme: catch exception?
				task->func_();
				runningTaskNum_ --;

				// notify ThreadPool::waitForAllDone()
				if (runningTaskNum_ == 0) {
					std::lock_guard<std::mutex> lc(mutex_);
					if (queue_.empty()) {
						cv_.notify_all();
					}
				}
			}
		}
	}

private:
	std::mutex mutex_;
	std::condition_variable cv_;

	std::string name_;
	std::vector<std::unique_ptr<std::thread>> threads_;
	std::queue<TaskPtr> queue_;
	size_t maxTaskNum_;

	std::atomic<size_t> runningTaskNum_{0};
	bool running_ = false;
};

#endif //THREADPOOL_H
