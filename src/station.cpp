#include "station.hpp"

Station::Station(std::string name, double x, double y) : _name(name), _position{x, y} {}

const std::string& Station::getName() const { return _name; }
const Vec2& Station::getPosition() const { return _position; }
const std::vector<Segment*>& Station::getSegments() const { return _segments; }

void Station::addSegment(Segment* segment)
{
    _segments.push_back(segment);
}