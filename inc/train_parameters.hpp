#pragma once

struct TrainParameters
{
    double mass;
    double maxSpeed;
    double engineForce;
    double brakeForce;
    double friction;
    double length;

    void validate() const;
};
