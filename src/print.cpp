#include "network.hpp"

void printStations()
{
    for (size_t i = 0; i < Registry<Station>::instance()->size(); ++i)
    {
        auto* s = Registry<Station>::instance()->get(i);

        Logger::instance()->debug()
            << "[Station] "
            << s->getName()
            << " (" << s->getX()
            << ", " << s->getY() << ")";
    }
}

void printSegments()
{
    for (size_t i = 0; i < Registry<Segment>::instance()->size(); ++i)
    {
        auto* s = Registry<Segment>::instance()->get(i);

        Logger::instance()->debug()
            << "[Segment] "
            << s->getStationA()->getName()
            << " - "
            << s->getStationB()->getName()
            << " | speed: " << s->getMaxSpeed()
            << " | length: " << s->getLength();
    }
}