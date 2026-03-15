#include "Loader.hpp"
#include "Printer.hpp"
#include "Simulation.hpp"
#include "Traveler.hpp"
#include "Printer.hpp"
#include <iostream>

int main(int argc, char** argv)
{
    if (argc < 3) {
        std::cout << "Usage: " << argv[0] << " <rails_file.txt> <trains_file.txt>\n";
        return 1;
    }
    if (argv[1] == std::string("-h") || argv[1] == std::string("--help")) {
        Printer::printHelp();
        return 0;
    }

    Loader loader;
    try {
        loader.loadRails(argv[1]);
        loader.loadTrains(argv[2]);
    } catch (const std::exception& ex) {
        std::cerr << "Error: " << ex.what() << "\n";
        return 1;
    }

    Printer::printStations();
    Printer::printTrains();

    Simulation sim;

    Simulation::addTraveler("Paris", "Zurich");
    Simulation::addTraveler("Rome", "Copenhagen");
    Simulation::addTraveler("Marseille", "Berlin");
    //Simulation::addTraveler("CityA", "CityB");

    sim.run(24.0);

    return 0;
}