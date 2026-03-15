#pragma once
#include <iostream>
#include <iomanip>
#include <vector>
#include <sys/stat.h>
#include <errno.h>
#include <cstdio>
#include <cstdlib>
#include <fstream>

class Train;
class Segment;

class Logger {
private:
    double lastPrintTime;
    double printInterval;

    void logTrain(Train* t, double currentTime);
    std::string computeState(Train* t, double distanceLeft, Segment* seg) const;
    std::string buildRailGraph(double segmentLength, double distanceDone) const;

public:
    Logger(double intervalMinutes = 5.0);

    void logTrains(double currentTime);
    static std::string formatTime(double currentTime);
};