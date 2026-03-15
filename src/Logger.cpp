#include "Logger.hpp"
#include "Registry.hpp"
#include "Network.hpp"
#include "Train.hpp"
#include "Segment.hpp"

Logger::Logger(double intervalMinutes) : lastPrintTime(-1), printInterval(intervalMinutes / 60.0)
{
    if (mkdir("log", 0755) != 0 && errno != EEXIST) {
        std::cerr << "Error : impossible to create log folder\n";
    }
    system("rm -f log/*.result");
}

void Logger::logTrains(double currentTime)
{
    if (currentTime - lastPrintTime < printInterval)
        return;

    lastPrintTime = currentTime;

    for (const auto& trainPtr : Registry<Train>::getInstance().getList()) {
        Train* t = trainPtr.get();
        if (t->getState() != TrainState::Running)
            continue;

        logTrain(t, currentTime);
    }
}

void Logger::logTrain(Train* t, double currentTime)
{
    std::ofstream logFile("log/" + t->getName() + "_" + formatTime(t->getDepartureHour()) + ".result", std::ios::app);
    if (!logFile.is_open()) {
        std::cerr << "Error : impossible log file for "
                  << t->getName() << "\n";
        return;
    }

    auto* seg = t->getBoard().getCurrentSegment();
    if (!seg) return;

    double segmentLength = seg->getLength();
    double distanceDone = t->getBoard().getDistanceOnSegment();
    double distanceLeft = segmentLength - distanceDone ;
    double pathDistance = Network::getInstance().calculatePathDistance(t);

    std::string time = formatTime(currentTime - t->getDepartureHour());

    Station* nextStation = t->getBoard().getNextStation();
    Station* fromStation = nullptr;
    Station* toStation = nullptr;

    if (nextStation) {
        fromStation = seg->getOtherStation(nextStation);
        toStation = nextStation;
    }

    if (!fromStation || !toStation) {
        fromStation = seg->getStationA();
        toStation = seg->getStationB();
    }

    std::string from = fromStation->getName();
    std::string to = toStation->getName();

    std::string state = computeState(t, distanceLeft, seg);

    std::string graph = buildRailGraph(segmentLength, distanceDone);

    logFile << "[" << time << "] - "
         << "[" << from << "]"
         << "[" << to << "] - "
         << "[" << std::fixed << std::setprecision(2) << pathDistance << "km] - "
         << "[" << state << "] - "
         << graph
         << "\n";
}

std::string Logger::formatTime(double currentTime)
{
    int totalMinutes = static_cast<int>(currentTime * 60);
    int hours = totalMinutes / 60;
    int minutes = totalMinutes % 60;

    char buffer[10];
    sprintf(buffer, "%02dh%02d", hours, minutes);

    return std::string(buffer);
}

std::string Logger::computeState(Train* t, double distanceLeft, Segment* seg) const
{
    if (t->getBoard().getStopTimeRemaining() > 0)
        return "Stopped";

    double stopping = t->getMotion().computeStoppingDistance();

    const double safetyMargin = 0.5;
    if (distanceLeft <= stopping + safetyMargin)
        return "Braking";

    if (t->getMotion().getSpeed() < seg->getMaxAllowedSpeed())
        return "Speed up";

    return "Cruising";
}

std::string Logger::buildRailGraph(double segmentLength, double distanceDone) const
{
    int totalCells = static_cast<int>(segmentLength);
    int trainCell = static_cast<int>(distanceDone);

    std::string bar;

    for (int i = 0; i < totalCells; i++) {

        bool nearStart = (i < 2);
        bool nearEnd = (i >= totalCells - 2);
        bool nearTrain = (i >= trainCell - 2 && i <= trainCell + 2);

        if (i == trainCell) {
            bar += "[x]";
            continue;
        }

        if (nearStart || nearEnd || nearTrain) {
            bar += "[ ]";
            continue;
        }

        if (i == 2 || i == trainCell + 3) {
            bar += "[ ... ]";
        }

        if (i > 2 && i < trainCell - 2)
            continue;

        if (i > trainCell + 2 && i < totalCells - 2)
            continue;
    }

    return bar;
}