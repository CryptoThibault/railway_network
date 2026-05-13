#include "chronometer.hpp"

void Chronometer::start()
{
    if (_running) return;
    _running = true;
    _start = Clock::now();
}

void Chronometer::stop()
{
    if (!_running) return;
    _running = false;
    _counter += std::chrono::duration_cast<Duration>(Clock::now() - _start);
}

void Chronometer::reset()
{
    _running = false;
    _counter = Duration(0);
}

bool Chronometer::isRunning() const
{
    return _running;
}

Chronometer::Duration Chronometer::elapsed() const
{
    return _running ? _counter + std::chrono::duration_cast<Duration>(Clock::now() - _start) : _counter;
}

