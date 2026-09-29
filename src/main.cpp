#include "network.hpp"
#include "simulation.hpp"
#include <charconv>
#include <chrono>
#include <csignal>
#include <iostream>
#include <stdexcept>
#include <string_view>
#include <thread>
#include <utility>

static volatile std::sig_atomic_t stopRequested = 0;
static volatile std::sig_atomic_t pauseRequested = 0;
static volatile std::sig_atomic_t requestedScale = 60;

static void requestStop(int)
{
    stopRequested = 1;
}

static void requestPause(int signal)
{
    pauseRequested = signal == SIGUSR1;
}

static void requestScale(int, siginfo_t* information, void*)
{
    const int scale = information->si_value.sival_int;
    if (scale >= 60 && scale <= 300)
        requestedScale = scale;
}

int main(int argc, char** argv)
{
    try
    {
        unsigned long steps = 0;
        for (int argument = 1; argument < argc; argument += 2)
        {
            const std::string_view option(argv[argument]);
            if (argument + 1 >= argc || (option != "--steps" && option != "--speed"))
                throw std::invalid_argument("Usage: network [--steps positive-count] [--speed 60..300]");
            const std::string_view value(argv[argument + 1]);
            unsigned long number = 0;
            const auto result = std::from_chars(value.data(), value.data() + value.size(), number);
            if (result.ec != std::errc{} || result.ptr != value.data() + value.size() || number == 0)
                throw std::invalid_argument("Option value must be a positive integer");
            if (option == "--steps")
                steps = number;
            else
            {
                if (number < 60 || number > 300)
                    throw std::invalid_argument("Speed must be between 60 and 300");
                requestedScale = static_cast<int>(number);
            }
        }
        struct sigaction scaleAction{};
        scaleAction.sa_sigaction = requestScale;
        scaleAction.sa_flags = SA_SIGINFO;
        sigemptyset(&scaleAction.sa_mask);
        if (sigaction(SIGRTMIN, &scaleAction, nullptr) != 0)
            throw std::runtime_error("Cannot configure speed control");
        std::signal(SIGINT, requestStop);
        std::signal(SIGTERM, requestStop);
        std::signal(SIGUSR1, requestPause);
        std::signal(SIGUSR2, requestPause);
        Initializer::initialize();
        auto* train = Registry<Train>::instance()->get(0);
        if (!train)
            throw std::runtime_error("The simulation requires a train");
        std::vector<Station*> route;
        for (const std::string name : {"Paris", "Lyon", "Marseille"})
        {
            auto* station = Registry<Station>::instance()->find(
                [&](const Station& candidate) { return candidate.getName() == name; });
            if (!station)
                throw std::runtime_error("Route references missing station: " + name);
            route.push_back(station);
        }
        Simulation simulation(*train, std::move(route));
        std::cout << "Single trip | Adjustable simulation speed | Ctrl+C to stop" << std::endl;
        int scale = requestedScale;
        simulation.print(std::cout, scale);
        auto previousFrame = std::chrono::steady_clock::now();
        auto previousOutput = previousFrame;
        double accumulated = 0.0;
        bool wasPaused = false;
        unsigned long tick = 0;
        while (!stopRequested && !simulation.isFinished() && (steps == 0 || tick < steps))
        {
            const auto now = std::chrono::steady_clock::now();
            const bool paused = pauseRequested;
            const int nextScale = requestedScale;
            const bool controlsChanged = paused != wasPaused || nextScale != scale;
            if (!paused && !wasPaused && steps == 0)
                accumulated += std::chrono::duration<double>(now - previousFrame).count() * scale;
            previousFrame = now;
            wasPaused = paused;
            scale = nextScale;
            bool stateChanged = false;
            if (!paused)
            {
                for (int batch = 0; batch < 600 && !stopRequested && !pauseRequested
                    && !simulation.isFinished() && (steps == 0 || tick < steps)
                    && (steps != 0 || accumulated + 1e-9 >= Simulation::stepSeconds); ++batch)
                {
                    const auto previousState = train->getState();
                    simulation.update();
                    ++tick;
                    if (steps == 0)
                        accumulated -= Simulation::stepSeconds;
                    const bool transition = previousState != train->getState();
                    stateChanged = stateChanged || transition;
                    if (steps != 0 && (tick % 100 == 0 || transition))
                        simulation.print(std::cout, scale);
                }
            }
            if (steps == 0 && (controlsChanged || stateChanged
                || (!paused && now - previousOutput >= std::chrono::milliseconds(100))))
            {
                simulation.print(std::cout, scale);
                previousOutput = now;
            }
            if (steps == 0 || paused)
                std::this_thread::sleep_until(now + std::chrono::milliseconds(16));
        }
        simulation.print(std::cout, scale);
        std::cout << "Simulation stopped" << std::endl;
        return 0;
    }
    catch (const std::exception& error)
    {
        std::cerr << "network: " << error.what() << '\n';
        return 1;
    }
}
