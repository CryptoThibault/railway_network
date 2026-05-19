#pragma once
#include "network.hpp"

class Station;

class Segment
{
public:
    Segment(Station* stationA, Station* stationB, double length, double maxSpeed);

    Station* getStationA() const;
    Station* getStationB() const;

    double getLength() const;
    double getMaxSpeed() const;

private:
    Station* _stationA;
    Station* _stationB;
    double _length;
    double _maxSpeed;
};