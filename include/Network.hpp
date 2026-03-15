#pragma once
#include <vector>
#include <unordered_map>
#include "EventManager.hpp"

class Train;
class Segment;
class Station;

class Network : public EventManager
{
private:
    Network() {}

    static const bool ENABLE_PRINT = false;
    std::unordered_map<Train*, std::vector<Segment*>> trainPaths;

    std::vector<Segment*> findPath(Station* start, Station* goal);
    
    void closeRandomStation();
    void stopRandomSegment();
    void reduceRandomSegmentSpeed();
    void delayRandomTrains();

public:
    static Network& getInstance() {
        static Network instance;
        return instance;
    }

    Network(const Network&) = delete;
    Network& operator=(const Network&) = delete;

    Segment* planNextSegment(Train* train);
    double calculatePathDistance(Train* train) const;
    std::vector<Train*> trainsPassingThrough(Station* station);
    void onEvent(Event e) override;

    bool hasPath(Train* train) const {return trainPaths.find(train) != trainPaths.end(); }
    const std::vector<Segment*>& getPath(Train* train) { return trainPaths.at(train); }
};