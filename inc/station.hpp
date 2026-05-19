#pragma once
#include "network.hpp"
#include <string>
#include <vector>

using Vec2 = IVector2<double>;

class Segment;

class Station
{
public:
    Station(std::string name, double x, double y);

    const std::string& getName() const;
    const Vec2& getPosition() const;
    const std::vector<Segment*>& getSegments() const;

    void addSegment(Segment* segment);

private:
    std::string _name;
    Vec2 _position;
    std::vector<Segment*> _segments;
};