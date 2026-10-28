#pragma once

#include "train.hpp"
#include "train_event.hpp"
#include <vector>

class Simulation
{
public:
    Simulation(Train& train, std::vector<Segment*> path);
    std::vector<TrainEvent> update();
    void restart();
    bool isFinished() const;
    double getElapsedSeconds() const;
    unsigned long getCompletedLegs() const;
    static constexpr double stepSeconds = 0.1;

private:
    Train& _train;
    std::vector<Station*> _stations;
    Station* _departure;
    Station* _destination;
    double _elapsed = 0.0;
    double _waiting = 0.0;
    unsigned long _completedLegs = 0;
};
