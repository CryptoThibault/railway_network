#include "telemetry.hpp"
#include <cmath>
#include <regex>

void TelemetryReader::append(std::string_view text)
{
    for (char character : text)
    {
        if (character == '\n')
        {
            if (!discarding)
                parse(pending);
            pending.clear();
            discarding = false;
        }
        else if (!discarding)
        {
            pending += character;
            if (pending.size() > 8192)
            {
                pending.clear();
                discarding = true;
            }
        }
    }
}

void TelemetryReader::parse(const std::string& line)
{
    static const std::regex pattern(
        R"(^Time (\d+h\d+m\d+s) \| (\d+)x \| Train (\d+) \(([^)]+)\) \| State: (Waiting|Accelerating|Cruising|Braking|Idle) \| Speed: (\d+(?:\.\d+)?) km/h \| Segment: (.+?) -> (.+?) \| Position: (\d+(?:\.\d+)?) / (\d+(?:\.\d+)?) km \| Legs: (\d+)$)");
    std::smatch match;
    if (!std::regex_match(line, match, pattern))
        return;
    try
    {
        TrainTelemetry value;
        value.time = match[1];
        value.timeScale = std::stoi(match[2]);
        value.train = match[3];
        value.type = match[4];
        value.state = match[5];
        value.speed = std::stod(match[6]);
        value.departure = match[7];
        value.destination = match[8];
        value.position = std::stod(match[9]);
        value.length = std::stod(match[10]);
        value.legs = std::stoul(match[11]);
        if (value.timeScale < 60 || value.timeScale > 300 || !std::isfinite(value.speed) || !std::isfinite(value.position)
            || !std::isfinite(value.length) || value.length <= 0.0
            || value.position > value.length)
            return;
        snapshot = std::move(value);
    }
    catch (const std::exception&)
    {
    }
}

void TelemetryReader::reset()
{
    pending.clear();
    discarding = false;
    snapshot.reset();
}

const std::optional<TrainTelemetry>& TelemetryReader::getSnapshot() const
{
    return snapshot;
}
