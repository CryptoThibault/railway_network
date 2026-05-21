#include "network.hpp"
#include <functional>
#include <stdexcept>

void factory_init()
{
    Factory::instantiate();
    auto factory = Factory::instance();

    factory->registerCreator<Station>(
        "Station",
        [](const FieldMap& m)
        {
            return Station(
                m.at("name"),
                m.at("x"),
                m.at("y")
            );
        }
    );

    factory->registerCreator<Segment>(
        "Segment",
        [](const FieldMap& m)
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
                m.at("length"),
                m.at("maxSpeed")
            );
        }
    );

    factory->registerCreator<Train>(
        "Train",
        [](const FieldMap& m)
        {
            static long id = 0;

            TrainType* type = Registry<TrainType>::instance()->find(
                [&](const TrainType& type)
                {
                    return type.name == static_cast<std::string>(m.at("type"));
                }
            );


            Station* station = Registry<Station>::instance()->find(
                [&](const Station& s)
                {
                    return s.getName() ==
                        static_cast<std::string>(m.at("station"));
                }
            );

            if (!station)
                throw std::runtime_error("Train references unknown station");

            return Train(
                id++,
                type,
                station
            );
        }
    );

    factory->registerCreator<Journey>(
        "Journey",
        [](const FieldMap& m)
        {
            const std::string nameStart = m.at("start");
            const std::string nameEnd = m.at("end");

            Station* start = Registry<Station>::instance()->find(
                [&](const Station& station) { return station.getName() == nameStart; }
            );
            Station* end = Registry<Station>::instance()->find(
                [&](const Station& station) { return station.getName() == nameEnd; }
            );

            if (!start || !end)
                throw std::runtime_error("Journey references unknown station");

            long departure = m.at("departure");
            long dwell = m.at("dwell");
            return Journey(
                start,
                end,
                static_cast<uint32_t>(departure),
                static_cast<uint32_t>(dwell)
            );
        }
    );
}