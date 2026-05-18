#include "network.hpp"
#include <functional>
#include <stdexcept>

void factory_init()
{
    Factory::instantiate();
    auto factory = Factory::instance();

    factory->registerCreator<Station>(
        "Station",
        std::function<Station(const FieldMap&)>([](const FieldMap& m)
        {
            return Station(
                m.at("name"),
                m.at("x"),
                m.at("y")
            );
        })
    );

    factory->registerCreator<Segment>(
        "Segment",
        std::function<Segment(const FieldMap&)>([](const FieldMap& m)
        {
            const std::string nameA = m.at("a");
            const std::string nameB = m.at("b");

            Station* stationA = Registry<Station>::instance()->find(
                [&](const Station& station) { return station.getName() == nameA; }
            );
            Station* stationB = Registry<Station>::instance()->find(
                [&](const Station& station) { return station.getName() == nameB; }
            );

            if (!stationA || !stationB)
                throw std::runtime_error("Segment references unknown station");

            return Segment(
                stationA,
                stationB,
                m.at("maxSpeed")
            );
        })
    );
}