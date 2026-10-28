#include "printer.hpp"
#include "train.hpp"
#include "station.hpp"
#include "segment.hpp"
#include "logger.hpp"
#include <algorithm>
#include "registry.hpp"
#include <iomanip>
#include <sstream>

void Printer::printStations()
{
    Logger::instance()->debug() << std::setprecision(12) << "Stations (" << Registry<Station>::instance()->size() << ")";
    for (const auto& station : *Registry<Station>::instance())
        Logger::instance()->debug() << "  " << station.getName()
            << " | x: " << station.getPosition().x
            << " | y: " << station.getPosition().y;
}

void Printer::printSegments()
{
    Logger::instance()->debug() << "Segments (" << Registry<Segment>::instance()->size() << ")";
    for (const auto& segment : *Registry<Segment>::instance())
        Logger::instance()->debug() << "  " << segment.getStationA()->getName()
            << " - " << segment.getStationB()->getName()
            << " | length: " << segment.getLength() << " m"
            << " | maxSpeed: " << segment.getMaxSpeed() << " m/s";
}

void Printer::printTrains()
{
    Logger::instance()->debug() << "Trains (" << Registry<Train>::instance()->size() << ")";
    for (const auto& train : *Registry<Train>::instance())
    {
        const auto& type = train.getMotion().getParameters();
        const long departure = static_cast<long>(train.getDeparture());
        Logger::instance()->debug() << "  " << train.getId() << " | " << train.getName()
            << " | " << train.getStart().getName() << " -> " << train.getEnd().getName()
            << " | departure: " << std::setfill('0') << std::setw(2) << departure / 3600
            << ':' << std::setw(2) << departure / 60 % 60
            << ':' << std::setw(2) << departure % 60 << std::setfill(' ')
            << " | pause: " << train.getPause() << " s per intermediate station";
        Logger::instance()->debug() << "    mass: " << type.mass << " kg | maxSpeed: " << type.maxSpeed
            << " m/s | engineForce: " << type.engineForce << " N | brakeForce: "
            << type.brakeForce << " N | friction: " << type.friction
            << " | length: " << type.length << " m";
    }
}

std::string Printer::formatRoute(const Train& train)
{
    std::ostringstream output;
    const Station* station = &train.getStart();
    output << station->getName();
    for (const auto* segment : train.getBoard().path)
    {
        station = segment->getStationA() == station
            ? segment->getStationB() : segment->getStationA();
        output << " -> " << station->getName();
    }
    return output.str();
}

std::string Printer::formatTime(double time)
{
    const auto seconds = static_cast<unsigned long long>(time + 1e-6);
    std::ostringstream output;
    output << std::setfill('0') << std::setw(2) << seconds / 3600 % 24
        << ':' << std::setw(2) << seconds / 60 % 60
        << ':' << std::setw(2) << seconds % 60
        << " | Day " << seconds / 86400 + 1;
    return output.str();
}

static const char* trainStateName(TrainState state)
{
    switch (state)
    {
        case TrainState::Scheduled: return "Scheduled";
        case TrainState::Completed: return "Completed";
        case TrainState::Waiting: return "Waiting";
        case TrainState::Accelerating: return "Accelerating";
        case TrainState::Cruising: return "Cruising";
        case TrainState::Braking: return "Braking";
    }
    return "Unknown";
}

std::string Printer::formatTrainState(const Train& train, double time)
{
    const auto& board = train.getBoard();
    std::ostringstream output;
    output << std::fixed << std::setprecision(2) << formatTime(time)
        << " | Train " << train.getId() << " (" << train.getName() << ")"
        << " | State: " << trainStateName(train.getState())
        << " | Speed: " << train.getMotion().getSpeed() << " m/s"
        << " | Distance: " << train.getMotion().getDistance() << " m";
    if (board.currentStation)
        output << " | Station: " << board.currentStation->getName();
    else
    {
        const Station* departure = &train.getStart();
        for (std::size_t index = 0; index < board.pathIndex; ++index)
        {
            const auto* segment = board.path[index];
            departure = segment->getStationA() == departure
                ? segment->getStationB() : segment->getStationA();
        }
        const Station* destination = board.currentSegment->getStationA() == departure
            ? board.currentSegment->getStationB() : board.currentSegment->getStationA();
        output << " | Segment: " << departure->getName() << " -> " << destination->getName()
            << " | Position: " << board.segmentProgress << " / "
            << board.currentSegment->getLength() << " m";
    }
    return output.str();
}

void Printer::printEvents(const std::vector<TrainEvent>& events)
{
    for (const auto& event : events)
    {
        auto line = Logger::instance()->info();
        line << formatTime(event.time) << " | Train " << event.train->getId()
            << " (" << event.train->getName() << ") | ";
        switch (event.type)
        {
            case TrainEventType::Departure:
                line << "Departed " << event.station->getName();
                if (event.firstDeparture)
                    line << " | Route: " << formatRoute(*event.train);
                break;
            case TrainEventType::StationStop:
                line << "Stopped at " << event.station->getName()
                    << " | pause: " << event.train->getPause() << " s";
                break;
            case TrainEventType::Arrival:
                line << "Completed at " << event.station->getName();
                break;
        }
    }
}
