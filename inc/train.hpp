#pragma once
#include "network.hpp"
#include "board.hpp"
#include "motion_system.hpp"

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
    double getSpeed() const;
    double getDistance() const;
    Board getBoard() const;
    MotionSystem getMotionSystem() const;

private:
    long _id;
    const TrainType* _type;

    double _speed{};
    double _distance{};

    Board _board;
    MotionSystem _motionSystem;
    StateMachine<TrainState> _stateMachine;
};