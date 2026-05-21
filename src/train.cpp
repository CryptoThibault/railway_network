#include "train.hpp"

Train::Train(long id, const TrainType* type, Station* initialStation)
    : _id(id),
      _type(type),
      _board{initialStation},
      _stateMachine{}
{
    auto sm = _stateMachine;

    sm.addState(TrainState::Idle);
    sm.addState(TrainState::Waiting);
    sm.addState(TrainState::Accelerating);
    sm.addState(TrainState::Cruising);
    sm.addState(TrainState::Braking);

}

long Train::getId() const { return _id; }
const TrainType* Train::getType() const { return _type; }
double Train::getSpeed() const { return _speed; }
double Train::getDistance() const { return _distance; }
Board Train::getBoard() const { return _board; }
MotionSystem Train::getMotionSystem() const { return _motionSystem; }