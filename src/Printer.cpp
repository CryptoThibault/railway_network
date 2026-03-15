#include "Printer.hpp"
#include <iostream>
#include <iomanip>

void Printer::printStations() {
    if (!ENABLE_PRINT)
        return;

    std::cout << "=== Stations List and their segments ===\n";

    for (const auto& stationPtr : Registry<Station>::getInstance().getList()) {
        Station* s = stationPtr.get();
        std::cout << "- " << s->getName() << ":\n";

        bool hasSegment = false;
        for (const auto& segPtr : Registry<Segment>::getInstance().getList()) {
            Segment* seg = segPtr.get();
            if (seg->getStationA() == s || seg->getStationB() == s) {
                hasSegment = true;
                Station* other = seg->getOtherStation(s);
                std::cout << "    * " << s->getName() << " <-> " 
                        << other->getName() 
                        << " | Length: " << seg->getLength() << " km"
                        << " | MaxSpeed: " << seg->getMaxAllowedSpeed() << " km/h"
                        << "\n";
            }
        }
        std::cout << "\n";
    }
    std::cout << "=====================================\n";
}

void Printer::printTrains() {
    if (!ENABLE_PRINT)
        return;

    std::cout << "=== Trains List ===\n";

    for (const auto& trainPtr : Registry<Train>::getInstance().getList()) {
        Train* t = trainPtr.get();
        std::cout << t->getName()
                << " | " << t->getId()
                << " | " << t->getDeparture()->getName()
                << " -> " << t->getArrival()->getName()
                << "\n";
    }
    std::cout << "========================\n";
}

#include <iostream>
#include <string>

#include <iostream>
#include <string>

void Printer::printHelp() {
    std::cout << "=== Train Simulator Help ===\n\n";

    std::cout << "Rails file format (first argument):\n\n";
    std::cout << "Node <StationName>\n";
    std::cout << "Rail <StationA> <StationB> <LengthKm> <MaxSpeedKmH>\n\n";

    std::cout << "Trains file format (second argument):\n\n";
    std::cout << "<Name> <Weight> <FrictionCoef> <MaxAcceleration> <MaxBrakeForce>\n";
    std::cout << "<DepartureStation> <ArrivalStation> <HourOfDeparture> <StopDuration>\n\n";

    std::cout << "Description of the fields:\n";
    std::cout << "  Segment :\n";
    std::cout << "    <StationA>     : Name of the first station\n";
    std::cout << "    <StationB>     : Name of the second station\n";
    std::cout << "    <LengthKm>     : Length of the segment in kilometers\n";
    std::cout << "    <MaxSpeedKmH>  : Maximum speed allowed on the segment (km/h)\n\n";

    std::cout << "  Train :\n";
    std::cout << "    <Name>            : Train name\n";
    std::cout << "    <Weight>          : Weight of the train in tons\n";
    std::cout << "    <FrictionCoef>    : Friction coefficient\n";
    std::cout << "    <MaxAcceleration> : Maximum acceleration\n";
    std::cout << "    <MaxBrakeForce>   : Maximum braking force\n";
    std::cout << "    <DepartureStation>: Name of the departure station\n";
    std::cout << "    <ArrivalStation>  : Name of the arrival station\n";
    std::cout << "    <HourOfDeparture> : Departure time in HHhMM format\n";
    std::cout << "    <StopDuration>    : Duration of stop at stations in HHhMM format\n\n";

    std::cout << "Usage:\n";
    std::cout << "  ./network <rails_file.txt> <trains_file.txt>\n";
    std::cout << "  ./network -help   : Show this help message\n";
    std::cout << "============================\n";
}