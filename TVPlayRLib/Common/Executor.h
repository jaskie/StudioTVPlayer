#pragma once

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
        BlockingCollection<std::function<void()>>  queue_;
        std::atomic_bool  is_running_ = true;
        std::thread       thread_;

    public:
        Executor(const std::string& name)
            : thread_(&Executor::run, this, name)
        { }

        Executor(const std::string& name, size_t max_queue_size)
            : thread_(&Executor::run, this, name)
            , queue_(max_queue_size)
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
            queue_.try_add([=]() mutable { (*task)(); });
            return task->get_future();
        }

        template <typename Func>
        auto invoke(Func&& func)
        {
            if (is_current())  // Avoids potential deadlock.
                return func();
            if (!is_running_) {
                THROW_EXCEPTION("executor not running.");
            }
            using result_type = decltype(func());
            auto task = std::make_shared<std::packaged_task<result_type()>>(std::forward<Func>(func));
            queue_.add([=]() mutable { (*task)(); });
            return task->get_future().get();
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
            if (!is_running_) 
                return;
            is_running_ = false;
            queue_.complete_adding();
        }

        void wait()
        {
            invoke([] {});
        }

        bool is_running() const { return is_running_; }

        bool is_current() const { return std::this_thread::get_id() == thread_.get_id(); }

    private:
        void run(std::string name)
        {
            SetThreadName(::GetCurrentThreadId(), name.c_str());
            std::function<void()> task;
            while (is_running_) {
                try {
                    std::function<void()> task;
                    auto status = queue_.take(task);
                    if (status == BlockingCollectionStatus::AddingCompleted)
                        return;
                    task();
                }
                catch (...) {
                    //LOG_CURRENT_EXCEPTION();
                }
            }
        }
    };

}}
