#include "network.hpp"

int main()
{
    factory_init();
    registry_init();
    logger_init();

    printStations();
    printSegments();

    return 0;
}