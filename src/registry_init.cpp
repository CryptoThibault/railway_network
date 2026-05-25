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

    auto trains = factory->createAll<Train>("Train", data.at("trains"));
    Registry<Train>::instance()->add(trains);
    
    auto journeys = factory->createAll<Journey>("Journey", data.at("journey"));
    Registry<Journey>::instance()->add(journeys);
}