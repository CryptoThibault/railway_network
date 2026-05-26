#include "network.hpp"

int main()
{
    logger_init();
    factory_init();
    registry_init();

    printStations();
    printSegments();
    printTrains();

    for (auto& train : *Registry<Train>::instance())
    {
        train.getBoard().currentSegment = Registry<Segment>::instance()->get(0);
        train.transitionTo(TrainState::Waiting);
        train.transitionTo(TrainState::Accelerating);
        for (int i = 0; i < 60; i++) train.update();
        // Logger::instance()->info() << "speed= " << train.getMotion().getSpeed() << " distance= " << train.getMotion().getDistance();
        // train.transitionTo(TrainState::Cruising);
        // for (int i = 0; i < 60; i++) train.update();
        // Logger::instance()->info() << "speed= " << train.getMotion().getSpeed() << " distance= " << train.getMotion().getDistance();
        // train.transitionTo(TrainState::Braking);
        // for (int i = 0; i < 60; i++) train.update();
        // Logger::instance()->info() << "speed= " << train.getMotion().getSpeed() << " distance= " << train.getMotion().getDistance();
    }

    return 0;
}