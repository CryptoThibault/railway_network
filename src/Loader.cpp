#include "Loader.hpp"
#include "Station.hpp"
#include "Segment.hpp"
#include "Registry.hpp"
#include "Train.hpp"
#include <fstream>
#include <sstream>
#include <iostream>
#include <stdexcept>

double Loader::parseTime(const std::string& timeStr) {
    size_t hPos = timeStr.find('h');
    if (hPos == std::string::npos) {
        throw std::runtime_error("Invalid time format: " + timeStr);
    }
    int hours = std::stoi(timeStr.substr(0, hPos));
    int minutes = std::stoi(timeStr.substr(hPos + 1));
    return hours + minutes / 60.0;
}

void Loader::loadRails(const std::string& filename)
{
    std::ifstream file(filename);
    if (!file) {
        throw std::runtime_error("Cannot open file: " + filename);
    }

    std::string line;
    int lineNumber = 0;
    while (std::getline(file, line))
    {
        lineNumber++;
        if (line.empty() || line[0] == '#') continue;

        std::istringstream iss(line);
        std::string type;
        if (!(iss >> type)) {
            std::cerr << "Parsing error at line " << lineNumber << ": empty or invalid line\n";
            std::exit(1);
        }

        if (type == "Node")
        {
            std::string name;
            if (!(iss >> name)) {
                throw std::runtime_error("Parsing error at line " + std::to_string(lineNumber) 
                                         + ": Node requires 1 field (name)");
            }

            if (!Registry<Station>::getInstance().findByName(name)) {
                auto s = std::make_unique<Station>(name);
                Registry<Station>::getInstance().add(std::move(s));
            }
        }
        else if (type == "Rail")
        {
            std::string nameA, nameB;
            double length, maxSpeed;
            if (!(iss >> nameA >> nameB >> length >> maxSpeed)) {
                throw std::runtime_error("Parsing error at line " + std::to_string(lineNumber) 
                                         + ": Rail requires 4 fields (nameA, nameB, length, maxSpeed)");
            }

            loadSegment(nameA, nameB, length, maxSpeed);
        }
        else
        {
            throw std::runtime_error("Parsing error at line " + std::to_string(lineNumber) 
                                     + ": unknown type \"" + type + "\"");
        }
    }
}

void Loader::loadTrains(const std::string& filename)
{
    std::ifstream file(filename);
    if (!file) {
        throw std::runtime_error("Cannot open file: " + filename);
    }

    std::string line;
    int lineNumber = 0;
    while (std::getline(file, line))
    {
        lineNumber++;
        if (line.empty() || line[0] == '#') continue;

        std::istringstream iss(line);
        std::string name, depName, arrName, depHourStr, stopDurStr;
        double weight, friction, maxAcceleration, maxBrakeForce;

        if (!(iss >> name >> weight >> friction >> maxAcceleration >> maxBrakeForce
              >> depName >> arrName >> depHourStr >> stopDurStr)) {
            throw std::runtime_error("Parsing error at line " + std::to_string(lineNumber));
        }

        if (name.rfind("Train", 0) != 0) {
            throw std::runtime_error("Parsing error at line " + std::to_string(lineNumber) 
                                     + ": train name must start with 'Train'");
        }

        double depHour = parseTime(depHourStr);
        double stopDur = parseTime(stopDurStr);

        loadTrain(name, weight, friction, maxAcceleration, maxBrakeForce, depName, arrName, depHour, stopDur);
    }
}

void Loader::loadSegment(const std::string& nameA, const std::string& nameB, double length, double maxSpeed)
{
    Station* a = Registry<Station>::getInstance().findByName(nameA);
    if (!a) {
        auto s = std::make_unique<Station>(nameA);
        a = s.get();
        Registry<Station>::getInstance().add(std::move(s));
    }

    Station* b = Registry<Station>::getInstance().findByName(nameB);
    if (!b) {
        auto s = std::make_unique<Station>(nameB);
        b = s.get();
        Registry<Station>::getInstance().add(std::move(s));
    }

    auto segment = std::make_unique<Segment>(a, b, length, maxSpeed);
    Registry<Segment>::getInstance().add(std::move(segment));
}

void Loader::loadTrain(const std::string& name, double weight, double frictionCoef, double maxAcceleration, double maxBrakeForce,
            const std::string& departureName, const std::string& arrivalName, double departureHour, double stopDuration)
{
    Station* departure = Registry<Station>::getInstance().findByName(departureName);
    Station* arrival = Registry<Station>::getInstance().findByName(arrivalName);

    if (!departure || !arrival) {
        throw std::runtime_error("Train refers to unknown station(s) "
                                 + departureName + " or " + arrivalName);
    }

    auto train = std::make_unique<Train>(
        nextTrainId++,
        name,
        weight,
        frictionCoef,
        maxAcceleration,
        maxBrakeForce,
        departure,
        arrival,
        departureHour,
        stopDuration
    );

    Registry<Train>::getInstance().add(std::move(train));
}