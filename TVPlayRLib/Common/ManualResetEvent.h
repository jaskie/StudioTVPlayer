#pragma once

namespace TVPlayR {
	namespace Common {
		class ManualResetEvent {
		private:
			std::condition_variable cv_;
			std::mutex mutex_;
			bool state_;

		public:
			ManualResetEvent(bool initialState)
			: state_(initialState)
			{ }

			void Set()
			{
				std::lock_guard<std::mutex> lock(mutex_);
				state_ = true;
				cv_.notify_all();
			}

			void Reset()
			{
				std::lock_guard<std::mutex> lock(mutex_);
				state_ = false;
			}

			void Wait()
			{
				std::unique_lock<std::mutex> lock(mutex_);
				while (!state_)
				{
					cv_.wait(lock, [&]() { return state_; });
				}
			}
		};
	}
}