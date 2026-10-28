#pragma once
#include <cstddef>
#include <vector>

class Station;
class Segment;

struct Board
{
    Station* currentStation;
    Segment* currentSegment{};
    double segmentProgress{};
    std::vector<Segment*> path{};
    std::size_t pathIndex{};
};