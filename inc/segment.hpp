#pragma once

class Station;

class Segment
{
public:
    Segment(Station* stationA, Station* stationB, double maxSpeed);

    Station* getStationA() const;
    Station* getStationB() const;

    double getMaxSpeed() const;
    double getLength() const;

private:
    Station* _stationA;
    Station* _stationB;
    double _maxSpeed;
    double _length;
};