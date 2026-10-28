#include "motion.hpp"
#include "network_simulation.hpp"
#include "pathfinder.hpp"
#include "printer.hpp"
#include "segment.hpp"
#include "station.hpp"
#include "train.hpp"
#include <array>
#include <cmath>
#include <functional>
#include <iostream>
#include <limits>
#include <random>
#include <stdexcept>
#include <string>
#include <vector>

static void require(bool condition, const std::string& message)
{
    if (!condition)
        throw std::runtime_error(message);
}

static void requireThrows(const std::function<void()>& action, const std::string& message)
{
    bool threw = false;
    try
    {
        action();
    }
    catch (const std::exception&)
    {
        threw = true;
    }
    require(threw, message);
}

static void checkEstimates()
{
    Station start("Start", 0.0, 0.0);
    Station end("End", 1.0, 1.0);
    const TrainParameters parameters{100.0, 10.0, 100.0, 100.0, 0.0, 10.0};
    Motion motion(parameters);
    const Segment longSegment(&start, &end, 200.0, 100.0);
    const Segment shortSegment(&start, &end, 25.0, 100.0);
    const Segment boundary(&start, &end, 100.0, 10.0);
    const Segment limited(&start, &end, 25.0, 2.0);
    require(std::abs(motion.estimateTravelTime(longSegment) - 30.0) < 1e-9,
        "Long-segment estimate must include acceleration, cruise, and braking");
    require(std::abs(motion.estimateTravelTime(shortSegment) - 10.0) < 1e-9,
        "Short-segment estimate must use a triangular speed profile");
    require(std::abs(motion.estimateTravelTime(boundary) - 20.0) < 1e-9,
        "Estimate must be continuous at the cruise threshold");
    require(std::abs(motion.estimateTravelTime(limited) - 14.5) < 1e-9,
        "Estimate must respect the segment speed limit");
    const Motion asymmetric({100.0, 10.0, 200.0, 100.0, 0.0, 10.0});
    require(std::abs(asymmetric.estimateTravelTime(Segment(&start, &end, 150.0, 10.0)) - 22.5) < 1e-9,
        "Estimate must distinguish engine and brake forces");
    require(motion.getSpeed() == 0.0 && motion.getDistance() == 0.0,
        "Estimating a route must not move the train");
    requireThrows([] { Motion invalid({0.0, 10.0, 100.0, 100.0, 0.0, 10.0}); },
        "Zero mass must be rejected");
    requireThrows([] { Motion invalid({100.0, 10.0, 0.0, 100.0, 0.0, 10.0}); },
        "Zero engine force must be rejected");
    requireThrows([] { Motion invalid({100.0, 10.0, 100.0, 0.0, 0.0, 10.0}); },
        "Zero brake force must be rejected");
    requireThrows([] { Motion invalid({100.0, 10.0, 100.0, 100.0, -1.0, 10.0}); },
        "Negative friction must be rejected");
    requireThrows([] { Station invalid("Invalid", std::numeric_limits<double>::quiet_NaN(), 0.0); },
        "Nonfinite coordinates must be rejected");
    requireThrows([&] { Segment invalid(&start, &end, std::numeric_limits<double>::infinity(), 10.0); },
        "Nonfinite segment lengths must be rejected");
}

static void checkLifecycle()
{
    Station start("Start", 0.0, 0.0);
    Station middle("Middle", 1.0, 0.0);
    Station end("End", 2.0, 0.0);
    Segment first(&start, &middle, 1000.0, 20.0);
    Segment second(&middle, &end, 1000.0, 20.0);
    Train train(1, "Lifecycle", {100.0, 20.0, 100.0, 100.0, 0.0, 10.0}, &start, &end, 0.0, 3.0);
    train.state_machine_init();
    Simulation service(train, {&first, &second});
    require(train.getState() == TrainState::Scheduled, "New services must remain Scheduled");
    require(Printer::formatTrainState(train, 0.0).find("State: Scheduled") != std::string::npos,
        "Scheduled state must be presented directly");
    requireThrows([&] { train.reset(); }, "Unfinished trains must not reset");
    requireThrows([&] { service.restart(); }, "Unfinished services must not restart");
    requireThrows([&] { train.update(0.0); }, "Zero time steps must be rejected");
    requireThrows([&] { train.transitionTo(TrainState::Completed); }, "Invalid transitions must be rejected");

    std::vector<TrainEvent> events;
    double lastDistance = 0.0;
    bool waiting = false;
    bool accelerating = false;
    bool cruising = false;
    bool braking = false;
    unsigned long steps = 0;
    while (!service.isFinished() && steps++ < 10000)
    {
        const auto emitted = service.update();
        for (const auto& event : emitted)
        {
            require(event.train == &train, "Event must identify the original train");
            require(event.time == service.getElapsedSeconds(), "Event must carry the current step time");
            if (event.type == TrainEventType::Departure)
                require(event.station == (event.firstDeparture ? &start : &middle),
                    "Departure must identify its actual station");
            if (event.type == TrainEventType::StationStop)
                require(train.getState() == TrainState::Waiting && event.station == &middle,
                    "Intermediate stops must enter Waiting");
            if (event.type == TrainEventType::Arrival)
                require(train.getState() == TrainState::Completed && event.station == &end,
                    "Arrival must enter Completed");
            events.push_back(event);
        }
        const auto state = train.getState();
        waiting = waiting || state == TrainState::Waiting;
        accelerating = accelerating || state == TrainState::Accelerating;
        cruising = cruising || state == TrainState::Cruising;
        braking = braking || state == TrainState::Braking;
        require(train.getMotion().getSpeed() >= 0.0 && train.getMotion().getSpeed() <= 20.0,
            "Speed must remain within physical limits");
        require(train.getMotion().getDistance() >= lastDistance, "Trip distance must be monotonic");
        require(train.getBoard().segmentProgress >= 0.0 && train.getBoard().segmentProgress <= 1000.0,
            "Position must stay within its active segment");
        lastDistance = train.getMotion().getDistance();
        if (state == TrainState::Waiting || state == TrainState::Completed)
            require(train.getMotion().getSpeed() == 0.0 && train.getBoard().currentStation,
                "Stationary lifecycle states must have a station and zero speed");
    }
    require(service.isFinished() && waiting && accelerating && cruising && braking,
        "Lifecycle must finish and visit every movement phase");
    require(events.size() == 4, "Two legs must emit exactly departure, stop, departure, arrival");
    require(events[0].type == TrainEventType::Departure && events[0].firstDeparture,
        "First event must be a trip departure");
    require(events[1].type == TrainEventType::StationStop && !events[2].firstDeparture,
        "Intermediate departures must not be labelled as new trips");
    require(std::abs(events[2].time - events[1].time - 3.0) < 0.11,
        "Intermediate pauses must respect the configured duration");
    const double estimate = train.getMotion().estimateTravelTime(first)
        + train.getMotion().estimateTravelTime(second) + train.getPause();
    require(std::abs(service.getElapsedSeconds() - estimate) < 1.0,
        "Constant-force estimates should agree with friction-free simulation within integration error");
    require(service.update().empty(), "Completed services must not emit duplicate arrivals");
    require(Printer::formatTrainState(train, 0.0).find("State: Completed") != std::string::npos,
        "Completed state must be presented directly");
    service.restart();
    require(train.getState() == TrainState::Scheduled && train.getBoard().currentStation == &start,
        "Restart must restore the origin and Scheduled state");
    require(train.getMotion().getDistance() == 0.0 && train.getMotion().getSpeed() == 0.0
        && train.getBoard().pathIndex == 0 && train.getBoard().segmentProgress == 0.0,
        "Restart must clear all movement counters");
    const auto restarted = service.update();
    require(restarted.size() == 1 && restarted.front().firstDeparture
        && restarted.front().station == &start, "Restarted service must emit a new origin departure");
}

static void checkNetworkEvents()
{
    Station start("Start", 0.0, 0.0);
    Station end("End", 1.0, 0.0);
    Segment segment(&start, &end, 100.0, 10.0);
    start.addSegment(&segment);
    end.addSegment(&segment);
    const TrainParameters parameters{100.0, 10.0, 100.0, 100.0, 0.0, 10.0};
    Registry<Train>::instantiate();
    auto& trains = *Registry<Train>::instance();
    trains.add(Train(1, "Early", parameters, &start, &end, 10.0, 0.0));
    trains.add(Train(2, "Late", parameters, &end, &start, 20.0, 0.0));
    for (auto& train : trains)
        train.state_machine_init();
    NetworkSimulation simulation(trains, false);
    require(simulation.getEvents().empty(), "No events should exist before the first update");
    require(trains.get(0)->getState() == TrainState::Scheduled
        && trains.get(1)->getState() == TrainState::Scheduled, "Services must await scheduled departures");
    std::vector<unsigned int> departures(2, 0);
    std::vector<unsigned int> arrivals(2, 0);
    unsigned long steps = 0;
    while (!simulation.isFinished() && steps++ < 2000)
    {
        simulation.update();
        for (const auto& event : simulation.getEvents())
        {
            require(event.time == simulation.getTime(), "Network events must have absolute simulation times");
            const std::size_t index = static_cast<std::size_t>(event.train->getId() - 1);
            if (event.type == TrainEventType::Departure)
            {
                require(event.time >= event.train->getDeparture(), "A train must never leave early");
                require(event.time - event.train->getDeparture() <= Simulation::stepSeconds + 1e-8,
                    "A ready train must depart within one simulation step");
                ++departures[index];
            }
            if (event.type == TrainEventType::Arrival)
                ++arrivals[index];
        }
        if (simulation.getTime() < 20.0)
            require(trains.get(1)->getState() == TrainState::Scheduled,
                "A later train must remain Scheduled while other trains move");
    }
    require(simulation.isFinished() && departures == std::vector<unsigned int>({1, 1})
        && arrivals == std::vector<unsigned int>({1, 1}), "Each service must emit exactly one departure and arrival");
    simulation.update();
    require(simulation.getEvents().empty(), "Network updates must clear previously consumed events");
    trains.clear();
}

static double exhaustiveDuration(Station* current, const Station* goal, const Motion& motion,
    double pause, std::vector<Station*>& visited)
{
    if (current == goal)
        return 0.0;
    visited.push_back(current);
    double best = std::numeric_limits<double>::infinity();
    for (auto* segment : current->getSegments())
    {
        auto* next = segment->getStationA() == current ? segment->getStationB() : segment->getStationA();
        if (std::find(visited.begin(), visited.end(), next) == visited.end())
        {
            const double candidate = motion.estimateTravelTime(*segment) + (next == goal ? 0.0 : pause)
                + exhaustiveDuration(next, goal, motion, pause, visited);
            best = std::min(best, candidate);
        }
    }
    visited.pop_back();
    return best;
}

static void checkPathfinding()
{
    std::mt19937 random(731);
    for (unsigned int sample = 0; sample < 12; ++sample)
    {
        std::array<Station, 6> stations{
            Station("A", 0.0, 0.0), Station("B", 1.0, 0.0), Station("C", 2.0, 0.0),
            Station("D", 3.0, 0.0), Station("E", 4.0, 0.0), Station("F", 5.0, 0.0)
        };
        std::vector<Segment> segments;
        segments.reserve(15);
        for (std::size_t first = 0; first < stations.size(); ++first)
            for (std::size_t second = first + 1; second < stations.size(); ++second)
                if (second == first + 1 || random() % 2 == 0)
                    segments.emplace_back(&stations[first], &stations[second],
                        50.0 + random() % 5000, 10.0 + random() % 60);
        for (auto& segment : segments)
        {
            segment.getStationA()->addSegment(&segment);
            segment.getStationB()->addSegment(&segment);
        }
        for (auto& start : stations)
        {
            for (auto& goal : stations)
            {
                if (&start == &goal)
                    continue;
                const TrainParameters parameters{100.0, 10.0 + random() % 100,
                    50.0 + random() % 1000, 50.0 + random() % 1000, 0.0, 10.0};
                const double pause = static_cast<double>(random() % 4) * 100.0;
                Train train(1, "Oracle", parameters, &start, &goal, 0.0, pause);
                const auto path = Pathfinder::findPath(train);
                double actual = 0.0;
                Station* current = &start;
                std::vector<Station*> visited{current};
                for (const auto* segment : path)
                {
                    require(segment->getStationA() == current || segment->getStationB() == current,
                        "Selected path must remain connected");
                    current = segment->getStationA() == current ? segment->getStationB() : segment->getStationA();
                    require(std::find(visited.begin(), visited.end(), current) == visited.end(),
                        "Positive-cost routes must not contain cycles");
                    visited.push_back(current);
                    actual += train.getMotion().estimateTravelTime(*segment) + (current == &goal ? 0.0 : pause);
                }
                require(current == &goal, "Selected path must reach the requested goal");
                visited.clear();
                const double expected = exhaustiveDuration(&start, &goal, train.getMotion(), pause, visited);
                require(std::abs(actual - expected) <= 1e-9 * std::max(1.0, expected),
                    "Dijkstra must match exhaustive search across alternative routes");
            }
        }
    }
}

int main()
{
    try
    {
        checkEstimates();
        checkLifecycle();
        checkNetworkEvents();
        checkPathfinding();
        std::cout << "Simulation core checks passed\n";
        return 0;
    }
    catch (const std::exception& error)
    {
        std::cerr << error.what() << '\n';
        return 1;
    }
}
