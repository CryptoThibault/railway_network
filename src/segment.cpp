#include "segment.hpp"
#include "station.hpp"
#include <cmath>
#include <stdexcept>

Segment::Segment(Station* stationA, Station* stationB, double length, double maxSpeed)
    : _stationA(stationA), _stationB(stationB), _length(length), _maxSpeed(maxSpeed)
{
    if (!stationA || !stationB || stationA == stationB
        || !std::isfinite(length) || length <= 0.0 || !std::isfinite(maxSpeed) || maxSpeed <= 0.0)
        throw std::invalid_argument("Invalid segment parameters");
}

Station* Segment::getStationA() const { return _stationA; }
Station* Segment::getStationB() const { return _stationB; }
double Segment::getLength() const { return _length; }
double Segment::getMaxSpeed() const { return _maxSpeed; }
