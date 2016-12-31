#ifndef TASKPOOL_HPP
#define TASKPOOL_HPP

#include <vector>
#include <queue>
#include <memory>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <future>
#include <functional>
#include <stdexcept>
#include "commons.hpp"


class TaskPool {
    public:
        TaskPool(size_t threads, std::function<void()> = [](){});
        ~TaskPool();

        template<class F, class... Args>
        auto enqueue(F&& f, Args&&... args)
        -> std::future<typename std::result_of<F(Args...)>::type>;

        void discard();
        int size();

    private:
        std::vector< std::thread > workers;
        std::queue< std::function<void()> > tasks;

        std::mutex queue_mutex;
        std::condition_variable condition;
        bool stop;
};

inline TaskPool::TaskPool(size_t threads, std::function<void()> init) : stop(false) {
    for(size_t i = 0;i<threads;++i)
        workers.emplace_back( [this, init] {
            init();
            for(;;) {
                std::unique_lock<std::mutex> lock(this->queue_mutex);
                while(!this->stop && this->tasks.empty())
                    this->condition.wait(lock);
                if(this->stop && this->tasks.empty())
                    return;
                std::function<void()> task(this->tasks.front());
                this->tasks.pop();
                lock.unlock();
                task();
            }
        });
}

inline TaskPool::~TaskPool() {
    {
        std::unique_lock<std::mutex> lock(queue_mutex);
        stop = true;
    }
    condition.notify_all();
    for(size_t i = 0; i < workers.size(); ++i)
        workers[i].join();
}


template<class F, class... Args>
auto TaskPool::enqueue(F&& f, Args&&... args)
-> std::future<typename std::result_of<F(Args...)>::type> {
    typedef typename std::result_of<F(Args...)>::type return_type;

    VBE_ASSERT(!stop, "enqueue on stopped ThreadPool");
    auto task = std::make_shared< std::packaged_task<return_type()> >(
                    std::bind(std::forward<F>(f), std::forward<Args>(args)...)
                    );
    std::future<return_type> res = task->get_future();

    {
        std::unique_lock<std::mutex> lock(queue_mutex);
        tasks.push([task](){ (*task)(); });
    }

    condition.notify_one();
    return res;
}

inline void TaskPool::discard() {
    std::unique_lock<std::mutex> lock(queue_mutex);
    tasks = std::queue< std::function<void()> >();
}

int TaskPool::size() {
    std::unique_lock<std::mutex> lock(queue_mutex);
    return tasks.size();
}

#endif //TASKPOOL_HPP
