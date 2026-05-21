#pragma once
#include "train_type.hpp"

class MotionSystem
{
public:
    void accelerate(double& speed, const TrainType& type, double dt);
    void brake(double& speed, const TrainType& type, double dt);
    void cruise(double& speed, const TrainType& type, double dt);
};