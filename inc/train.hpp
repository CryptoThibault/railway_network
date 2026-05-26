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

    void update();
    void transitionTo(TrainState state);

    long getId() const;
    Board& getBoard();
    const Board& getBoard() const;
    const Motion& getMotion() const;

private:
    long _id;
    Board _board;
    Motion _motion;
    StateMachine<TrainState> _stateMachine;

    void state_machine_init(); 
};