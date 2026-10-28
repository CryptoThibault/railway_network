#include "simulation_controller.hpp"
#include <cmath>
#include <cstdlib>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <limits>
#include <stdexcept>
#include <string>
#include <unistd.h>

static void require(bool condition, const std::string& message)
{
    if (!condition)
        throw std::runtime_error(message);
}

class ScenarioFixture
{
public:
    ScenarioFixture()
    {
        char pattern[] = "/tmp/railway-dashboard-XXXXXX";
        const auto* created = mkdtemp(pattern);
        if (!created)
            throw std::runtime_error("Cannot create dashboard fixture");
        directory = created;
        std::ofstream scenario(directory / "data.json");
        scenario << R"({
            "stations":[{"name":"A","x":0,"y":0},{"name":"B","x":1,"y":0},{"name":"C","x":2,"y":0}],
            "segments":[{"a":"A","b":"B","length":2000,"maxSpeed":20},{"a":"B","b":"C","length":2000,"maxSpeed":20}],
            "trains":[
                {"name":"First","mass":100,"maxSpeed":20,"engineForce":100,"brakeForce":100,"friction":0,"length":10,"start":"A","end":"C","departure":0,"pause":30},
                {"name":"Second","mass":100,"maxSpeed":20,"engineForce":100,"brakeForce":100,"friction":0,"length":10,"start":"C","end":"A","departure":120,"pause":30}
            ]})";
    }

    ~ScenarioFixture()
    {
        std::error_code error;
        std::filesystem::remove_all(directory, error);
    }

    std::filesystem::path directory;
};

int main()
{
    try
    {
        ScenarioFixture fixture;
        SimulationController controller(fixture.directory / "data.json", fixture.directory / "log");
        require(!controller.isRunning() && !controller.hasStarted(), "Demo must start ready and paused");
        require(controller.getTrains().empty() && controller.getServiceCount() == 2, "Scheduled trains must not have cards");
        require(controller.getUpcoming() && controller.getUpcoming()->name == "First", "Empty dashboard must show the next departure");
        require(!controller.advance(30.0) && controller.getTime() == 0.0, "An unstarted dashboard must not advance");
        controller.setTimeScale(60);
        controller.togglePlayback();
        require(controller.advance(1.0), "Starting must advance the shared simulation");
        require(std::abs(controller.getTime() - 60.0) < 1e-8, "Playback must respect the selected speed");
        require(controller.getTrains().size() == 1 && controller.getTrains()[0].name == "First", "Cards must appear only at departure");
        const auto first = controller.getTrains()[0];
        require(first.stations == std::vector<std::string>({"A", "B", "C"}), "Dashboard must expose the complete chosen route");
        require(first.stationDistances == std::vector<double>({0.0, 2000.0, 4000.0}), "Station positions must follow railway distances");
        require(first.routePosition > 0.0 && first.routePosition < first.routeLength
            && first.legOrigin == "A" && first.legDestination == "B", "Train marker must follow the active leg");
        require(first.speed <= first.segmentLimit && first.parameters.mass == 100, "Live metrics must reflect core data");
        controller.togglePlayback();
        const double paused = controller.getTime();
        controller.setTimeScale(600);
        require(!controller.advance(120.0) && controller.getTime() == paused, "Pause must freeze time even if wall time passes");
        require(controller.getTrains()[0].routePosition == first.routePosition, "Pause must freeze train positions");
        controller.togglePlayback();
        controller.advance(0.0);
        require(controller.getTime() == paused, "Resume must not catch up paused wall time");
        controller.advance(0.11);
        require(std::abs(controller.getTime() - 126.0) < 1e-8, "Live speed changes must affect future time only");
        require(controller.getTrains().size() == 2, "A new departure must append another dashboard");
        const auto& waiting = controller.getTrains()[0];
        require(waiting.state == TrainState::Waiting && waiting.station == "B", "Intermediate pauses must keep the train visible");
        require(std::abs(waiting.routePosition - 2000.0) < 1e-8 && waiting.speed == 0.0,
            "Paused-at-station train marker must align exactly with the station");
        require(waiting.pauseRemaining > 0.0 && waiting.pauseRemaining <= 30.0, "Station stop must expose a countdown");
        require(controller.getTrains()[1].legOrigin == "C" && controller.getTrains()[1].legDestination == "B",
            "Reverse journeys must retain correct direction");
        controller.setTimeScale(6000);
        controller.advance(0.1);
        require(controller.getTrains().empty(), "Arrived trains must leave the list");
        require(controller.getUpcoming() && controller.getUpcoming()->time == 86400.0,
            "After arrivals the next daily departure must be exposed");
        while (controller.getTime() < 86460.0)
            controller.advance(0.01);
        require(controller.getTrains().size() == 1 && controller.getTrains()[0].name == "First",
            "The next day's services must reappear without restarting the app");
        require(controller.getTrains()[0].started >= 86400.0 && controller.getTrains()[0].routePosition < 2000.0,
            "Daily restarts must reset the progress bar");
        const auto beforeLag = controller.getTime();
        controller.advance(100.0);
        require(controller.getTime() - beforeLag <= 1000.0001, "Large frame delays must not monopolize the UI thread");
        controller.togglePlayback();
        controller.togglePlayback();
        const auto resumed = controller.getTime();
        controller.advance(0.0);
        require(controller.getTime() == resumed, "Pausing must discard queued playback backlog");
        bool invalidSpeed = false;
        try { controller.setTimeScale(0); } catch (const std::invalid_argument&) { invalidSpeed = true; }
        require(invalidSpeed, "Invalid speed must be rejected");
        bool invalidFrame = false;
        try { controller.advance(std::numeric_limits<double>::quiet_NaN()); }
        catch (const std::invalid_argument&) { invalidFrame = true; }
        require(invalidFrame, "Nonfinite frame time must be rejected");
        require(std::filesystem::exists(fixture.directory / "log" / "First.txt")
            && std::filesystem::exists(fixture.directory / "log" / "Second.txt"), "GUI must retain per-train logs");
        std::cout << "GUI controller checks passed\n";
        return 0;
    }
    catch (const std::exception& error)
    {
        std::cerr << error.what() << '\n';
        return 1;
    }
}
