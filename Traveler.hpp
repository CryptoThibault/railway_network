#pragma once
#include <vector>
#include "Station.hpp"

class Traveler
{
private:
    Station* start;
    Station* end;
    std::vector<double> bestTimes;

public:
    Traveler(Station* start, Station* end) : start(start), end(end) {}

    double computeFastestTravelTime(double departureTime);

    Station* getStart() const { return start; }
    Station* getEnd() const { return end; }
    double getAverageTravelTime() const;
};