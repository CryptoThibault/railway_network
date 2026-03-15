#pragma once
#include <vector>
#include <string>
#include "Station.hpp"
#include "Segment.hpp"
#include "Train.hpp"
#include "Registry.hpp"


class Printer {
private:
    static const bool ENABLE_PRINT = false;
public:
    static void printStations();
    static void printTrains();
    static void printHelp();
};