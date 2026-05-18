#include "network.hpp"

int main()
{
    factory_init();

    Registry<Station>::instantiate();
    Registry<Segment>::instantiate();
    Logger::instantiate();
    Logger::instance()->setLevel(Logger::Level::Debug);

    FieldMap data = Loader::loadJSON("data.json");

    auto stations = Factory::instance()->createAll<Station>("Station", data.at("stations"));
    Registry<Station>::instance()->add(stations);

    auto segments = Factory::instance()->createAll<Segment>("Segment", data.at("segments"));
    Registry<Segment>::instance()->add(segments);

    printStations();
    printSegments();

    return 0;
}