#include "chronometer.hpp"
#include <iostream>
#include <thread>

int main() {
    Chronometer chrono;

    // Start the chronometer
    chrono.start();
    std::cout << "Chronometer started\n";

    // Let it run for 500 ms
    std::this_thread::sleep_for(std::chrono::milliseconds(500));
    std::cout << "Elapsed after 0.5s: " << chrono.elapsed().count() << " ms\n";

    // Stop the chronometer
    chrono.stop();
    std::cout << "Chronometer stopped\n";

    // Wait 300 ms while stopped (should not count)
    std::this_thread::sleep_for(std::chrono::milliseconds(300));
    std::cout << "Elapsed while stopped: " << chrono.elapsed().count() << " ms\n";

    // Resume
    chrono.start();
    std::cout << "Chronometer resumed\n";
    std::this_thread::sleep_for(std::chrono::milliseconds(200));
    std::cout << "Elapsed after resuming 0.2s: " << chrono.elapsed().count() << " ms\n";

    // Reset
    chrono.reset();
    std::cout << "Chronometer reset\n";
    std::cout << "Elapsed after reset: " << chrono.elapsed().count() << " ms\n";

    return 0;
}