#pragma once
#include "train.hpp"
#include <vector>

class Pathfinder
{
public:
    static std::vector<Segment*> findPath(const Train& train);
};
