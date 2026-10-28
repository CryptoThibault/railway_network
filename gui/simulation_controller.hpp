#pragma once
#include "network_simulation.hpp"
#include "train_log.hpp"
#include <filesystem>
#include <memory>
#include <optional>
#include <string>
#include <unordered_map>
#include <vector>

struct TrainDashboard
{
    long id;
    std::string name;
    TrainState state;
    TrainParameters parameters;
    std::string origin;
    std::string destination;
    std::string station;
    std::string legOrigin;
    std::string legDestination;
    std::vector<std::string> stations;
    std::vector<double> stationDistances;
    double speed;
    double segmentLimit;
    double segmentPosition;
    double segmentLength;
    double routePosition;
    double routeLength;
    double departure;
    double started;
    double elapsed;
    double pauseRemaining;
};

struct UpcomingService
{
    std::string name;
    std::string origin;
    std::string destination;
    double time;
};

class SimulationController
{
public:
    SimulationController(const std::filesystem::path& scenario, const std::filesystem::path& logs);

    void togglePlayback();
    void setTimeScale(int scale);
    bool advance(double realSeconds);
    bool isRunning() const;
    bool hasStarted() const;
    int getTimeScale() const;
    double getTime() const;
    std::size_t getServiceCount() const;
    const std::vector<TrainDashboard>& getTrains() const;
    const std::optional<UpcomingService>& getUpcoming() const;

private:
    void refresh();

    std::unique_ptr<NetworkSimulation> simulation;
    std::unique_ptr<TrainLog> logs;
    std::vector<TrainDashboard> trains;
    std::optional<UpcomingService> upcoming;
    std::unordered_map<long, double> departures;
    std::unordered_map<long, double> stops;
    double pending = 0.0;
    int timeScale = 1200;
    bool running = false;
    bool started = false;
};
