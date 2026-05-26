#pragma once
#include "train_type.hpp"

struct TrainType;
struct Board;

class Motion
{
public:
    Motion(TrainType type);

    void move();
    void accelerate(const Board& board);
    void brake();

    const TrainType& getType() const;
    double getSpeed() const;
    double getDistance() const;

private:
    TrainType _type;
    double _speed{};
    double _distance{};
};