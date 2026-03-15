#pragma once
#include "Station.hpp"
#include "Segment.hpp"

class Train;

class Board
{
private:
    double stopDuration;

    Segment* currentSegment;
    double distanceOnSegment;
    double stopTimeRemaining;
    Station* nextStation;
    Train* train;

public:
    Board(double stopDuration_, Train* train_)
        : stopDuration(stopDuration_),
          currentSegment(nullptr),
          distanceOnSegment(0),
          stopTimeRemaining(0),
          nextStation(nullptr),
          train(train_)
    {}

    void updatePosition(double dt, double speed);
    void nextStationReached();
    void decreaseStopTime(double dt) { stopTimeRemaining -= dt; }
    
    void setCurrentSegment(Segment* seg) { currentSegment = seg; }
    void setNextStation(Station* station) { nextStation = station; }
    void setDistanceOnSegment(double d) { distanceOnSegment = d; }
    void setStopTimeRemaining(double t) { stopTimeRemaining = t; }
    
    int getStopDuration() const { return stopDuration; }
    double getDistanceOnSegment() const { return distanceOnSegment; }
    Segment* getCurrentSegment() const { return currentSegment; }
    Station* getNextStation() const { return nextStation; }
    double getStopTimeRemaining() const { return stopTimeRemaining; }
    bool isAtStation() const { return stopTimeRemaining > 0; }
};