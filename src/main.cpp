#include "initializer.hpp"
#include "logger.hpp"
#include "printer.hpp"
#include "network_simulation.hpp"
#include "train_log.hpp"
#include <charconv>
#include <chrono>
#include <csignal>
#include <iostream>
#include <stdexcept>
#include <string>
#include <string_view>
#include <thread>

static volatile std::sig_atomic_t stopRequested = 0;

static void requestStop(int)
{
    stopRequested = 1;
}

static unsigned long positiveNumber(std::string_view value)
{
    unsigned long number = 0;
    const auto result = std::from_chars(value.data(), value.data() + value.size(), number);
    if (result.ec != std::errc{} || result.ptr != value.data() + value.size() || number == 0)
        throw std::invalid_argument("Option value must be a positive integer");
    return number;
}

int main(int argc, char** argv)
{
    try
    {
        Logger::instantiate();
        Logger::instance()->setLevel(Logger::Level::Info);
        bool dataOnly = false;
        bool once = false;
        unsigned long days = 0;
        unsigned long speed = 1200;
        for (int index = 1; index < argc; ++index)
        {
            const std::string_view option(argv[index]);
            if (option == "--data-only")
                dataOnly = true;
            else if (option == "--once")
                once = true;
            else if ((option == "--days" || option == "--speed") && index + 1 < argc)
            {
                const auto number = positiveNumber(argv[++index]);
                if (option == "--days")
                    days = number;
                else
                    speed = number;
            }
            else
                throw std::invalid_argument("Usage: network [--data-only | --once | --days count] [--speed scale]");
        }
        if ((once && days != 0) || (dataOnly && (once || days != 0)))
            throw std::invalid_argument("Choose only one of --data-only, --once, or --days");
        std::signal(SIGINT, requestStop);
        std::signal(SIGTERM, requestStop);
        Initializer::initialize();
        Printer::printStations();
        Printer::printSegments();
        Printer::printTrains();
        if (dataOnly)
            return 0;

        auto& trains = *Registry<Train>::instance();
        NetworkSimulation simulation(trains, !once);
        TrainLog logs(trains);
        logs.update(simulation.getTime());
        const bool fast = once || days != 0;
        Logger::instance()->info() << "Simulation started | " << trains.size()
            << " trains | " << Printer::formatTime(simulation.getTime())
            << (fast ? " | Fast run" : " | Playback: ")
            << (fast ? "" : std::to_string(speed) + "x | Ctrl+C to stop");
        const auto started = std::chrono::steady_clock::now();
        const double initialTime = simulation.getTime();
        const double endTime = static_cast<double>(days) * 86400.0;
        while (!stopRequested && !simulation.isFinished()
            && (days == 0 || simulation.getTime() + 1e-8 < endTime))
        {
            if (!fast)
            {
                const double elapsed = std::chrono::duration<double>(
                    std::chrono::steady_clock::now() - started).count();
                if (simulation.getTime() + Simulation::stepSeconds > initialTime + elapsed * speed)
                {
                    std::this_thread::sleep_for(std::chrono::milliseconds(16));
                    continue;
                }
            }
            simulation.update();
            Printer::printEvents(simulation.getEvents());
            logs.update(simulation.getTime(), simulation.getEvents());
        }
        Logger::instance()->info() << (stopRequested ? "Simulation stopped | " : "Simulation finished | ")
            << Printer::formatTime(simulation.getTime());
        return 0;
    }
    catch (const std::exception& error)
    {
        std::cerr << "network: " << error.what() << '\n';
        return 1;
    }
}
