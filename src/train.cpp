#include "train.hpp"
#include "station.hpp"
#include <utility>
#include <cmath>
#include <stdexcept>

Train::Train(long id, std::string name, TrainParameters parameters, Station* start, Station* end, double departure, double pause)
    : _id(id),
      _name(std::move(name)),
      _start(start),
      _end(end),
      _departure(departure),
      _pause(pause),
      _board{start},
      _motion(parameters)
{
    if (!_start || !_end || _start == _end || _id <= 0 || _name.empty())
        throw std::invalid_argument("Invalid train identity or stations");
    if (!std::isfinite(departure) || departure < 0.0 || departure >= 86400.0
        || std::floor(departure) != departure || !std::isfinite(pause) || pause < 0.0)
        throw std::invalid_argument("Invalid train schedule");
}

void Train::update(double seconds)
{
    if (!std::isfinite(seconds) || seconds <= 0.0)
        throw std::invalid_argument("Simulation step must be positive and finite");
    _stepSeconds = seconds;
    _stateMachine.update();
}

void Train::reset()
{
    if (_state != TrainState::Completed)
        throw std::logic_error("Cannot reset an unfinished train");
    transitionTo(TrainState::Scheduled);
    _motion = Motion(_motion.getParameters());
    _board.currentStation = _start;
    _board.currentSegment = _board.path.empty() ? nullptr : _board.path.front();
    _board.segmentProgress = 0.0;
    _board.pathIndex = 0;
    _stepSeconds = 1.0;
}

void Train::transitionTo(TrainState state)
{
    _stateMachine.transitionTo(state);
    _state = state;
}

TrainState Train::getState() const { return _state; }

long Train::getId() const { return _id; }
const std::string& Train::getName() const { return _name; }
const Station& Train::getStart() const { return *_start; }
const Station& Train::getEnd() const { return *_end; }
double Train::getDeparture() const { return _departure; }
double Train::getPause() const { return _pause; }
Board& Train::getBoard() { return _board; }
const Board& Train::getBoard() const { return _board; }
const Motion& Train::getMotion() const { return _motion; }

void Train::state_machine_init()
{
    auto& sm = _stateMachine;

    sm.addState(TrainState::Scheduled);
    sm.addState(TrainState::Waiting);
    sm.addState(TrainState::Accelerating);
    sm.addState(TrainState::Cruising);
    sm.addState(TrainState::Braking);
    sm.addState(TrainState::Completed);

    sm.addAction(TrainState::Scheduled, []{});

    sm.addAction(TrainState::Waiting, []{});

    sm.addAction(TrainState::Accelerating, [this]
    {
        _motion.accelerate(_board, _stepSeconds);
        _motion.move(_stepSeconds);
    });

    sm.addAction(TrainState::Cruising, [this]
    {
        _motion.move(_stepSeconds);
    });

    sm.addAction(TrainState::Braking, [this]
    {
        _motion.brake(_stepSeconds);
        _motion.move(_stepSeconds);
    });

    sm.addTransition(TrainState::Scheduled, TrainState::Accelerating, []{});
    sm.addTransition(TrainState::Completed, TrainState::Scheduled, []{});

    sm.addTransition(TrainState::Waiting, TrainState::Accelerating, []{});

    sm.addTransition(TrainState::Accelerating, TrainState::Cruising, []{});
    sm.addTransition(TrainState::Accelerating, TrainState::Braking, []{});

    sm.addTransition(TrainState::Cruising, TrainState::Accelerating, []{});
    sm.addTransition(TrainState::Cruising, TrainState::Braking, []{});

    sm.addTransition(TrainState::Braking, TrainState::Accelerating, []{});
    sm.addTransition(TrainState::Braking, TrainState::Cruising, []{});
    sm.addTransition(TrainState::Braking, TrainState::Waiting, []{});
    sm.addTransition(TrainState::Braking, TrainState::Completed, []{});
}