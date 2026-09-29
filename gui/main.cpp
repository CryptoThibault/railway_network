#include "window.hpp"
#include <exception>
#include <iostream>

int main()
{
    try
    {
        ApplicationWindow window;
        window.run();
        return 0;
    }
    catch (const std::exception& error)
    {
        std::cerr << "network_window: " << error.what() << '\n';
        return 1;
    }
}
