#pragma once

#include "Exceptions.h"
#include "Semaphore.h"

namespace TVPlayR {
    namespace Common {

        typedef struct tagTHREADNAME_INFO
        {
            DWORD dwType; // must be 0x1000
            LPCSTR szName; // pointer to name (in user addr space)
            DWORD dwThreadID; // thread ID (-1=caller thread)
            DWORD dwFlags; // reserved for future use, must be zero
        } THREADNAME_INFO;

        static void SetThreadName(DWORD dwThreadID, LPCSTR szThreadName)
        {
            THREADNAME_INFO info;
            {
                info.dwType = 0x1000;
                info.szName = szThreadName;
                info.dwThreadID = dwThreadID;
                info.dwFlags = 0;
            }
            __try
            {
                RaiseException(0x406D1388, 0, sizeof(info) / sizeof(DWORD), (ULONG_PTR*)&info);
            }
            __except (EXCEPTION_CONTINUE_EXECUTION) {}
        }

    class Executor final
    {
    private:
        Executor(const Executor&);

        using task_t = std::function<void()>;
        using queue_t = std::deque<task_t>;

        const std::string name_;
        std::atomic<bool> is_running_ = true;
        queue_t           queue_;
        std::thread       thread_;
        Common::Semaphore semaphore_;

    public:
        Executor(const std::string& name)
            : thread_(std::thread([this] { run(); }))
            , name_(name)
        { }

        ~Executor()
        {
            stop();
            thread_.join();
        }

        template <typename Func>
        auto begin_invoke(Func&& func)
        {
            if (!is_running_) {
                THROW_EXCEPTION("executor not running.");
            }

            using result_type = decltype(func());

            auto task = std::make_shared<std::packaged_task<result_type()>>(std::forward<Func>(func));

            queue_.push_back([=]() mutable { (*task)(); });
            semaphore_.notify();

            return task->get_future();
        }

        template <typename Func>
        auto invoke(Func&& func)
        {
            if (is_current()) { // Avoids potential deadlock.
                return func();
            }

            return begin_invoke(std::forward<Func>(func)).get();
        }

        template <typename Func>
        typename std::enable_if<std::is_same<void, decltype(std::declval<Func>())>::value, void>::type invoke(Func&& func)
        {
            if (is_current()) { // Avoids potential deadlock.
                func();
                return;
            }

            begin_invoke(std::forward<Func>(func)).wait();
        }

        void stop()
        {
            if (!is_running_) {
                return;
            }
            is_running_ = false;
            queue_.push_back(nullptr);
            semaphore_.notify();
        }

        void wait()
        {
            invoke([] {});
        }

        bool is_running() const { return is_running_; }

        bool is_current() const { return std::this_thread::get_id() == thread_.get_id(); }

    private:
        void run()
        {
            SetThreadName(::GetCurrentThreadId(), name_.c_str());
            task_t task;
            while (is_running_) {
                try {
                    semaphore_.wait();
                    task = queue_.front();
                    queue_.pop_front();
                    if (!task) {
                        return;
                    }
                    task();
                }
                catch (...) {
                    //LOG_CURRENT_EXCEPTION();
                }
            }
        }
    };

}}
