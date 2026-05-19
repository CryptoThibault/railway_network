#include "network.hpp"

void registry_init()
{
    Registry<Station>::instantiate();
    Registry<Segment>::instantiate();

    FieldMap data = Loader::loadJSON("data.json");

    auto stations = Factory::instance()->createAll<Station>("Station", data.at("stations"));
    Registry<Station>::instance()->add(stations);

    auto segments = Factory::instance()->createAll<Segment>("Segment", data.at("segments"));
    Registry<Segment>::instance()->add(segments);

    for (size_t i = 0; i < Registry<Segment>::instance()->size(); ++i)
    {
        auto* seg = Registry<Segment>::instance()->get(i);

        seg->getStationA()->addSegment(seg);
        seg->getStationB()->addSegment(seg);
    }

}