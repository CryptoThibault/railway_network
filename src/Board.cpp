#include "Board.hpp"
#include "Train.hpp"
#include "Network.hpp"

void Board::updatePosition(double dt, double speed) {
    if (stopTimeRemaining > 0) {
        stopTimeRemaining -= dt;
        if (stopTimeRemaining <= 0)
            nextStationReached();
        return;
    }

    distanceOnSegment += speed * dt;
    if (distanceOnSegment >= currentSegment->getLength()) {
        distanceOnSegment = 0;
        nextStationReached();
    }
}

void Board::nextStationReached() {
    train->motion.stop();

    if (getNextStation() == train->getArrival()) {
        train->state = TrainState::Arrived;
        train->motion.setSpeed(0);
        currentSegment = nullptr;
        nextStation = nullptr;
        return;
    }

    stopTimeRemaining = stopDuration;
    train->state = TrainState::Waiting;
}