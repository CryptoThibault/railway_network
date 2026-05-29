#include "network.hpp"

int main()
{
    Initializer::initialize();

    Printer::printStations();
    Printer::printSegments();
    Printer::printTrains();

    for (auto& train : *Registry<Train>::instance())
    {
        Logger::instance()->info() << "Train " << train.getId();
        train.getBoard().currentSegment = Registry<Segment>::instance()->get(0);
        train.transitionTo(TrainState::Waiting);
        train.transitionTo(TrainState::Accelerating);
        for (int i = 0; i < 60; i++) train.update();
        Logger::instance()->info() << "speed= " << train.getMotion().getSpeed() << " distance= " << train.getMotion().getDistance();
        train.transitionTo(TrainState::Cruising);
        for (int i = 0; i < 60; i++) train.update();
        Logger::instance()->info() << "speed= " << train.getMotion().getSpeed() << " distance= " << train.getMotion().getDistance();
        train.transitionTo(TrainState::Braking);
        for (int i = 0; i < 60; i++) train.update();
        Logger::instance()->info() << "speed= " << train.getMotion().getSpeed() << " distance= " << train.getMotion().getDistance();
    }

    return 0;
}