#include "timer.hpp"
#include <iostream>
#include <thread>

int main() {
    Timer t;
    t.start(Timer::Duration(2000)); // start timer for 2 seconds
    std::cout << "Timer started for 2 seconds\n";

    // Loop until the timer expires
    while (!t.isTimeout()) {
        auto rem = t.remaining().count(); // remaining time in ms
        std::cout << "Remaining: " << rem << " ms\n";
        std::this_thread::sleep_for(std::chrono::milliseconds(500));
    }

    std::cout << "Timer expired!\n";

    // Restart the timer
    t.reset();
    std::cout << "Timer reset\n";
    std::this_thread::sleep_for(std::chrono::milliseconds(1000));
    std::cout << "After 1 second, remaining: " << t.remaining().count() << " ms\n";
    std::cout << "Is timeout? " << (t.isTimeout() ? "Yes" : "No") << "\n";

    return 0;
}