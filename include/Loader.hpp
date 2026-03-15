#pragma once

#include <string>

class Loader
{
private:
    long nextTrainId = 1;

    static double parseTime(const std::string& timeStr);
    void loadSegment(const std::string& nameA, const std::string& nameB, double length, double maxSpeed);
    void loadTrain(const std::string& name, double weight, double frictionCoef, double maxAcceleration, double maxBrakeForce,
            const std::string& departureName, const std::string& arrivalName, double departureHour, double stopDuration);

public:
    void loadRails(const std::string& filename);
    void loadTrains(const std::string& filename);
};

