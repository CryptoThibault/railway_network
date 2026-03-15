#pragma once
#include "Station.hpp"

class Segment
{
private:
    Station* stationA;
    Station* stationB;
    double length;
    double maxAllowedSpeed;

public:
    Segment(Station* a, Station* b, double l, double mas)
        : stationA(a), stationB(b), length(l), maxAllowedSpeed(mas)
    {
        stationA->addSegment(this);
        stationB->addSegment(this);
    }

    void setMaxAllowedSpeed(double speed) { maxAllowedSpeed = speed; }

    Station* getStationA() const { return stationA; }
    Station* getStationB() const { return stationB; }
    Station* getOtherStation(Station* current) const;
    double getLength() const { return length; }
    double getMaxAllowedSpeed() const { return maxAllowedSpeed; }

};