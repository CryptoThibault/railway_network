#include "station.hpp"
#include <cmath>
#include <stdexcept>

Station::Station(std::string name, double x, double y) : _name(name), _position{x, y}
{
    if (name.empty() || !std::isfinite(x) || !std::isfinite(y)
        || std::abs(x) > 20037508.342789244 || std::abs(y) > 20037508.342789244)
        throw std::invalid_argument("Invalid station name or EPSG:3857 coordinates");
}

const std::string& Station::getName() const { return _name; }
const Vec2& Station::getPosition() const { return _position; }
const std::vector<Segment*>& Station::getSegments() const { return _segments; }

void Station::addSegment(Segment* segment)
{
    _segments.push_back(segment);
}