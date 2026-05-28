#include "network.hpp"

void registry_init()
{
    Registry<Station>::instantiate();
    Registry<Segment>::instantiate();
    Registry<TrainType>::instantiate();
    Registry<Train>::instantiate();
    Registry<Journey>::instantiate();

    FieldMap data = Loader::loadJSON("data.json");
    auto factory = Factory::instance();

    auto stations = factory->createAll<Station>("Station", data.at("stations"));
    Registry<Station>::instance()->add(stations);

    auto segments = factory->createAll<Segment>("Segment", data.at("segments"));
    Registry<Segment>::instance()->add(segments);
    for (auto& seg : *Registry<Segment>::instance())
    {
        seg.getStationA()->addSegment(&seg);
        seg.getStationB()->addSegment(&seg);
    }

    FieldVector trainTypes = data.at("trainTypes");
    for (auto& field : trainTypes)
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

    FieldVector trainData = data.at("trains");
    for (const auto& trainField : trainData)
    {
        const FieldMap& m = trainField;
        
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
        
        Registry<Train>::instance()->emplace(++trainId, trainType, station);
    }
    
    auto journeys = factory->createAll<Journey>("Journey", data.at("journey"));
    Registry<Journey>::instance()->add(journeys);
}