#pragma once
#include "board.hpp"
#include "motion.hpp"
#include <string>
#include <vector>
#include "state_machine.hpp"

enum class TrainState
{
    Scheduled,
    Waiting,
    Accelerating,
    Cruising,
    Braking,
    Completed,
};

class Train
{
public:
    Train(long id, std::string name, TrainParameters parameters, Station* start, Station* end, double departure, double pause);

    void state_machine_init();
    void update(double seconds = 1.0);
    void reset();
    TrainState getState() const;
    void transitionTo(TrainState state);

    long getId() const;
    const std::string& getName() const;
    const Station& getStart() const;
    const Station& getEnd() const;
    double getDeparture() const;
    double getPause() const;
    Board& getBoard();
    const Board& getBoard() const;
    const Motion& getMotion() const;
    
private:
    double _stepSeconds = 1.0;
    TrainState _state = TrainState::Scheduled;
    long _id;
    std::string _name;
    Station* _start;
    Station* _end;
    double _departure;
    double _pause;
    Board _board;
    Motion _motion;
    StateMachine<TrainState> _stateMachine;

};