#include "motion.hpp"
#include "train.hpp"

Motion::Motion(Train* train) : _train(train) {}

void Motion::move()
{
    _distance += _speed;
}

void Motion::accelerate()
{
    double force = _train->getType()->engineForce - _train->getType()->friction * _speed;
    _speed += force / _train->getType()->mass;

    if (_speed > _train->getType()->maxSpeed)
        _speed = _train->getType()->maxSpeed;
}

void Motion::brake()
{
    double force = -_train->getType()->brakeForce - _train->getType()->friction * _speed;
    _speed += force / _train->getType()->mass;

    if (_speed < 0.0)
        _speed = 0.0;
}

Train* Motion::getTrain() const { return _train; }
double Motion::getSpeed() const { return _speed; }
double Motion::getDistance() const { return _distance; }