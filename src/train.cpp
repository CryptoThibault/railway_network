#include "train.hpp"

Train::Train(long id, TrainType type, Station* initialStation)
    : _id(id),
      _board{initialStation},
      _motion(type)
{}

void Train::update()
{
    _stateMachine.update();
}

void Train::transitionTo(TrainState state)
{
    return _stateMachine.transitionTo(state);
}

long Train::getId() const { return _id; }
Board& Train::getBoard() { return _board; }
const Board& Train::getBoard() const { return _board; }
const Motion& Train::getMotion() const { return _motion; }

void Train::state_machine_init()
{
    auto& sm = _stateMachine;

    sm.addState(TrainState::Idle);
    sm.addState(TrainState::Waiting);
    sm.addState(TrainState::Accelerating);
    sm.addState(TrainState::Cruising);
    sm.addState(TrainState::Braking);

    sm.addAction(TrainState::Idle, []{});

    sm.addAction(TrainState::Waiting, []{});

    sm.addAction(TrainState::Accelerating, [this]
    {
        _motion.accelerate(_board);
        _motion.move();
    });

    sm.addAction(TrainState::Cruising, [this]
    {
        _motion.move();
    });

    sm.addAction(TrainState::Braking, [this]
    {
        _motion.brake();
        _motion.move();
    });

    sm.addTransition(TrainState::Idle, TrainState::Waiting, []{});

    sm.addTransition(TrainState::Waiting, TrainState::Accelerating, []{});
    sm.addTransition(TrainState::Waiting, TrainState::Idle, []{});

    sm.addTransition(TrainState::Accelerating, TrainState::Cruising, []{});
    sm.addTransition(TrainState::Accelerating, TrainState::Braking, []{});

    sm.addTransition(TrainState::Cruising, TrainState::Accelerating, []{});
    sm.addTransition(TrainState::Cruising, TrainState::Braking, []{});

    sm.addTransition(TrainState::Braking, TrainState::Accelerating, []{});
    sm.addTransition(TrainState::Braking, TrainState::Cruising, []{});
    sm.addTransition(TrainState::Braking, TrainState::Waiting, []{});
}