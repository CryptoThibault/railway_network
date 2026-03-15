#include "Motion.hpp"
#include "Segment.hpp"

void Motion::accelerate(double dt, Segment* currentSegment) {
    double accel = (maxAcceleration*1000 - frictionCoef*weight*1000*9.81) / (weight*1000);
    speed += accel * dt * 12960;
    if (currentSegment && speed > currentSegment->getMaxAllowedSpeed())
        speed = currentSegment->getMaxAllowedSpeed();
}

void Motion::brake(double dt) {
    double decel = (maxBrakeForce*1000 + frictionCoef*weight*1000*9.81) / (weight*1000);
    speed -= decel * dt * 12960;
    if (speed < 0)
        speed = 0;
}

double Motion::computeStoppingDistance() const
{
    double v = speed * 1000.0 / 3600.0; // m/s
    double mass = weight * 1000.0;      // kg
    double brakeForce = maxBrakeForce * 1000.0; // N
    double frictionForce = frictionCoef * mass * 9.81; // N

    double a = (brakeForce + frictionForce) / mass;

    double d = (v * v) / (2 * a); // m
    return d / 1000.0 * 1.2;     // km, with safety factor
}
