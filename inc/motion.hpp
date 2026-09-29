#pragma once
#include "train_type.hpp"

struct TrainType;
struct Board;

class Motion
{
public:
    Motion(TrainType type);

    void move(double seconds = 1.0);
    void accelerate(const Board& board, double seconds = 1.0);
    void brake(double seconds = 1.0);

    const TrainType& getType() const;
    double getSpeed() const;
    double getDistance() const;

private:
    TrainType _type;
    double _speed{};
    double _distance{};
};