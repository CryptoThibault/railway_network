#include "network_simulation.hpp"
#include "pathfinder.hpp"
#include <algorithm>
#include <cmath>

NetworkSimulation::NetworkSimulation(Registry<Train>& trains, bool repeat)
    : _trains(trains), _repeat(repeat)
{
    _services.reserve(trains.size());
    bool first = true;
    for (auto& train : trains)
    {
        _services.emplace_back(train, Pathfinder::findPath(train));
        _nextDepartures.push_back(train.getDeparture());
        if (first || train.getDeparture() < _start)
            _start = train.getDeparture();
        first = false;
    }
}

void NetworkSimulation::update()
{
    _events.clear();
    if (isFinished())
        return;
    const double nextTime = _start + static_cast<double>(_ticks + 1) * Simulation::stepSeconds;
    std::size_t index = 0;
    for (auto& train : _trains)
    {
        auto& service = _services[index];
        if (getTime() + 1e-8 >= _nextDepartures[index])
        {
            if (_repeat && service.isFinished())
                service.restart();
            for (auto event : service.update())
            {
                event.time = nextTime;
                _events.push_back(event);
            }
            if (_repeat && service.isFinished())
            {
                const double arrival = nextTime;
                const double nextDay = std::floor((arrival - train.getDeparture()) / 86400.0) + 1.0;
                _nextDepartures[index] = train.getDeparture() + nextDay * 86400.0;
            }
        }
        ++index;
    }
    ++_ticks;
}

bool NetworkSimulation::isFinished() const
{
    if (_repeat)
        return _services.empty();
    return std::all_of(_services.begin(), _services.end(),
        [](const Simulation& service) { return service.isFinished(); });
}

double NetworkSimulation::getTime() const
{
    return _start + static_cast<double>(_ticks) * Simulation::stepSeconds;
}

const std::vector<TrainEvent>& NetworkSimulation::getEvents() const
{
    return _events;
}
