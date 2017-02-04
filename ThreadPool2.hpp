#ifndef THREAD_POOL_H
#define THREAD_POOL_H

#include <vector>
#include <queue>
#include <memory>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <future>
#include <functional>
#include <stdexcept>
#include "TSQ.hpp"

class ThreadPool2 {
    
    using func = std::function<void(void)>;
    
public:
    ThreadPool2(int);
    
    template<class F, class... Args>
    auto enqueue(F&& f, Args&&... args)
    -> std::future<typename std::result_of<F(Args...)>::type>;
    
    ~ThreadPool2();
    
private:
    
    std::vector< std::thread > pool;
    TSQ<func> queue;
    

    std::mutex itemMutex;
    std::condition_variable condition;
    std::atomic<bool> shouldContinue;

};

// the constructor just launches some amount of workers
inline ThreadPool2::ThreadPool2(int threads)
:   shouldContinue(false)
{
    for(int i = 0;i<threads;++i)
        pool.emplace_back(
                             [this]
                             {
                                 for(;;)
                                 {
                                     std::function<void(void)> task;
                                     
                                     {
                                         std::unique_lock<std::mutex> lock(this->itemMutex);
                                         this->condition.wait(lock,
                                                              [this]{ return this->shouldContinue || !this->queue.empty(); });
                                         if(this->shouldContinue && this->queue.empty())
                                             return;
                                         //task = std::move(this->tasks.front());
                                         this->queue.dequeue(task);
                                     }
                                     
                                     task();
                                 }
                             }
                             );
}

// add new work item to the pool
template<class F, class... Args>
auto ThreadPool2::enqueue(F&& f, Args&&... args)
-> std::future<typename std::result_of<F(Args...)>::type>
{
    using return_type = typename std::result_of<F(Args...)>::type;
    
    auto task = std::make_shared< std::packaged_task<return_type()> >(
                                                                      std::bind(std::forward<F>(f), std::forward<Args>(args)...)
                                                                      );
    
    std::future<return_type> res = task->get_future();
    {
        std::unique_lock<std::mutex> lock(itemMutex);
        
        // don't allow enqueueing after stopping the pool
        if(shouldContinue)
            throw std::runtime_error("enqueue on stopped ThreadPool");
        
        queue.enqueue([task](){ (*task)(); });
    }
    condition.notify_one();
    return res;
}

// the destructor joins all threads
inline ThreadPool2::~ThreadPool2()
{
    {
        std::unique_lock<std::mutex> lock(itemMutex);
        shouldContinue = true;
    }
    condition.notify_all();
    for(std::thread &worker: pool)
        worker.join();
}

#endif
                                         
