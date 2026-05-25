#include "network.hpp"

void printStations()
{
    for (auto& s : *Registry<Station>::instance())
    {
        Logger::instance()->debug()
            << "[Station] "
            << s.getName()
            << " (" << s.getPosition().x
            << ", " << s.getPosition().y << ")";

        for (auto* seg : s.getSegments())
        {
            Station* next =
                (seg->getStationA() == &s)
                ? seg->getStationB()
                : seg->getStationA();

            Logger::instance()->trace()
                << "   . Segment to " << next->getName()
                << " | maxSpeed: " << seg->getMaxSpeed()
                << " | length: " << seg->getLength();
        }
    }
}

void printSegments()
{
    for (auto& s : *Registry<Segment>::instance())
    {

        Logger::instance()->debug()
            << "[Segment] "
            << s.getStationA()->getName()
            << " - "
            << s.getStationB()->getName()
            << " | length: " << s.getLength()
            << " | speed: " << s.getMaxSpeed();
    }
}

void printTrains()
{
    for (auto& t : *Registry<Train>::instance())
    {
        Logger::instance()->debug()
            << "[Train] "
            << t.getId()
            << " | type: "
            << t.getType()->name
            << " | speed: "
            << t.getMotion().getSpeed()
            << " | distance: "
            << t.getMotion().getDistance()
            << " | station: "
            << t.getBoard().currentStation->getName();

        Logger::instance()->trace()
            << "   . mass: "
            << t.getType()->mass
            << " | maxSpeed: "
            << t.getType()->maxSpeed
            << " | engineForce: "
            << t.getType()->engineForce
            << " | brakeForce: "
            << t.getType()->brakeForce
            << " | friction: "
            << t.getType()->friction
            << " | length: "
            << t.getType()->length;
    }
}