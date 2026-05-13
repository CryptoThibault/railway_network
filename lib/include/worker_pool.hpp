#pragma once
#include "pool.hpp"
#include "thread.hpp"
#include "thread_safe_queue.hpp"
#include <functional>
#include <vector>
#include <memory>
#include <mutex>
#include <condition_variable>

class WorkerPool
{
public:
    class IJobs
    {
    public:
        virtual ~IJobs() = default;
        virtual void execute() = 0;
    };

    WorkerPool(size_t numThreads);
    ~WorkerPool();

    void addJob(const std::function<void()>& jobToExecute);

private:
    class FunctionJob : public IJobs
    {
    public:
        FunctionJob() = default;
        FunctionJob(std::function<void()> f) : _func(std::move(f)) {}
        void execute() override { _func(); }

    private:
        std::function<void()> _func;
    };

    Pool<FunctionJob> _jobPool;
    std::vector<Thread> _threads;
    ThreadSafeQueue<std::shared_ptr<IJobs>> _jobs;
    std::mutex _mutex;
    std::condition_variable _cv;
    bool _stop{};

    void workerLoop();
};

