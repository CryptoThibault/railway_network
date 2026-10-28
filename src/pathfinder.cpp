#include "pathfinder.hpp"
#include "station.hpp"
#include "segment.hpp"
#include <algorithm>
#include <cmath>
#include <queue>
#include <stdexcept>
#include <unordered_map>
#include <utility>

std::vector<Segment*> Pathfinder::findPath(const Train& train)
{
    Station* start = train.getBoard().currentStation;
    const Station* goal = &train.getEnd();
    const double trainSpeed = train.getMotion().getParameters().maxSpeed;
    const double pause = train.getPause();
    if (!start || start == goal || !std::isfinite(trainSpeed) || trainSpeed <= 0.0
        || !std::isfinite(pause) || pause < 0.0)
        throw std::invalid_argument("Invalid train route parameters");

    std::unordered_map<Station*, double> duration;
    std::unordered_map<Station*, Segment*> previous;
    using Entry = std::pair<double, Station*>;
    auto later = [](const Entry& first, const Entry& second)
    {
        return first.first > second.first;
    };
    std::priority_queue<Entry, std::vector<Entry>, decltype(later)> pending(later);
    duration.emplace(start, 0.0);
    pending.emplace(0.0, start);

    while (!pending.empty())
    {
        const auto [seconds, station] = pending.top();
        pending.pop();
        if (seconds > duration.at(station))
            continue;
        if (station == goal)
        {
            std::vector<Segment*> path;
            for (Station* current = station; current != start;)
            {
                Segment* segment = previous.at(current);
                path.push_back(segment);
                current = segment->getStationA() == current
                    ? segment->getStationB() : segment->getStationA();
            }
            std::reverse(path.begin(), path.end());
            return path;
        }

        for (auto* segment : station->getSegments())
        {
            if (!segment || !segment->getStationA() || !segment->getStationB()
                || (segment->getStationA() != station && segment->getStationB() != station)
                || !std::isfinite(segment->getLength()) || segment->getLength() <= 0.0
                || !std::isfinite(segment->getMaxSpeed()) || segment->getMaxSpeed() <= 0.0)
                throw std::runtime_error("Invalid route segment");
            Station* next = segment->getStationA() == station
                ? segment->getStationB() : segment->getStationA();
            const double travel = train.getMotion().estimateTravelTime(*segment);
            const double candidate = seconds + travel + (next == goal ? 0.0 : pause);
            if (!std::isfinite(candidate))
                throw std::runtime_error("Route duration exceeds supported range");
            const auto known = duration.find(next);
            if (known == duration.end() || candidate < known->second)
            {
                duration.insert_or_assign(next, candidate);
                previous.insert_or_assign(next, segment);
                pending.emplace(candidate, next);
            }
        }
    }
    throw std::runtime_error("No route from " + start->getName() + " to " + goal->getName());
}
