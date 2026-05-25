#include "train.hpp"

Train::Train(long id, const TrainType* type, Station* initialStation)
    : _id(id),
      _type(type),
      _board{initialStation},
      _motion(this)
{
    state_machine_init();

    auto& sm = _stateMachine;
    sm.transitionTo(TrainState::Waiting);
    sm.transitionTo(TrainState::Accelerating);
    for (int i = 0; i < 60; i++) sm.update();
    Logger::instance()->info() << "speed= " << _motion.getSpeed() << " distance= " << _motion.getDistance();
    sm.transitionTo(TrainState::Cruising);
    for (int i = 0; i < 60; i++) sm.update();
    Logger::instance()->info() << "speed= " << _motion.getSpeed() << " distance= " << _motion.getDistance();
    sm.transitionTo(TrainState::Braking);
    for (int i = 0; i < 60; i++) sm.update();
    Logger::instance()->info() << "speed= " << _motion.getSpeed() << " distance= " << _motion.getDistance();
}

long Train::getId() const { return _id; }
const TrainType* Train::getType() const { return _type; }
Board Train::getBoard() const { return _board; }
Motion Train::getMotion() const { return _motion; }

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
        _motion.accelerate();
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