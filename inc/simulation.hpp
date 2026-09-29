#pragma once

#include "train.hpp"
#include <iosfwd>

class Simulation
{
public:
    explicit Simulation(Train& train);
    void update();
    bool isFinished() const;
    void print(std::ostream& output, int timeScale = 60) const;
    double getElapsedSeconds() const;
    unsigned long getCompletedLegs() const;
    static constexpr double stepSeconds = 0.1;

private:
    Train& _train;
    Station* _departure;
    Station* _destination;
    double _elapsed = 0.0;
    double _waiting = 30.0;
    unsigned long _completedLegs = 0;
};
