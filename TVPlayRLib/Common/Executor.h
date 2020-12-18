#pragma once

#include "Exceptions.h"
#include "blockingconcurrentqueue.h"
#include <thread>
#include <atomic>
#include <functional>
#include <future>

namespace TVPlayR {
    namespace Common {

    class Executor final
    {
        Executor(const Executor&);

        using task_t = std::function<void()>;
        using queue_t = moodycamel::BlockingConcurrentQueue<task_t>;

        std::atomic<bool> is_running_{ true };
        queue_t           queue_;
        std::thread       thread_;

    public:
        Executor()
            : thread_(std::thread([this] { run(); }))
        {
        }

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

            queue_.try_enqueue([=]() mutable { (*task)(); });

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
            queue_.try_enqueue(nullptr);
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
            task_t task;

            while (is_running_) {
                try {
                    queue_.wait_dequeue(task);
                    do {
                        if (!task) {
                            return;
                        }
                        task();
                    } while (queue_.try_dequeue(task));
                }
                catch (...) {
                    //LOG_CURRENT_EXCEPTION();
                }
            }
        }
    };

}}
