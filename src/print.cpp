#include "network.hpp"

void printStations()
{
    for (size_t i = 0; i < Registry<Station>::instance()->size(); ++i)
    {
        auto* s = Registry<Station>::instance()->get(i);

        Logger::instance()->debug()
            << "[Station] "
            << s->getName()
            << " (" << s->getPosition().x
            << ", " << s->getPosition().y << ")";

        const auto& segments = s->getSegments();

        for (auto* seg : segments)
        {
            Logger::instance()->trace()
                << "   -> Segment to "
                << (seg->getStationA() == s
                        ? seg->getStationB()->getName()
                        : seg->getStationA()->getName())
                << " | maxSpeed: " << seg->getMaxSpeed()
                << " | length: " << seg->getLength();
        }
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
            << " | length: " << s->getLength()
            << " | speed: " << s->getMaxSpeed();
    }
}