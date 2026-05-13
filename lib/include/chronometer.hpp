#pragma once
#include <chrono>

class Chronometer
{
public:
    using Clock = std::chrono::system_clock;
    using Duration = std::chrono::milliseconds;

    void start();
    void stop();
    void reset();
    bool isRunning() const;
    Duration elapsed() const;

private:
    bool _running{};
    Clock::time_point _start{};
    Duration _counter{};
};