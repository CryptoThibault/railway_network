#include "Simulation.hpp"
#include "Train.hpp"
#include "Registry.hpp"
#include "Network.hpp"
#include "Traveler.hpp"
#include "Logger.hpp"
#include "EventSystem.hpp"
#include <iostream>

void Simulation::addTraveler(const std::string& from, const std::string& to)
{
    Station* stationFrom = Registry<Station>::getInstance().findByName(from);
    Station* stationTo   = Registry<Station>::getInstance().findByName(to);

    if (!stationFrom || !stationTo) {
        std::cerr << "Warning: cannot create traveler " << from << " -> " << to
                  << " (station missing)\n";
        return;
    }

    Registry<Traveler>::getInstance().add(std::make_unique<Traveler>(stationFrom, stationTo));
}

void Simulation::run(double endTime) {
    std::srand(static_cast<unsigned int>(std::time(nullptr)));
    EventSystem eventSystem;
    eventSystem.subscribe(&Network::getInstance());

    while (currentTime < endTime) {
        tick();
        if (currentTime >= nextTravelerCheck && currentTime <= 16.0)
        {
            for (const auto& travelerPtr : Registry<Traveler>::getInstance().getList())
                travelerPtr->computeFastestTravelTime(currentTime);
            nextTravelerCheck += 1.0;
        }
        if (rand() % 5000 == 0)
        {
            Event e = static_cast<Event>(rand() % 4);
            eventSystem.trigger(e);
        }
        logger.logTrains(currentTime);
        currentTime += timeStep;
    }

    for (const auto& travelerPtr : Registry<Traveler>::getInstance().getList())
    {
        std::cout
            << travelerPtr->getStart()->getName()
            << " -> "
            << travelerPtr->getEnd()->getName()
            << " average travel time: "
            << Logger::formatTime(travelerPtr->getAverageTravelTime()) << std::endl;
    }
}

void Simulation::tick() {
    for (const auto& t : Registry<Train>::getInstance().getList()) {
        if (t->getState() == TrainState::Loading && currentTime >= t->getDepartureHour())
        {
            Network::getInstance().planNextSegment(t.get());
            t->startTrain();
        }

        if (t->getState() == TrainState::Running || t->getState() == TrainState::Waiting)
            t->advance(timeStep);
    }
}