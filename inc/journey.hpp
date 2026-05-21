#pragma once

class Station;

struct Journey
{
    Station* start;
    Station* end;
    uint32_t departure;
    uint32_t dwell{};
};