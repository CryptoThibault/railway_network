#pragma once
#include "train_type.hpp"

class Train;

class Motion
{
public:
    Motion(Train* train);

    void move();
    void accelerate();
    void brake();

    Train* getTrain() const;
    double getSpeed() const;
    double getDistance() const;

private:
    Train* _train;
    double _speed{};
    double _distance{};
};