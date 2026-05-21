#include "network.hpp"

int main()
{
    logger_init();
    factory_init();
    registry_init();

    printStations();
    printSegments();
    printTrains();

    return 0;
}