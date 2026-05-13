#include "worker_pool.hpp"
#include <iostream>
#include "thread_safe_iostream.hpp"

int main() {
    WorkerPool pool(4);

    auto job = []() {
        threadSafeCout << "Executing job on thread: " << std::this_thread::get_id() << std::endl;
        std::this_thread::sleep_for(std::chrono::milliseconds(10)); // Simulate work
    };

    for (int i = 0; i < 1000; ++i) {
        pool.addJob(job);
    }

    std::this_thread::sleep_for(std::chrono::seconds(2)); // Wait for jobs to finish

    return 0;
}
