#include "simulation.hpp"
#include <algorithm>
#include <cmath>
#include <iomanip>
#include <ostream>
#include <stdexcept>

static const char* stateName(TrainState state)
{
    switch (state)
    {
        case TrainState::Idle: return "Idle";
        case TrainState::Waiting: return "Waiting";
        case TrainState::Accelerating: return "Accelerating";
        case TrainState::Cruising: return "Cruising";
        case TrainState::Braking: return "Braking";
    }
    return "Unknown";
}

Simulation::Simulation(Train& train) : _train(train),
    _departure(train.getBoard().currentStation), _destination(nullptr)
{
    if (!_departure || _departure->getSegments().empty())
        throw std::runtime_error("The train needs a station with a connected segment");
    auto* segment = _departure->getSegments().front();
    if (!segment || !segment->getStationA() || !segment->getStationB()
        || (segment->getStationA() != _departure && segment->getStationB() != _departure)
        || !std::isfinite(segment->getLength()) || segment->getLength() <= 0.0
        || !std::isfinite(segment->getMaxSpeed()) || segment->getMaxSpeed() <= 0.0)
        throw std::runtime_error("Invalid simulation segment");
    const auto& type = train.getMotion().getType();
    if (!std::isfinite(type.mass) || type.mass <= 0.0
        || !std::isfinite(type.engineForce) || type.engineForce <= 0.0
        || !std::isfinite(type.brakeForce) || type.brakeForce <= 0.0
        || !std::isfinite(type.friction) || type.friction < 0.0
        || !std::isfinite(type.maxSpeed) || type.maxSpeed <= 0.0)
        throw std::runtime_error("Invalid train motion parameters");
    _destination = segment->getStationA() == _departure
        ? segment->getStationB() : segment->getStationA();
    train.getBoard().currentSegment = segment;
    train.getBoard().segmentProgress = 0.0;
    train.transitionTo(TrainState::Waiting);
}

void Simulation::update()
{
    if (isFinished())
        return;
    _elapsed += stepSeconds;
    auto& board = _train.getBoard();
    const auto& motion = _train.getMotion();
    const auto& type = motion.getType();
    if (_train.getState() == TrainState::Waiting)
    {
        _waiting -= stepSeconds;
        if (_waiting > 1e-8)
            return;
        board.segmentProgress = 0.0;
        board.currentStation = nullptr;
        _train.transitionTo(TrainState::Accelerating);
    }

    const double remaining = board.currentSegment->getLength() - board.segmentProgress;
    const double speed = motion.getSpeed();
    const double deceleration = (type.brakeForce + type.friction * speed) / type.mass;
    const double stoppingDistance = speed * speed / (2.0 * deceleration);
    const double limit = std::min(type.maxSpeed, board.currentSegment->getMaxSpeed());
    if (_train.getState() != TrainState::Braking
        && remaining <= stoppingDistance + speed * stepSeconds)
        _train.transitionTo(TrainState::Braking);
    else if (_train.getState() == TrainState::Accelerating && speed >= limit)
        _train.transitionTo(TrainState::Cruising);

    const double previousDistance = motion.getDistance();
    _train.update(stepSeconds);
    board.segmentProgress = std::min(board.currentSegment->getLength(),
        board.segmentProgress + motion.getDistance() - previousDistance);
    if (_train.getState() == TrainState::Braking && motion.getSpeed() == 0.0)
    {
        if (board.currentSegment->getLength() - board.segmentProgress > 2.0 * limit * stepSeconds + 0.01)
        {
            _train.transitionTo(TrainState::Accelerating);
            return;
        }
        board.segmentProgress = board.currentSegment->getLength();
        board.currentStation = _destination;
        ++_completedLegs;
        _train.transitionTo(TrainState::Waiting);
        _train.transitionTo(TrainState::Idle);
    }
}

void Simulation::print(std::ostream& output, int timeScale) const
{
    const auto& board = _train.getBoard();
    const auto seconds = static_cast<unsigned long>(_elapsed + 1e-6);
    output << std::fixed << std::setprecision(1)
        << "Time " << seconds / 3600 << 'h' << (seconds / 60) % 60 << 'm' << seconds % 60
        << "s | " << timeScale << "x | Train " << _train.getId() << " (" << _train.getMotion().getType().name << ")"
        << " | State: " << stateName(_train.getState())
        << " | Speed: " << _train.getMotion().getSpeed() * 3.6 << " km/h"
        << " | Segment: " << _departure->getName() << " -> " << _destination->getName()
        << " | Position: " << board.segmentProgress / 1000.0 << " / "
        << board.currentSegment->getLength() / 1000.0 << " km"
        << " | Legs: " << _completedLegs << std::endl;
}

double Simulation::getElapsedSeconds() const { return _elapsed; }
unsigned long Simulation::getCompletedLegs() const { return _completedLegs; }

bool Simulation::isFinished() const { return _completedLegs == 1; }
