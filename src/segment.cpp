#include "segment.hpp"
#include "station.hpp"

Segment::Segment(Station* stationA, Station* stationB, double length, double maxSpeed)
    : _stationA(stationA), _stationB(stationB), _length(length), _maxSpeed(maxSpeed) {}

Station* Segment::getStationA() const { return _stationA; }
Station* Segment::getStationB() const { return _stationB; }
double Segment::getLength() const { return _length; }
double Segment::getMaxSpeed() const { return _maxSpeed; }
