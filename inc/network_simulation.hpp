#pragma once
#include "simulation.hpp"
#include <algorithm>
#include "registry.hpp"
#include <vector>

class NetworkSimulation
{
public:
    explicit NetworkSimulation(Registry<Train>& trains, bool repeat = true);

    void update();
    bool isFinished() const;
    const std::vector<TrainEvent>& getEvents() const;
    double getTime() const;

private:
    Registry<Train>& _trains;
    std::vector<Simulation> _services;
    std::vector<double> _nextDepartures;
    std::vector<TrainEvent> _events;
    bool _repeat;
    double _start = 0.0;
    unsigned long _ticks = 0;
};
