#include "network.hpp"

void logger_init()
{
    Logger::instantiate();
    Logger::instance()->setLevel(Logger::Level::Debug);
}