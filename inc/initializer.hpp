#pragma once
#include "field.hpp"

class Initializer
{
public:
    static void initialize(const std::string& path = "data.json");

private:
    static void initFactory();
    static void initStations(const FieldVector& data);
    static void initSegments(const FieldVector& data);
    static void initTrains(const FieldVector& data);
};
