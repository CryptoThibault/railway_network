#include "simulation.hpp"
#include "station.hpp"
#include "segment.hpp"
#include <algorithm>
#include <cmath>
#include <stdexcept>
#include <utility>

Simulation::Simulation(Train& train, std::vector<Segment*> path)
    : _train(train), _departure(train.getBoard().currentStation), _destination(nullptr)
{
    if (path.empty() || !_departure)
        throw std::runtime_error("The route must start at the train's station");
    _stations.push_back(_departure);
    for (const auto* segment : path)
    {
        Station* from = _stations.back();
        if (!segment || !segment->getStationA() || !segment->getStationB()
            || segment->getStationA() == segment->getStationB()
            || (segment->getStationA() != from && segment->getStationB() != from)
            || !std::isfinite(segment->getLength()) || segment->getLength() <= 0.0
            || !std::isfinite(segment->getMaxSpeed()) || segment->getMaxSpeed() <= 0.0)
            throw std::runtime_error("Missing or invalid route segment");
        _stations.push_back(segment->getStationA() == from
            ? segment->getStationB() : segment->getStationA());
    }
    if (_stations.back() != &train.getEnd())
        throw std::runtime_error("The route must end at the train's destination");
    train.getMotion().getParameters().validate();
    _destination = _stations[1];
    train.getBoard().path = std::move(path);
    train.getBoard().pathIndex = 0;
    train.getBoard().currentSegment = train.getBoard().path.front();
    train.getBoard().segmentProgress = 0.0;
}

void Simulation::restart()
{
    if (!isFinished())
        throw std::logic_error("Cannot restart an unfinished journey");
    _train.reset();
    _departure = _stations.front();
    _destination = _stations[1];
    _elapsed = 0.0;
    _waiting = 0.0;
    _completedLegs = 0;
}

std::vector<TrainEvent> Simulation::update()
{
    std::vector<TrainEvent> events;
    if (isFinished())
        return events;
    _elapsed += stepSeconds;
    auto& board = _train.getBoard();
    const auto& motion = _train.getMotion();
    const auto& type = motion.getParameters();
    if (_train.getState() == TrainState::Scheduled || _train.getState() == TrainState::Waiting)
    {
        _waiting = std::max(0.0, _waiting - stepSeconds);
        if (_waiting > 1e-8)
            return events;
        board.pathIndex = _completedLegs;
        board.currentSegment = board.path[board.pathIndex];
        _departure = _stations[board.pathIndex];
        _destination = _stations[board.pathIndex + 1];
        board.segmentProgress = 0.0;
        board.currentStation = nullptr;
        _train.transitionTo(TrainState::Accelerating);
        events.push_back({TrainEventType::Departure, &_train, _departure, _elapsed, _completedLegs == 0});
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
            return events;
        }
        board.segmentProgress = board.currentSegment->getLength();
        board.currentStation = _destination;
        ++_completedLegs;
        _waiting = _train.getPause();
        const bool finished = _completedLegs == board.path.size();
        _train.transitionTo(finished ? TrainState::Completed : TrainState::Waiting);
        events.push_back({finished ? TrainEventType::Arrival : TrainEventType::StationStop,
            &_train, _destination, _elapsed});
    }
    return events;
}

double Simulation::getElapsedSeconds() const { return _elapsed; }
unsigned long Simulation::getCompletedLegs() const { return _completedLegs; }

bool Simulation::isFinished() const { return _train.getState() == TrainState::Completed; }
