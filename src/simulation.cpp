#include "simulation.hpp"
#include <algorithm>
#include <cmath>
#include <iomanip>
#include <ostream>
#include <stdexcept>
#include <utility>

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

Simulation::Simulation(Train& train, std::vector<Station*> stations)
    : _train(train), _stations(std::move(stations)),
      _departure(train.getBoard().currentStation), _destination(nullptr)
{
    if (_stations.size() < 2 || !_departure || _stations.front() != _departure)
        throw std::runtime_error("The route must start at the train's station");
    std::vector<Segment*> path;
    for (std::size_t index = 1; index < _stations.size(); ++index)
    {
        auto* from = _stations[index - 1];
        auto* to = _stations[index];
        if (!from || !to || from == to)
            throw std::runtime_error("Invalid route station");
        Segment* connection = nullptr;
        for (auto* segment : from->getSegments())
            if (segment && ((segment->getStationA() == from && segment->getStationB() == to)
                || (segment->getStationB() == from && segment->getStationA() == to)))
            {
                connection = segment;
                break;
            }
        if (!connection || !std::isfinite(connection->getLength()) || connection->getLength() <= 0.0
            || !std::isfinite(connection->getMaxSpeed()) || connection->getMaxSpeed() <= 0.0)
            throw std::runtime_error("Missing or invalid route segment");
        path.push_back(connection);
    }
    const auto& type = train.getMotion().getType();
    if (!std::isfinite(type.mass) || type.mass <= 0.0
        || !std::isfinite(type.engineForce) || type.engineForce <= 0.0
        || !std::isfinite(type.brakeForce) || type.brakeForce <= 0.0
        || !std::isfinite(type.friction) || type.friction < 0.0
        || !std::isfinite(type.maxSpeed) || type.maxSpeed <= 0.0)
        throw std::runtime_error("Invalid train motion parameters");
    _destination = _stations[1];
    train.getBoard().path = std::move(path);
    train.getBoard().pathIndex = 0;
    train.getBoard().currentSegment = train.getBoard().path.front();
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
        _waiting = std::max(0.0, _waiting - stepSeconds);
        if (_waiting > 1e-8)
            return;
        if (_completedLegs == board.path.size())
        {
            _finished = true;
            return;
        }
        board.pathIndex = _completedLegs;
        board.currentSegment = board.path[board.pathIndex];
        _departure = _stations[board.pathIndex];
        _destination = _stations[board.pathIndex + 1];
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
        _waiting = passengerStopSeconds;
        _train.transitionTo(TrainState::Waiting);
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
        << " | Legs: " << _completedLegs << " | Route: ";
    for (std::size_t index = 0; index < _stations.size(); ++index)
    {
        if (index > 0)
            output << " -> ";
        output << _stations[index]->getName();
    }
    output << " | Lengths: ";
    for (std::size_t index = 0; index < board.path.size(); ++index)
    {
        if (index > 0)
            output << ',';
        output << board.path[index]->getLength() / 1000.0;
    }
    output << " | Wait: " << static_cast<unsigned long>(std::ceil(std::max(0.0, _waiting - 1e-8)))
        << std::endl;
}

double Simulation::getElapsedSeconds() const { return _elapsed; }
unsigned long Simulation::getCompletedLegs() const { return _completedLegs; }

bool Simulation::isFinished() const { return _finished; }
