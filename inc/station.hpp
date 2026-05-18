#pragma once
#include <string>
#include <vector>

class Segment;

class Station
{
public:
    Station(std::string name, double x, double y);

    const std::string& getName() const;
    double getX() const;
    double getY() const;
    const std::vector<Segment*>& getSegments() const;

    void addSegment(Segment* segment);

private:
    std::string _name;
    double _x;
    double _y;
    std::vector<Segment*> _segments;
};