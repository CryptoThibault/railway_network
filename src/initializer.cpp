#include "initializer.hpp"
#include "station.hpp"
#include "segment.hpp"
#include "train.hpp"
#include "factory.hpp"
#include "loader.hpp"
#include <algorithm>
#include "registry.hpp"
#include <cctype>
#include <cmath>
#include <stdexcept>
#include <unordered_set>

static const Field& requiredField(const FieldMap& data, const std::string& key)
{
    const auto found = data.find(key);
    if (found == data.end())
        throw std::invalid_argument("Missing field: " + key);
    return found->second;
}

static std::string readName(const FieldMap& data, const std::string& key)
{
    const auto& field = requiredField(data, key);
    if (!field.isString())
        throw std::invalid_argument("Expected a string for " + key);
    const std::string value = field;
    if (value.empty() || std::isspace(static_cast<unsigned char>(value.front()))
        || std::isspace(static_cast<unsigned char>(value.back()))
        || std::any_of(value.begin(), value.end(), [](unsigned char character) { return std::iscntrl(character); }))
        throw std::invalid_argument("Invalid name in field: " + key);
    return value;
}

static double readNumber(const FieldMap& data, const std::string& key)
{
    const auto& field = requiredField(data, key);
    if (!field.isNumber())
        throw std::invalid_argument("Expected a number for " + key);
    const double value = field;
    if (!std::isfinite(value))
        throw std::invalid_argument("Expected a finite number for " + key);
    return value;
}

static FieldVector readCollection(const FieldMap& data, const std::string& key)
{
    const auto& field = requiredField(data, key);
    if (!field.isVector())
        throw std::invalid_argument("Expected an array for " + key);
    const FieldVector values = field;
    for (const auto& value : values)
        if (!value.isMap())
            throw std::invalid_argument("Expected objects in " + key);
    return values;
}

void Initializer::initialize(const std::string& path)
{
    const Field root = Loader::loadJSON(path);
    if (!root.isMap())
        throw std::invalid_argument("Scenario must be an object");
    const FieldMap data = root;
    const auto stations = readCollection(data, "stations");
    const auto segments = readCollection(data, "segments");
    const auto trains = readCollection(data, "trains");
    initFactory();
    initStations(stations);
    initSegments(segments);
    initTrains(trains);
}

void Initializer::initFactory()
{
    Factory::instantiate();
    auto factory = Factory::instance();
    factory->registerCreator<Station>("Station", [](const FieldMap& data)
    {
        return Station(readName(data, "name"), readNumber(data, "x"), readNumber(data, "y"));
    });
    factory->registerCreator<Segment>("Segment", [](const FieldMap& data)
    {
        const auto nameA = readName(data, "a");
        const auto nameB = readName(data, "b");
        auto* stations = Registry<Station>::instance();
        auto* stationA = stations->find([&](const Station& station) { return station.getName() == nameA; });
        auto* stationB = stations->find([&](const Station& station) { return station.getName() == nameB; });
        if (!stationA || !stationB)
            throw std::invalid_argument("Segment references unknown station: " + nameA + " - " + nameB);
        return Segment(stationA, stationB, readNumber(data, "length"), readNumber(data, "maxSpeed"));
    });
    factory->registerCreator<Train>("Train", [](const FieldMap& data)
    {
        const auto name = readName(data, "name");
        const auto nameStart = readName(data, "start");
        const auto nameEnd = readName(data, "end");
        auto* stations = Registry<Station>::instance();
        auto* start = stations->find([&](const Station& station) { return station.getName() == nameStart; });
        auto* end = stations->find([&](const Station& station) { return station.getName() == nameEnd; });
        if (!start || !end)
            throw std::invalid_argument("Train references unknown station: " + name);
        const TrainParameters parameters{
            readNumber(data, "mass"), readNumber(data, "maxSpeed"),
            readNumber(data, "engineForce"), readNumber(data, "brakeForce"),
            readNumber(data, "friction"), readNumber(data, "length")
        };
        static long trainId = 0;
        return Train(++trainId, name, parameters, start, end,
            readNumber(data, "departure"), readNumber(data, "pause"));
    });
}

void Initializer::initStations(const FieldVector& data)
{
    auto stations = Factory::instance()->createAll<Station>("Station", data);
    std::unordered_set<std::string> names;
    for (const auto& station : stations)
        if (!names.insert(station.getName()).second)
            throw std::invalid_argument("Duplicate station name: " + station.getName());
    Registry<Station>::instantiate();
    Registry<Station>::instance()->add(stations);
}

void Initializer::initSegments(const FieldVector& data)
{
    auto segments = Factory::instance()->createAll<Segment>("Segment", data);
    Registry<Segment>::instantiate();
    Registry<Segment>::instance()->add(segments);
    for (auto& segment : *Registry<Segment>::instance())
    {
        segment.getStationA()->addSegment(&segment);
        segment.getStationB()->addSegment(&segment);
    }
}

void Initializer::initTrains(const FieldVector& data)
{
    auto trains = Factory::instance()->createAll<Train>("Train", data);
    Registry<Train>::instantiate();
    Registry<Train>::instance()->add(trains);
    for (auto& train : *Registry<Train>::instance())
        train.state_machine_init();
}
