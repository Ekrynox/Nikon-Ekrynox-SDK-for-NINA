#include "nek_threading.hpp"



using namespace nek::utils;



//ThreadedClassBase
ThreadedClassBase::ThreadedClassBase() {
	running_ = true;
}

void ThreadedClassBase::sendTaskAsync(std::function<void()> task) {
	mutexTasks_.lock();

	tasks_.push_back(task);

	mutexTasks_.unlock();
	cvTasks_.notify_one();
}

void ThreadedClassBase::sendTask(std::function<void()> task) {
	std::promise<void> p;
	auto f = p.get_future();

	sendTaskAsync([&] { p.set_value(); task(); });

	f.get();
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
	mutexThread_.lock();
	thread_ = std::thread([this] { this->threadTask(); });
	mutexThread_.unlock();
}

void ThreadedClass::stopThread() {
	running_ = false;
	cvTasks_.notify_all();
	if (thread_.joinable()) {
		thread_.join();
	}
	thread_.~thread();
}



//MultiThreadedClass
MultiThreadedClass::~MultiThreadedClass() {
	stopThread();
}

void MultiThreadedClass::startThread() {
	mutexThreads_.lock();
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