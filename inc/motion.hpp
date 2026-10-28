#pragma once
#include "train_parameters.hpp"

class Segment;
struct Board;

class Motion
{
public:
    explicit Motion(TrainParameters parameters);

    void move(double seconds = 1.0);
    void accelerate(const Board& board, double seconds = 1.0);
    void brake(double seconds = 1.0);

    const TrainParameters& getParameters() const;
    double estimateTravelTime(const Segment& segment) const;
    double getSpeed() const;
    double getDistance() const;

private:
    TrainParameters _parameters;
    double _speed{};
    double _distance{};
};