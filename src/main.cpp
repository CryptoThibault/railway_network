#include "network.hpp"
#include <algorithm>
#include <cmath>
#include <filesystem>
#include <iostream>
#include <stdexcept>

static void require(bool condition, const char* message)
{
    if (!condition)
        throw std::runtime_error(message);
}

static void checkStopped(Train& train)
{
    const double distance = train.getMotion().getDistance();
    train.update();
    require(train.getMotion().getSpeed() == 0.0, "Stopped train has nonzero speed");
    require(train.getMotion().getDistance() == distance, "Stopped train moved");
}

static void runPhase(Train& train, TrainState state, int steps, const char* name)
{
    train.transitionTo(state);
    const double limit = std::min(train.getMotion().getType().maxSpeed,
        train.getBoard().currentSegment->getMaxSpeed());
    require(std::isfinite(limit) && limit > 0.0, "Invalid speed limit");

    for (int step = 0; step < steps; ++step)
    {
        const double previousSpeed = train.getMotion().getSpeed();
        const double previousDistance = train.getMotion().getDistance();
        train.update();
        const double speed = train.getMotion().getSpeed();
        const double distance = train.getMotion().getDistance();
        require(std::isfinite(speed) && speed >= 0.0 && speed <= limit,
            "Speed is outside its valid range");
        require(std::isfinite(distance) && distance >= previousDistance,
            "Distance is invalid or decreased");
        if (state == TrainState::Accelerating)
            require(speed >= previousSpeed, "Speed decreased while accelerating");
        else if (state == TrainState::Cruising)
            require(speed == previousSpeed, "Cruising speed changed");
        else if (state == TrainState::Braking)
            require(speed <= previousSpeed, "Speed increased while braking");
    }

    Logger::instance()->info() << name << " | speed: " << train.getMotion().getSpeed()
        << " | distance: " << train.getMotion().getDistance();
}

int main()
{
    Initializer::initialize();
    Printer::printStations();
    Printer::printSegments();
    Printer::printTrains();

    auto* train = Registry<Train>::instance()->get(0);
    require(train != nullptr, "The demo requires at least one train");
    auto* station = train->getBoard().currentStation;
    require(station != nullptr && !station->getSegments().empty(),
        "The first train requires a station with a connected segment");
    train->getBoard().currentSegment = station->getSegments().front();
    require(train->getBoard().currentSegment != nullptr, "Missing segment");

    checkStopped(*train);
    bool rejected = false;
    try
    {
        train->transitionTo(TrainState::Cruising);
    }
    catch (const std::invalid_argument&)
    {
        rejected = true;
    }
    require(rejected, "Idle to Cruising should be rejected");

    train->transitionTo(TrainState::Waiting);
    checkStopped(*train);
    runPhase(*train, TrainState::Accelerating, 600, "Acceleration");
    require(train->getMotion().getSpeed() > 0.0, "Train did not accelerate");
    const double cruisingStart = train->getMotion().getDistance();
    const double cruisingSpeed = train->getMotion().getSpeed();
    runPhase(*train, TrainState::Cruising, 20, "Cruising");
    require(std::abs(train->getMotion().getDistance() - cruisingStart
        - cruisingSpeed * 20.0) < 1e-6, "Unexpected cruising distance");
    runPhase(*train, TrainState::Braking, 600, "Braking");
    checkStopped(*train);
    train->transitionTo(TrainState::Waiting);
    checkStopped(*train);
    train->transitionTo(TrainState::Idle);
    checkStopped(*train);

    Logger::instance()->info() << "All demo checks passed";
    return 0;
}
