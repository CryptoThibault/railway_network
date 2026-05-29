#include "initializer.hpp"
#include <functional>
#include <stdexcept>

void Initializer::initialize()
{
    Logger::instantiate();
    Logger::instance()->setLevel(Logger::Level::LOG_LEVEL);

    initFactory();

    FieldMap data = Loader::loadJSON("data.json");

    initStations(data.at("stations"));
    initSegments(data.at("segments"));
    initTrainTypes(data.at("trainTypes"));
    initTrains(data.at("trains"));
    initJourneys(data.at("journey"));
}

void Initializer::initFactory()
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
            static long trainId = 0;

            TrainType trainType = *Registry<TrainType>::instance()->find(
                [&](const TrainType& type)
                {
                    return type.name == static_cast<std::string>(m.at("type"));
                }
            );
            
            Station* station = Registry<Station>::instance()->find(
                [&](const Station& s)
                {
                    return s.getName() == static_cast<std::string>(m.at("station"));
                }
            );
            
            if (!station)
                throw std::runtime_error("Train references unknown station");

            return Train(
                ++trainId,
                trainType,
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

void Initializer::initStations(const FieldVector& data)
{
    Registry<Station>::instantiate();
    auto stations = Factory::instance()->createAll<Station>("Station", data);
    Registry<Station>::instance()->add(stations);
}

void Initializer::initSegments(const FieldVector& data)
{
    Registry<Segment>::instantiate();
    auto segments = Factory::instance()->createAll<Segment>("Segment", data);
    Registry<Segment>::instance()->add(segments);
    for (auto& seg : *Registry<Segment>::instance())
    {
        seg.getStationA()->addSegment(&seg);
        seg.getStationB()->addSegment(&seg);
    }
}

void Initializer::initTrainTypes(const FieldVector& data)
{
    Registry<TrainType>::instantiate();
    for (const auto& field : data)
    {
        FieldMap m = field;
        Registry<TrainType>::instance()->add(
            TrainType{
                m.at("name"),
                m.at("mass"),
                m.at("maxSpeed"),
                m.at("engineForce"),
                m.at("brakeForce"),
                m.at("friction"),
                m.at("length")
            }
        );
    }
}

void Initializer::initTrains(const FieldVector& data)
{
    Registry<Train>::instantiate();
    auto trains = Factory::instance()->createAll<Train>("Train", data);
    Registry<Train>::instance()->add(trains);
    for (auto& train : *Registry<Train>::instance())
        train.state_machine_init();
}

void Initializer::initJourneys(const FieldVector& data)
{
    Registry<Journey>::instantiate();
    auto journeys = Factory::instance()->createAll<Journey>("Journey", data);
    Registry<Journey>::instance()->add(journeys);
}
