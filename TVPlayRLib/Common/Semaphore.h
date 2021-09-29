#pragma once
#include "../Common/NonCopyable.h"

namespace TVPlayR {
	namespace Common {

class Semaphore final : Common::NonCopyable
{
private:
	std::mutex mutex_;
	std::condition_variable cv_;
	int count_ = 0; // Initialized as locked.

public:
	Semaphore(int count = 0)
		:count_ (count)
	{}

	void notify() 
	{
		std::lock_guard<std::mutex> lock(mutex_);
		++count_;
		cv_.notify_one();
	}

	void wait() 
	{
		std::unique_lock<std::mutex> lock(mutex_);
		while (!count_) 
			cv_.wait(lock);
		--count_;
	}

	bool try_wait() 
	{
		std::lock_guard<std::mutex> lock(mutex_);
		if (count_) 
		{
			--count_;
			return true;
		}
		return false;
	}
};

}}