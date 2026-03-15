#pragma once

class Segment;

class Motion
{
private:
    double weight;
    double frictionCoef;
    double maxAcceleration;
    double maxBrakeForce;
    double speed;

public:
    Motion(double weight_, double frictionCoef_, double maxAcceleration_, double maxBrakeForce_)
        : weight(weight_), frictionCoef(frictionCoef_), maxAcceleration(maxAcceleration_), maxBrakeForce(maxBrakeForce_), speed(0) {}

    void accelerate(double dt, Segment* currentSegment);
    void brake(double dt);
    double computeStoppingDistance() const;
    void stop() { speed = 0; }

    double getSpeed() const { return speed; }
    void setSpeed(double s) { speed = s; }
    double getWeight() const { return weight; }
    double getFrictionCoef() const { return frictionCoef; }
    double getMaxAcceleration() const { return maxAcceleration; }
    double getMaxBrakeForce() const { return maxBrakeForce; }
};
