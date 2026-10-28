#include "train_log.hpp"
#include "printer.hpp"
#include <cmath>
#include <cctype>
#include <stdexcept>
#include <string>
#include <utility>
#include <unordered_set>

static std::string logFileName(const Train& train)
{
    std::string name = train.getName();
    for (char& character : name)
        if (!std::isalnum(static_cast<unsigned char>(character)) && character != '-' && character != '_')
            character = '_';
    if (name.empty())
        name = "train_" + std::to_string(train.getId());
    return name + ".txt";
}

TrainLog::TrainLog(const Registry<Train>& trains, const std::filesystem::path& directory)
    : _trains(trains)
{
    if (std::filesystem::exists(directory)
        && !std::filesystem::is_directory(std::filesystem::symlink_status(directory)))
        throw std::runtime_error("Train log path is not a directory: " + directory.string());
    std::filesystem::remove_all(directory);
    std::filesystem::create_directories(directory);
    _files.reserve(trains.size());
    std::unordered_set<std::string> names;
    for (const auto& train : trains)
    {
        std::string name = logFileName(train);
        while (!names.insert(name).second)
            name.insert(name.size() - 4, "_" + std::to_string(train.getId()));
        const auto path = directory / name;
        std::ofstream file(path, std::ios::trunc);
        if (!file)
            throw std::runtime_error("Cannot open train log: " + path.string());
        file.exceptions(std::ios::badbit | std::ios::failbit);
        _files.push_back(std::move(file));
    }
}

void TrainLog::update(double time, const std::vector<TrainEvent>& events)
{
    const bool snapshot = time + 1e-8 >= _nextSnapshot;
    std::size_t index = 0;
    for (const auto& train : _trains)
    {
        const bool boundary = std::any_of(events.begin(), events.end(), [&](const TrainEvent& event)
        {
            return event.train == &train && (event.type == TrainEventType::Arrival
                || (event.type == TrainEventType::Departure && event.firstDeparture));
        });
        const bool active = train.getState() != TrainState::Scheduled
            && train.getState() != TrainState::Completed;
        if (boundary || (active && snapshot))
        {
            auto& file = _files.at(index);
            file << Printer::formatTrainState(train, time) << '\n';
            file.flush();
        }
        ++index;
    }
    if (snapshot)
        _nextSnapshot = (std::floor((time + 1e-8) / 300.0) + 1.0) * 300.0;
}
