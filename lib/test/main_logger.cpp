#include "logger.hpp"
#include <thread>
#include <vector>
#include <chrono>

void worker(int id)
{
    for (int i = 0; i < 3; ++i)
    {
        Logger::instance()->info() << "Worker " << id << " running";
        std::this_thread::sleep_for(std::chrono::milliseconds(10)); // Simulate work
    }
}

int main()
{
    Logger::instantiate();
    auto logger = Logger::instance();

    // Logger configuration
    logger->setLevel(Logger::Level::Debug);
    logger->setShowLevel(true);
    logger->setShowTime(true);

    // ===== Console test =====
    logger->info() << "=== Console Test Start ===";

    logger->trace() << "Trace message";
    logger->debug() << "Debug message";
    logger->info() << "Info message";
    logger->warning() << "Warning message";
    logger->error() << "Error message";
    logger->critical() << "Critical message";

    // Multi-thread test
    std::vector<std::thread> threads;
    for (int i = 0; i < 3; ++i)
        threads.emplace_back(worker, i);

    for (auto& t : threads)
        t.join();

    logger->info() << "=== Console Test End ===";

    // ===== File test =====
    static std::ofstream file("log.txt");
    threadSafeCout.setOutput(file);

    logger->info() << "=== File Test Start ===";

    logger->trace() << "Trace message";
    logger->debug() << "Debug message";
    logger->info() << "Info message";
    logger->warning() << "Warning message";
    logger->error() << "Error message";
    logger->critical() << "Critical message";

    threads.clear();
    for (int i = 0; i < 3; ++i)
        threads.emplace_back(worker, i);

    for (auto& t : threads)
        t.join();

    logger->info() << "=== File Test End ===";
    threadSafeCout.resetOutput();
    logger->info() << "Back to console";

    return 0;
}