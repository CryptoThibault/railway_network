#pragma once

class Train;
class Station;

enum class TrainEventType
{
    Departure,
    StationStop,
    Arrival,
};

struct TrainEvent
{
    TrainEventType type;
    const Train* train;
    const Station* station;
    double time;
    bool firstDeparture = false;
};
