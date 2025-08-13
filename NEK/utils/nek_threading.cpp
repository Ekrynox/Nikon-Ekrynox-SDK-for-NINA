#include "nek_threading.hpp"



using namespace nek::utils;



//ThreadedClassBase
std::future<void> ThreadedClassBase::sendTaskAsync(std::function<void()> task) {
	auto p = std::make_shared<std::promise<void>>();
	auto f = p->get_future();

	std::lock_guard<std::mutex> lock(mutexTasks_);
	tasks_.push_back([task = std::move(task), p] {
		try {
			task();
			p->set_value();
		}
		catch (...) {
			p->set_exception(std::current_exception());
		}
		});
	cvTasks_.notify_one();

	return f;
}

void ThreadedClassBase::sendTask(std::function<void()> task) {
	return sendTaskAsync(std::move(task)).get();
}

void ThreadedClassBase::threadTask() {
	while (running_) {
		mutexTasks_.lock();
		if (tasks_.size() > 0) {
			auto task = tasks_.front();
			tasks_.pop_front();
			mutexTasks_.unlock();

			task();
		} else {
			mutexTasks_.unlock();

			std::unique_lock lk(mutexTasks_);
			cvTasks_.wait(lk, [this] { return !this->running_ || (this->tasks_.size() > 0); });
			lk.unlock();
		}
	}
}



//ThreadedClass
ThreadedClass::~ThreadedClass() {
	stopThread();
}

void ThreadedClass::startThread() {
	running_ = true;
	mutexThread_.lock();
	thread_ = std::thread([this] { this->threadTask(); });
	mutexThread_.unlock();
}

void ThreadedClass::stopThread() {
	running_ = false;
	mutexThread_.lock();
	cvTasks_.notify_all();
	if (std::this_thread::get_id() != thread_.get_id() && thread_.joinable()) {
		thread_.join();
		thread_.~thread();
	}
	mutexThread_.unlock();
}



//MultiThreadedClass
MultiThreadedClass::~MultiThreadedClass() {
	stopThread();
}

void MultiThreadedClass::startThread() {
	mutexThreads_.lock();
	running_ = true;
	threads_.push_back(std::thread([this] { this->threadTask(); }));
	mutexThreads_.unlock();
}

void MultiThreadedClass::stopThread() {
	running_ = false;
	cvTasks_.notify_all();


	mutexThreads_.lock();
	std::thread::id threadId = std::this_thread::get_id();
	std::thread thOwn;
	while (threads_.size() > 0) {
		if (threadId == threads_.back().get_id()) {
			std::swap(thOwn, threads_.back());
			thOwn.detach();
		}
		else if (threads_.back().joinable()) {
			threads_.back().join();
		}
		threads_.pop_back();
	}
	mutexThreads_.unlock();

}