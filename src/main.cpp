#include "network.hpp"

int main()
{
    Logger::instantiate();
    Logger::instance()->info() << "Hello World!";
    return 0;
}