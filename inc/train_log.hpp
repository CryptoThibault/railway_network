#pragma once
#include "train.hpp"
#include "train_event.hpp"
#include <algorithm>
#include "registry.hpp"
#include <filesystem>
#include <fstream>
#include <vector>

class TrainLog
{
public:
    explicit TrainLog(const Registry<Train>& trains, const std::filesystem::path& directory = "log");

    void update(double time, const std::vector<TrainEvent>& events = {});

private:
    const Registry<Train>& _trains;
    std::vector<std::ofstream> _files;
    double _nextSnapshot = 0.0;
};
