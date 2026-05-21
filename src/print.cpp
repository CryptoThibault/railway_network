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

void printTrains()
{
    // for (size_t i = 0; i < Registry<Train>::instance()->size(); ++i)
    // {
    //     auto* t = Registry<Train>::instance()->get(i);

    //     Logger::instance()->debug()
    //         << "[Train] "
    //         << t->getId()
    //         << " | type: "
    //         << t->getType()->name
    //         << " | station: "
    //         << (t->getCurrentStation()
    //                 ? t->getCurrentStation()->getName()
    //                 : "NONE");

    //     Logger::instance()->trace()
    //         << "   -> maxSpeed: "
    //         << t->getType()->maxSpeed
    //         << " | acceleration: "
    //         << t->getType()->acceleration
    //         << " | braking: "
    //         << t->getType()->braking
    //         << " | friction: "
    //         << t->getType()->friction
    //         << " | length: "
    //         << t->getType()->length
    //         << " | weight: "
    //         << t->getType()->weight;
    // }
}