#include "Network.hpp"
#include "Train.hpp"
#include "Segment.hpp"
#include "Registry.hpp"
#include "Station.hpp"
#include <iostream>
#include <queue>
#include <limits>
#include <algorithm>

std::vector<Segment*> Network::findPath(Station* start, Station* goal)
{
    std::unordered_map<Station*, double> dist;
    std::unordered_map<Station*, Segment*> previous;
    std::priority_queue<
        std::pair<double, Station*>,
        std::vector<std::pair<double, Station*>>,
        std::greater<>
    > pq;

    for (auto& st : Registry<Station>::getInstance().getList())
        dist[st.get()] = std::numeric_limits<double>::infinity();

    dist[start] = 0;
    pq.push({0, start});

    while (!pq.empty())
    {
        std::pair<double, Station*> top = pq.top();
        pq.pop();

        double d = top.first;
        Station* station = top.second;
        if (station == goal) break;

        for (const auto& seg : Registry<Segment>::getInstance().getList())
        {
            if (seg->getStationA() != station && seg->getStationB() != station) continue;

            Station* next = seg->getOtherStation(station);
            double newDist = d + seg->getLength();

            if (newDist < dist[next])
            {
                dist[next] = newDist;
                previous[next] = seg.get();
                pq.push({newDist, next});
            }
        }
    }

    std::vector<Segment*> path;
    Station* current = goal;
    while (current != start)
    {
        Segment* seg = previous[current];
        path.push_back(seg);
        current = seg->getOtherStation(current);
    }
    std::reverse(path.begin(), path.end());
    return path;
}

Segment* Network::planNextSegment(Train* train)
{
    auto& board = train->getBoard();

    if (!hasPath(train))
        trainPaths[train] = findPath(train->getDeparture(), train->getArrival());

    auto& path = trainPaths[train];
    if (path.empty()) return nullptr;

    Segment* currSeg = board.getCurrentSegment();
    Station* currentStation = nullptr;
    if (!currSeg) {
        currentStation = train->getDeparture();
    } else {
        currentStation = board.getNextStation();
        if (!currentStation)
            currentStation = train->getDeparture();
    }

    size_t index = 0;
    if (currSeg)
    {
        auto it = std::find(path.begin(), path.end(), currSeg);
        if (it == path.end()) return nullptr;
        index = std::distance(path.begin(), it) + 1;
    }

    if (index >= path.size()) return nullptr;

    Segment* nextSeg = path[index];
    board.setCurrentSegment(nextSeg);
    board.setNextStation(nextSeg->getOtherStation(currentStation));
    board.setDistanceOnSegment(0);

    return nextSeg;
}

double Network::calculatePathDistance(Train* train) const
{
    if (!hasPath(train)) return 0.0;

    const auto& path = trainPaths.at(train);
    Segment* currSeg = train->getBoard().getCurrentSegment();
    if (!currSeg) return 0.0;

    auto it = std::find(path.begin(), path.end(), currSeg);
    if (it == path.end()) return 0.0;

    size_t index = std::distance(path.begin(), it);

    double distanceRemaining = 0.0;

    distanceRemaining += currSeg->getLength() - train->getBoard().getDistanceOnSegment();

    for (size_t i = index + 1; i < path.size(); ++i)
        distanceRemaining += path[i]->getLength();

    return distanceRemaining;
}

std::vector<Train*> Network::trainsPassingThrough(Station* station)
{
    std::vector<Train*> result;

    for (const auto& trainPtr : Registry<Train>::getInstance().getList()) {

        Train* t = trainPtr.get();

        auto it = trainPaths.find(t);
        if (it == trainPaths.end())
            continue;

        const std::vector<Segment*>& path = it->second;

        for (Segment* seg : path) {
            if (seg->getStationA() == station || seg->getStationB() == station) {
                result.push_back(t);
                break;
            }
        }
    }

    return result;
}

void Network::onEvent(Event e)
{
    switch(e)
    {
        case Event::BombAlert:
            closeRandomStation();
            break;
        case Event::PowerOutage:
            stopRandomSegment();
            break;
        case Event::TrackMaintenance:
            reduceRandomSegmentSpeed();
            break;
        case Event::SignalFailure:
            delayRandomTrains();
            break;
    }
}

void Network::closeRandomStation()
{
    if (ENABLE_PRINT)
        std::cout << "Event: Bomb Alert - Closing a random station\n";
    auto& stations = Registry<Station>::getInstance().getList();
    if (stations.size() < 5) return;

    int i = rand() % stations.size();
    Station* s = stations[i].get();
    s->setClosed(true);
    s->setCloseTicks(100);
}

void Network::stopRandomSegment() {
    if (ENABLE_PRINT)
        std::cout << "Event: Power Outage - Stopping a random segment\n";
    auto& segments = Registry<Segment>::getInstance().getList();
    if (segments.size() < 5) return;
    Segment* seg = segments[rand() % segments.size()].get();
    for (Train* t : trainsPassingThrough(seg->getStationA())) {
        if (t->getBoard().getCurrentSegment() == seg) {
            t->stopTrain();
        }
    }
    for (Train* t : trainsPassingThrough(seg->getStationB())) {
        if (t->getBoard().getCurrentSegment() == seg) {
            t->stopTrain();
        }
    }
}

void Network::reduceRandomSegmentSpeed() {
    if (ENABLE_PRINT)
        std::cout << "Event: Track Maintenance - Reducing speed on a random segment\n";
    auto& segments = Registry<Segment>::getInstance().getList();
    if (segments.size() < 5) return;
    segments[rand() % segments.size()]->setMaxAllowedSpeed(segments[rand() % segments.size()]->getMaxAllowedSpeed() * 0.5);
}

void Network::delayRandomTrains() {
    if (ENABLE_PRINT)
        std::cout << "Event: Signal Failure - Delaying random trains\n";
    auto& segments = Registry<Train>::getInstance().getList();
    if (segments.size() < 5) return;
    Train* t = segments[rand() % segments.size()].get();
    t->getBoard().setStopTimeRemaining(t->getBoard().getStopTimeRemaining() + 0.5);
}