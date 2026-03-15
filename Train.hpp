#pragma once
#include <string>
#include "Station.hpp"
#include "Segment.hpp"
#include "Motion.hpp"
#include "Board.hpp"

enum class TrainState { Loading, Waiting, Running, Stopped, Arrived };

class Train
{
private:
    long id;
    std::string name;

    TrainState state;
    Station* departure;
    Station* arrival;
    double departureHour;

    Board board;
    Motion motion;

    friend class Board;
public:
    Train(long id_,
          const std::string& name_,
          double weight_,
          double frictionCoef_,
          double maxAcceleration_,
          double maxBrakeForce_,
          Station* departure_,
          Station* arrival_,
          double departureHour_,
          double stopDuration_)
        : id(id_),
            name(name_),
            state(TrainState::Loading),
            departure(departure_),
            arrival(arrival_),
            departureHour(departureHour_),
            board(stopDuration_, this),
            motion(weight_, frictionCoef_, maxAcceleration_, maxBrakeForce_)
    {}

    void startTrain() { state = TrainState::Running; }
    void stopTrain() { state = TrainState::Stopped; motion.stop(); }
    void advance(double dt);
    void nextStationReached() { board.nextStationReached(); }
    void accelerate(double dt) { motion.accelerate(dt, board.getCurrentSegment()); }
    void brake(double dt) { motion.brake(dt); }
    double computeStoppingDistance() const { return motion.computeStoppingDistance(); }

    Board& getBoard() { return board; }
    const Board& getBoard() const { return board; }
    Motion& getMotion() { return motion; }
    const Motion& getMotion() const { return motion; }

    long getId() const { return id; }
    const std::string& getName() const { return name; }
    TrainState getState() const { return state; }
    Station* getDeparture() const { return departure; }
    Station* getArrival() const { return arrival; }
    int getDepartureHour() const { return departureHour; }

    Station* getNextStationFrom(Station* station);
    double getSegmentTimeFrom(Station* station);
    double getNextDeparture(Station*) const { return departureHour; }
};