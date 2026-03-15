#include "Loader.hpp"
#include "Printer.hpp"
#include "Simulation.hpp"
#include "Traveler.hpp"
#include "Printer.hpp"

int main(int argc, char** argv)
{
    if (argc < 2) {
        std::cout << "Usage: " << argv[0] << " <input_file.txt>\n";
        return 1;
    }
    if (argv[1] == std::string("-h") || argv[1] == std::string("--help")) {
        Printer::printHelp();
        return 0; 
    }
    
    Loader loader;
    loader.loadFile(argv[1]);
    
    Station* stationA = Registry<Station>::getInstance().findByName("Paris");
    Station* stationB = Registry<Station>::getInstance().findByName("Zurich");
    Registry<Traveler>::getInstance().add(std::make_unique<Traveler>(stationA, stationB));

    Station* stationC = Registry<Station>::getInstance().findByName("Rome");
    Station* stationD = Registry<Station>::getInstance().findByName("Copenhagen");
    Registry<Traveler>::getInstance().add(std::make_unique<Traveler>(stationC, stationD));

    Station* stationE = Registry<Station>::getInstance().findByName("Marseille");
    Station* stationF = Registry<Station>::getInstance().findByName("Berlin");
    Registry<Traveler>::getInstance().add(std::make_unique<Traveler>(stationE, stationF));

    Printer::printStations();
    Printer::printTrains();

    Simulation sim;
    sim.run(24.0);

    return 0;
}