#pragma once
#include <vector>
#include <stdexcept>
#include "Station.hpp"

class Traveler
{
private:
    Station* start;
    Station* end;
    std::vector<double> bestTimes;

public:
    Traveler(Station* start, Station* end) : start(start), end(end)
    {
        if (!start || !end) {
            throw std::invalid_argument("Traveler requires a valid start and end station");
        }
    }

    double computeFastestTravelTime(double departureTime);

    Station* getStart() const { return start; }
    Station* getEnd() const { return end; }
    double getAverageTravelTime() const;
};