#pragma once

namespace TVPlayR {
    namespace Common {

#ifdef DEBUG
        static void SetThreadName(DWORD dwThreadID, LPCSTR szThreadName)
        {
            HANDLE hThread = ::GetCurrentThread();
            const size_t size = strlen(szThreadName) + 1;
            wchar_t* wThreadName = new wchar_t[size];
            size_t outSize;
            try
            {
                if (mbstowcs_s(&outSize, wThreadName, size, szThreadName, size - 1) == 0)
                    ::SetThreadDescription(hThread, wThreadName);
            }
            catch (...) {}
            delete[] wThreadName;
        }
#endif

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
            if (!is_running_)
                THROW_EXCEPTION("Executor: not running.");

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
            if (!is_running_)
                THROW_EXCEPTION("Executor: not running.");

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
#ifdef DEBUG
            SetThreadName(::GetCurrentThreadId(), name.c_str());
#endif
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
