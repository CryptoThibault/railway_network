#include "worker_pool.hpp"

WorkerPool::WorkerPool(size_t numThreads)
{
    constexpr size_t DefaultPoolSize = 256;
    _jobPool.resize(DefaultPoolSize);

    for (size_t i = 0; i < numThreads; ++i)
        _threads.emplace_back("Worker " + std::to_string(i),
            [this]() { workerLoop(); });
    
    for (auto& t : _threads)
        t.start();
}

WorkerPool::~WorkerPool()
{
    {   
        std::lock_guard<std::mutex> lock(_mutex);
        _stop = true;
    }
    _cv.notify_all(); 
    
    for (auto& t : _threads)
        t.stop();
}

void WorkerPool::addJob(const std::function<void()>& jobToExecute)
{
    auto obj = _jobPool.acquire(jobToExecute);
    auto* raw = obj.operator->();

    auto holder = std::make_shared<Pool<FunctionJob>::Object>(std::move(obj));
    std::shared_ptr<IJobs> jobPtr(raw,[holder](IJobs*) mutable {});

    _jobs.push_back(jobPtr);
    _cv.notify_one();
}

void WorkerPool::workerLoop()
{
    while (true)
    {
        std::shared_ptr<IJobs> job;
        
        {
            std::unique_lock<std::mutex> lock(_mutex);
            _cv.wait(lock, [this]() { return !_jobs.empty() || _stop; });

            if (_stop && _jobs.empty())
                return;

            job = _jobs.pop_front();
        }
        job->execute();
    }
}