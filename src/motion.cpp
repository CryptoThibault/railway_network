#include "motion.hpp"
#include "board.hpp"
#include "segment.hpp"
#include <algorithm>
#include <cmath>
#include <stdexcept>

Motion::Motion(TrainParameters parameters) : _parameters(parameters)
{
    _parameters.validate();
}

void Motion::move(double seconds)
{
    _distance += _speed * seconds;
}

void Motion::accelerate(const Board& board, double seconds)
{
    double force = _parameters.engineForce - _parameters.friction * _speed;
    _speed += force / _parameters.mass * seconds;

    double maxSpeed = std::min(_parameters.maxSpeed, board.currentSegment->getMaxSpeed());
    if (_speed > maxSpeed) _speed = maxSpeed;
}

void Motion::brake(double seconds)
{
    double force = -_parameters.brakeForce - _parameters.friction * _speed;
    _speed += force / _parameters.mass * seconds;

    if (_speed < 0.0) _speed = 0.0;
}

const TrainParameters& Motion::getParameters() const { return _parameters; }
double Motion::getSpeed() const { return _speed; }
double Motion::getDistance() const { return _distance; }

double Motion::estimateTravelTime(const Segment& segment) const
{
    const double length = segment.getLength();
    const double limit = std::min(_parameters.maxSpeed, segment.getMaxSpeed());
    if (!std::isfinite(length) || length <= 0.0
        || !std::isfinite(segment.getMaxSpeed()) || segment.getMaxSpeed() <= 0.0)
        throw std::invalid_argument("Cannot estimate an invalid segment");
    const double acceleration = _parameters.engineForce / _parameters.mass;
    const double braking = _parameters.brakeForce / _parameters.mass;
    const double distanceFactor = 0.5 / acceleration + 0.5 / braking;
    const double peak = std::min(limit, std::sqrt(length / distanceFactor));
    const double cruiseDistance = std::max(0.0, length - peak * peak * distanceFactor);
    const double seconds = peak / acceleration + peak / braking + cruiseDistance / peak;
    if (!std::isfinite(seconds) || seconds <= 0.0)
        throw std::runtime_error("Segment duration exceeds supported range");
    return seconds;
}
