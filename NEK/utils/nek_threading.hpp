#pragma once
#include "../nek.hpp"

#include <atomic>
#include <future>
#include <mutex>
#include <queue>
#include <thread>



namespace nek::utils {

	class ThreadedClassBase {
	public:
		NEK_API virtual void startThread() = 0;
		NEK_API virtual void stopThread() = 0;

		NEK_API std::future<void> sendTaskAsync(std::function<void()> task);
		NEK_API void sendTask(std::function<void()> task);
		template<typename T> T sendTaskWithResult(std::function<T()> task) {
			auto p = std::promise<T>();
			auto f = p.get_future();

			mutexTasks_.lock();
			tasks_.push_back([task, &p] {
				try {
					p.set_value(task());
				}
				catch (...) {
					p.set_exception(std::current_exception());
				}
				});
			mutexTasks_.unlock();
			cvTasks_.notify_one();

			return f.get();
		}

	protected:
		virtual void threadTask();

		std::deque<std::function<void()>> tasks_;
		std::atomic<bool> running_;
		std::mutex mutexTasks_;
		std::condition_variable cvTasks_;
	};



	class ThreadedClass : public ThreadedClassBase {
	public:
		NEK_API ThreadedClass() {};
		NEK_API ~ThreadedClass();

		NEK_API void startThread() override;
		NEK_API void stopThread() override;

	protected:
		std::thread thread_;
		std::mutex mutexThread_;
	};



	class MultiThreadedClass : public ThreadedClassBase {
	public:
		NEK_API MultiThreadedClass() {};
		NEK_API ~MultiThreadedClass();

		NEK_API void startThread() override;
		NEK_API void stopThread() override;

	protected:
		std::vector<std::thread> threads_;
		std::mutex mutexThreads_;
	};

}