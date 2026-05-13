#include "timer.hpp"

void Timer::start(Duration duration)
{
    _start = Clock::now();
    _duration = duration;
}

void Timer::reset()
{
    _start = Clock::now();
}

bool Timer::isTimeout() const
{
    return Clock::now() - _start >= _duration;
}

Timer::Duration Timer::remaining() const
{
    auto elapsed = Clock::now() - _start;
    return isTimeout() ? Duration(0) : std::chrono::duration_cast<Duration>(_duration - elapsed);
}