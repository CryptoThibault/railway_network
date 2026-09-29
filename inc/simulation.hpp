#pragma once

#include "train.hpp"
#include <iosfwd>
#include <vector>

class Simulation
{
public:
    Simulation(Train& train, std::vector<Station*> stations);
    void update();
    bool isFinished() const;
    void print(std::ostream& output, int timeScale = 60) const;
    double getElapsedSeconds() const;
    unsigned long getCompletedLegs() const;
    static constexpr double stepSeconds = 0.1;

private:
    static constexpr double passengerStopSeconds = 600.0;

    Train& _train;
    std::vector<Station*> _stations;
    bool _finished = false;
    Station* _departure;
    Station* _destination;
    double _elapsed = 0.0;
    double _waiting = passengerStopSeconds;
    unsigned long _completedLegs = 0;
};
