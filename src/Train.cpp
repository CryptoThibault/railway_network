#include "Train.hpp"
#include "Network.hpp"

void Train::advance(double dt) {
    if (state == TrainState::Waiting) {
        board.decreaseStopTime(dt);

        if (board.getStopTimeRemaining() <= 0) {
            state = TrainState::Running;
            Segment* nextSeg = Network::getInstance().planNextSegment(this);
            if (!nextSeg) {
                state = TrainState::Arrived;
                motion.setSpeed(0);
                return;
            }
            board.setCurrentSegment(nextSeg);
        }
        return;
    }

    Segment* currSeg = board.getCurrentSegment();
    if (!currSeg) return;

    double distanceLeft = currSeg->getLength() - board.getDistanceOnSegment();

    if (distanceLeft <= motion.computeStoppingDistance())
        motion.brake(dt);
    else
        motion.accelerate(dt, board.getCurrentSegment());

    board.updatePosition(dt, motion.getSpeed());

    if (board.getDistanceOnSegment() >= currSeg->getLength())
        board.nextStationReached();
}

Station* Train::getNextStationFrom(Station* station)
{
    Network& network = Network::getInstance();

    const auto& path = network.getPath(this);

    for (Segment* seg : path) {

        if (seg->getStationA() == station)
            return seg->getStationB();

        if (seg->getStationB() == station)
            return seg->getStationA();
    }

    return nullptr;
}

double Train::getSegmentTimeFrom(Station* station)
{
    Network& network = Network::getInstance();
    const auto& path = network.getPath(this);

    for (Segment* seg : path) {

        if (seg->getStationA() == station || seg->getStationB() == station) {

            double distance = seg->getLength();
            double speed = seg->getMaxAllowedSpeed();

            return distance / speed;
        }
    }

    return -1;
}