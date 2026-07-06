#include "network.hpp"

int main()
{
    Initializer::initialize();

    Printer::printStations();
    Printer::printSegments();
    Printer::printTrains();

    auto& train = *Registry<Train>::instance()->get(0);
    train.getBoard().currentSegment = Registry<Segment>::instance()->get(0);
    train.transitionTo(TrainState::Waiting);
    train.transitionTo(TrainState::Accelerating);

    for (int i = 0; i < 600; i++)
    {

        train.update();
        Logger::instance()->info() << i << "s. "
            << "Train " << train.getId()
            << " | distance: " << train.getMotion().getDistance()
            << " | speed: " << train.getMotion().getSpeed();

        std::this_thread::sleep_for(std::chrono::milliseconds(10));
    }

    return 0;
}