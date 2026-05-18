#include "station.hpp"

Station::Station(std::string name, double x, double y)
    : _name(name), _x(x), _y(y) {}

const std::string& Station::getName() const { return _name; }
double Station::getX() const { return _x; }
double Station::getY() const { return _y; }
const std::vector<Segment*>& Station::getSegments() const { return _segments; }

void Station::addSegment(Segment* segment)
{
    _segments.push_back(segment);
}