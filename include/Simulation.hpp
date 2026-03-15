#pragma once
#include <string>
#include "Logger.hpp"

class Simulation {
private:
    double currentTime = 0;
    double timeStep = 1.0;
    double nextTravelerCheck = 0;

    Logger logger;

public:
    Simulation(double secondsPerTick = 1.0) : timeStep(secondsPerTick / 3600.0), logger(5.0) {}

    void run(double endTime);
    void tick();

    static void addTraveler(const std::string& from, const std::string& to);

    double getCurrentTime() const { return currentTime; }
};