#include "Traveler.hpp"
#include "Network.hpp"
#include "Train.hpp"
#include <queue>
#include <map>
#include <algorithm>

struct PassengerState {
    Station* station;
    double arrivalTime;
};

struct CompareArrivalTime {
    bool operator()(const PassengerState& a, const PassengerState& b) {
        return a.arrivalTime > b.arrivalTime;
    }
};

double Traveler::computeFastestTravelTime(double departureTime)
{
    auto& network = Network::getInstance();

    std::priority_queue<PassengerState, std::vector<PassengerState>, CompareArrivalTime> pq;
    pq.push({start, departureTime});

    std::map<Station*, double> bestArrival;

    while (!pq.empty()) {
        PassengerState current = pq.top();
        pq.pop();

        if (current.station == end) {
            double travelTime = current.arrivalTime - departureTime;
            bestTimes.push_back(travelTime);
            return travelTime;
        }

        if (bestArrival.find(current.station) != bestArrival.end() &&
            current.arrivalTime >= bestArrival[current.station])
            continue;

        bestArrival[current.station] = current.arrivalTime;


        for (Train* t : network.trainsPassingThrough(current.station)) {
            double trainDeparture = std::max(current.arrivalTime, t->getNextDeparture(current.station));
            double travelTime = t->getSegmentTimeFrom(current.station);
            double stopTime = t->getBoard().getStopDuration();

            Station* nextStation = t->getNextStationFrom(current.station);
            if (!nextStation) continue;

            pq.push({nextStation, trainDeparture + travelTime + stopTime});
        }
    }

    return -1.0;
}

double Traveler::getAverageTravelTime() const
{
    if (bestTimes.empty()) return  0.55;
    double sum = 0;
    for (double t : bestTimes) sum += t;
    return sum / bestTimes.size();
}