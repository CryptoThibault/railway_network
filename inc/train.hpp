#pragma once
#include "network.hpp"
#include "train_type.hpp"
#include "board.hpp"
#include "motion.hpp"

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
    Train(long id, const TrainType* type, Station* initialStation);

    long getId() const;
    const TrainType* getType() const;
    Board getBoard() const;
    Motion getMotion() const;

private:
    long _id;
    const TrainType* _type;

    Board _board;
    Motion _motion;
    StateMachine<TrainState> _stateMachine;

    void state_machine_init(); 
};