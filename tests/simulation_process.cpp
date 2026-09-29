#include "simulation_process.hpp"
#include <chrono>
#include <cstdio>
#include <iostream>
#include <fstream>
#include <cstdlib>
#include <stdexcept>
#include <thread>

static void require(bool condition, const char* message)
{
    if (!condition)
        throw std::runtime_error(message);
}

static unsigned long elapsedSeconds(const TrainTelemetry& telemetry)
{
    unsigned long hours = 0;
    unsigned long minutes = 0;
    unsigned long seconds = 0;
    require(std::sscanf(telemetry.time.c_str(), "%luh%lum%lus", &hours, &minutes, &seconds) == 3,
        "Invalid telemetry clock");
    return hours * 3600 + minutes * 60 + seconds;
}

class OutputFixture
{
public:
    OutputFixture()
    {
        char pattern[] = "/tmp/railway-output-XXXXXX";
        const char* created = mkdtemp(pattern);
        if (!created)
            throw std::runtime_error("Cannot create output fixture");
        directory = created;
    }

    ~OutputFixture()
    {
        std::error_code error;
        std::filesystem::remove_all(directory, error);
    }

    std::filesystem::path directory;
};

int main(int argc, char** argv)
{
    try
    {
        require(argc == 2, "Expected repository path");
        TelemetryReader reader;
        const std::string sample = "Time 1h2m3s | 60x | Train 1 (TGV) | State: Cruising | Speed: 250.0 km/h | Segment: Paris -> Lyon | Position: 120.0 / 427.0 km | Legs: 2";
        reader.append(sample.substr(0, 50));
        require(!reader.getSnapshot(), "Partial telemetry was accepted");
        reader.append(sample.substr(50) + "\n");
        require(reader.getSnapshot() && reader.getSnapshot()->speed == 250.0
            && reader.getSnapshot()->departure == "Paris" && reader.getSnapshot()->legs == 2,
            "Telemetry fields were not decoded");
        reader.append("noise\nTime invalid\n");
        require(reader.getSnapshot()->speed == 250.0, "Invalid output replaced valid telemetry");
        reader.reset();
        require(!reader.getSnapshot(), "Telemetry reset failed");
        reader.append(std::string(9000, 'x') + "\n" + sample + "\n");
        require(reader.getSnapshot().has_value(), "Telemetry did not recover after an oversized line");
        SimulationProcess simulation(argv[1]);
        require(!simulation.isRunning(), "Simulation started before requested");
        for (int run = 0; run < 2; ++run)
        {
            simulation.start();
            require(simulation.getOutput().empty(), "Previous output was not cleared");
            require(simulation.isRunning(), "Simulation did not start");
            simulation.start();
            const auto deadline = std::chrono::steady_clock::now() + std::chrono::seconds(5);
            while (simulation.getOutput().find("Speed:") == std::string::npos
                && std::chrono::steady_clock::now() < deadline)
            {
                simulation.update();
                std::this_thread::sleep_for(std::chrono::milliseconds(10));
            }
            require(simulation.isRunning(), "Continuous simulation exited unexpectedly");
            require(simulation.getOutput().find("Speed:") != std::string::npos,
                "Train telemetry was not captured");
            require(simulation.getTelemetry().has_value(), "Dashboard telemetry is missing");
            simulation.togglePause();
            require(simulation.isPaused(), "Pause state was not set");
            std::this_thread::sleep_for(std::chrono::milliseconds(50));
            simulation.update();
            const auto pausedTime = simulation.getTelemetry()->time;
            const auto pausedPosition = simulation.getTelemetry()->position;
            std::this_thread::sleep_for(std::chrono::milliseconds(400));
            simulation.update();
            require(simulation.getTelemetry()->time == pausedTime, "Clock advanced during pause");
            require(simulation.getTelemetry()->position == pausedPosition, "Train moved during pause");
            simulation.togglePause();
            require(!simulation.isPaused(), "Resume state was not set");
            std::this_thread::sleep_for(std::chrono::milliseconds(250));
            simulation.update();
            require(simulation.getTelemetry()->time != pausedTime, "Clock did not resume");
            require(simulation.getTelemetry()->time.find("0h0m") == 0,
                "Clock jumped after resume");
            require(std::stoi(simulation.getTelemetry()->time.substr(4)) < 30,
                "Resume caught up with paused time");
            if (run == 1)
                simulation.togglePause();
            simulation.stop();
            require(!simulation.isPaused(), "Stop retained the pause state");
            require(!simulation.isRunning(), "Simulation did not stop");
            require(simulation.getStatus() == "Simulation stopped", "Stop status missing");
        }
        simulation.setTimeScale(120);
        simulation.start();
        const auto speedDeadline = std::chrono::steady_clock::now() + std::chrono::seconds(3);
        while (!simulation.getTelemetry() && std::chrono::steady_clock::now() < speedDeadline)
        {
            simulation.update();
            std::this_thread::sleep_for(std::chrono::milliseconds(10));
        }
        require(simulation.getTelemetry() && simulation.getTelemetry()->timeScale == 120,
            "Initial speed was not applied");
        simulation.togglePause();
        std::this_thread::sleep_for(std::chrono::milliseconds(60));
        simulation.update();
        const auto frozenTime = elapsedSeconds(*simulation.getTelemetry());
        simulation.setTimeScale(300);
        std::this_thread::sleep_for(std::chrono::milliseconds(60));
        simulation.update();
        require(simulation.getTelemetry()->timeScale == 300, "Paused speed change was not applied");
        require(elapsedSeconds(*simulation.getTelemetry()) == frozenTime, "Speed change advanced paused time");
        simulation.togglePause();
        std::this_thread::sleep_for(std::chrono::milliseconds(350));
        simulation.update();
        const auto fastTime = elapsedSeconds(*simulation.getTelemetry());
        require(fastTime - frozenTime >= 60 && fastTime - frozenTime <= 135,
            "300x playback did not use the expected clock rate");
        simulation.setTimeScale(60);
        std::this_thread::sleep_for(std::chrono::milliseconds(60));
        simulation.update();
        require(simulation.getTelemetry()->timeScale == 60, "Live speed reduction was not applied");
        const auto slowStart = elapsedSeconds(*simulation.getTelemetry());
        std::this_thread::sleep_for(std::chrono::milliseconds(350));
        simulation.update();
        const auto slowTime = elapsedSeconds(*simulation.getTelemetry());
        require(slowTime >= slowStart + 10 && slowTime <= slowStart + 35,
            "Live speed reduction jumped the clock");
        simulation.setTimeScale(301);
        require(simulation.getTimeScale() == 60, "Out-of-range speed was accepted");
        simulation.stop();
        SimulationProcess missing(std::filesystem::path(argv[1]) / "CMakeLists.txt");
        missing.start();
        require(!missing.isRunning(), "Missing executable started");
        require(missing.getStatus().find("Cannot launch network:") == 0, "Missing executable was not reported");
        OutputFixture fixture;
        const auto executable = fixture.directory / "network";
        {
            std::ofstream script(executable);
            script << "#!/bin/sh\nprintf 'first line\\n'\nsleep 0.2\nprintf 'error line\\n' >&2\nexit 7\n";
        }
        std::filesystem::permissions(executable, std::filesystem::perms::owner_all);
        SimulationProcess failing(fixture.directory);
        failing.start();
        bool liveOutput = false;
        const auto deadline = std::chrono::steady_clock::now() + std::chrono::seconds(5);
        while (failing.isRunning() && std::chrono::steady_clock::now() < deadline)
        {
            failing.update();
            liveOutput = liveOutput || (failing.isRunning()
                && failing.getOutput().find("first line") != std::string::npos);
            std::this_thread::sleep_for(std::chrono::milliseconds(10));
        }
        require(!failing.isRunning(), "Output fixture did not finish");
        require(liveOutput, "Output was not available while running");
        require(failing.getOutput() == "first line\nerror line\n", "stdout or stderr was lost");
        require(failing.getStatus() == "Simulation failed (exit 7)", "Failure status was lost");
        return 0;
    }
    catch (const std::exception& error)
    {
        std::cerr << error.what() << '\n';
        return 1;
    }
}
