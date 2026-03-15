#include "Segment.hpp"

Station* Segment::getOtherStation(Station* current) const
{
    if (current == stationA)
        return stationB;
    if (current == stationB)
        return stationA;

    return nullptr;
}
