#include "motion.hpp"
#include "board.hpp"
#include "segment.hpp"

Motion::Motion(TrainType type) : _type(type) {}

void Motion::move(double seconds)
{
    _distance += _speed * seconds;
}

void Motion::accelerate(const Board& board, double seconds)
{
    double force = _type.engineForce - _type.friction * _speed;
    _speed += force / _type.mass * seconds;

    double maxSpeed = std::min(_type.maxSpeed, board.currentSegment->getMaxSpeed());
    if (_speed > maxSpeed) _speed = maxSpeed;
}

void Motion::brake(double seconds)
{
    double force = -_type.brakeForce - _type.friction * _speed;
    _speed += force / _type.mass * seconds;

    if (_speed < 0.0) _speed = 0.0;
}

const TrainType& Motion::getType() const { return _type; }
double Motion::getSpeed() const { return _speed; }
double Motion::getDistance() const { return _distance; }