#include "simulation_controller.hpp"
#include "initializer.hpp"
#include "segment.hpp"
#include "station.hpp"
#include <algorithm>
#include <cmath>
#include <stdexcept>
#include <utility>

SimulationController::SimulationController(const std::filesystem::path& scenario, const std::filesystem::path& directory)
{
    Initializer::initialize(scenario.string());
    auto& registry = *Registry<Train>::instance();
    simulation = std::make_unique<NetworkSimulation>(registry);
    logs = std::make_unique<TrainLog>(registry, directory);
    refresh();
}

void SimulationController::togglePlayback()
{
    if (simulation->isFinished())
        return;
    running = !running;
    started = true;
    pending = 0.0;
}

void SimulationController::setTimeScale(int scale)
{
    if (scale < 1 || scale > 6000)
        throw std::invalid_argument("Playback speed must be between 1 and 6000");
    timeScale = scale;
}

bool SimulationController::advance(double realSeconds)
{
    if (!std::isfinite(realSeconds) || realSeconds < 0.0)
        throw std::invalid_argument("Frame duration must be finite and nonnegative");
    if (!running)
        return false;
    const double accumulated = pending + realSeconds * timeScale;
    if (!std::isfinite(accumulated))
        throw std::invalid_argument("Frame duration exceeds the supported range");
    pending = accumulated;
    bool changed = false;
    for (int batch = 0; batch < 10000 && pending + 1e-9 >= Simulation::stepSeconds; ++batch)
    {
        simulation->update();
        pending = std::max(0.0, pending - Simulation::stepSeconds);
        for (const auto& event : simulation->getEvents())
        {
            if (event.type == TrainEventType::Departure && event.firstDeparture)
                departures[event.train->getId()] = event.time;
            if (event.type == TrainEventType::StationStop)
                stops[event.train->getId()] = event.time;
        }
        logs->update(simulation->getTime(), simulation->getEvents());
        changed = true;
        if (simulation->isFinished())
        {
            running = false;
            break;
        }
    }
    if (changed)
        refresh();
    return changed;
}

void SimulationController::refresh()
{
    trains.clear();
    upcoming.reset();
    const double time = simulation->getTime();
    for (const auto& train : *Registry<Train>::instance())
    {
        const auto state = train.getState();
        if (state == TrainState::Scheduled || state == TrainState::Completed)
        {
            double departure = std::floor(time / 86400.0) * 86400.0 + train.getDeparture();
            if (departure < time - 1e-8 || (state == TrainState::Completed && departure <= time + 1e-8))
                departure += 86400.0;
            if (!upcoming || departure < upcoming->time)
                upcoming = UpcomingService{train.getName(), train.getStart().getName(), train.getEnd().getName(), departure};
            continue;
        }
        const auto& board = train.getBoard();
        TrainDashboard value{};
        value.id = train.getId();
        value.name = train.getName();
        value.state = state;
        value.parameters = train.getMotion().getParameters();
        value.origin = train.getStart().getName();
        value.destination = train.getEnd().getName();
        value.station = board.currentStation ? board.currentStation->getName() : "";
        value.speed = train.getMotion().getSpeed();
        value.departure = train.getDeparture();
        value.started = departures.at(train.getId());
        value.elapsed = time - value.started;
        value.segmentPosition = board.segmentProgress;
        value.segmentLength = board.currentSegment->getLength();
        value.segmentLimit = std::min(value.parameters.maxSpeed, board.currentSegment->getMaxSpeed());
        const Station* station = &train.getStart();
        value.stations.push_back(station->getName());
        value.stationDistances.push_back(0.0);
        double distance = 0.0;
        for (std::size_t index = 0; index < board.path.size(); ++index)
        {
            const auto* segment = board.path[index];
            const Station* next = segment->getStationA() == station
                ? segment->getStationB() : segment->getStationA();
            if (index == board.pathIndex)
            {
                value.legOrigin = station->getName();
                value.legDestination = next->getName();
                value.routePosition = distance + board.segmentProgress;
            }
            distance += segment->getLength();
            value.stations.push_back(next->getName());
            value.stationDistances.push_back(distance);
            station = next;
        }
        value.routeLength = distance;
        if (state == TrainState::Waiting)
            value.pauseRemaining = std::max(0.0, train.getPause() - (time - stops.at(train.getId())));
        trains.push_back(std::move(value));
    }
    std::sort(trains.begin(), trains.end(), [](const TrainDashboard& first, const TrainDashboard& second)
    {
        return first.started != second.started ? first.started < second.started : first.id < second.id;
    });
}

bool SimulationController::isRunning() const { return running; }
bool SimulationController::hasStarted() const { return started; }
int SimulationController::getTimeScale() const { return timeScale; }
double SimulationController::getTime() const { return simulation->getTime(); }
std::size_t SimulationController::getServiceCount() const { return Registry<Train>::instance()->size(); }
const std::vector<TrainDashboard>& SimulationController::getTrains() const { return trains; }
const std::optional<UpcomingService>& SimulationController::getUpcoming() const { return upcoming; }
