#pragma once

class Station;
class Segment;
struct Journey;

struct Board
{
    Station* currentStation;
    Segment* currentSegment{};
    double segmentProgress{};
    std::vector<Segment*> path{};
    size_t pathIndex{};
    const Journey* journey{};
};