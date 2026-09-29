#pragma once

#include "telemetry.hpp"
#include <filesystem>
#include <string>
#include <sys/types.h>

class SimulationProcess
{
public:
    explicit SimulationProcess(std::filesystem::path directory);
    ~SimulationProcess();
    SimulationProcess(const SimulationProcess&) = delete;
    SimulationProcess& operator=(const SimulationProcess&) = delete;

    void start();
    void stop();
    void togglePause();
    bool isPaused() const;
    void setTimeScale(int scale);
    int getTimeScale() const;
    bool update();
    bool isRunning() const;
    const std::string& getStatus() const;
    const std::string& getOutput() const;
    const std::optional<TrainTelemetry>& getTelemetry() const;

private:
    bool readOutput();

    TelemetryReader telemetry;
    int capture = -1;
    std::string output;
    std::filesystem::path directory;
    pid_t process = -1;
    bool paused = false;
    int timeScale = 60;
    std::string status = "Ready to start";
};
