#pragma once

#include <optional>
#include <string>
#include <string_view>
#include <vector>

struct TrainTelemetry
{
    std::string time;
    std::string train;
    std::string type;
    std::string state;
    std::string departure;
    std::string destination;
    int timeScale = 60;
    double speed = 0.0;
    double position = 0.0;
    double length = 0.0;
    unsigned long legs = 0;
    std::vector<std::string> stations;
    std::vector<double> segmentLengths;
    unsigned long waitingSeconds = 0;
};

class TelemetryReader
{
public:
    void append(std::string_view text);
    void reset();
    const std::optional<TrainTelemetry>& getSnapshot() const;

private:
    void parse(const std::string& line);
    std::string pending;
    bool discarding = false;
    std::optional<TrainTelemetry> snapshot;
};
