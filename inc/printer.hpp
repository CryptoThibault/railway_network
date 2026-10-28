#pragma once
#include "train_event.hpp"
#include <string>
#include <vector>

class Train;

class Printer
{
public:
    static void printStations();
    static void printSegments();
    static void printTrains();
    static void printEvents(const std::vector<TrainEvent>& events);
    static std::string formatTime(double time);
    static std::string formatTrainState(const Train& train, double time);
    static std::string formatRoute(const Train& train);
};