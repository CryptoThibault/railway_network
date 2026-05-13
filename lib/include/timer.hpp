#pragma once
#include <chrono>

class Timer
{
public:
    using Clock = std::chrono::system_clock;
    using Duration = std::chrono::milliseconds;

    void start(Duration duration);
    void reset();
    bool isTimeout() const;
    Duration remaining() const;
private:
    Clock::time_point _start{};
    Duration _duration{};
};