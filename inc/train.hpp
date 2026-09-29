#pragma once
#include "network.hpp"
#include "board.hpp"
#include "motion.hpp"

struct TrainType;

enum class TrainState
{
    Idle,
    Waiting,
    Accelerating,
    Cruising,
    Braking,
};

class Train
{
public:
    Train(long id, TrainType type, Station* initialStation);

    void state_machine_init();
    void update(double seconds = 1.0);
    TrainState getState() const;
    void transitionTo(TrainState state);

    long getId() const;
    Board& getBoard();
    const Board& getBoard() const;
    const Motion& getMotion() const;
    
private:
    double _stepSeconds = 1.0;
    TrainState _state = TrainState::Idle;
    long _id;
    Board _board;
    Motion _motion;
    StateMachine<TrainState> _stateMachine;

};