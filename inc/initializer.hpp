#pragma once
#include "network.hpp"

class Initializer
{
public:
    static void initialize();

private:
    static void initFactory();
    static void initStations(const FieldVector& data);
    static void initSegments(const FieldVector& data);
    static void initTrainTypes(const FieldVector& data);
    static void initTrains(const FieldVector& data);
    static void initJourneys(const FieldVector& data);
};
