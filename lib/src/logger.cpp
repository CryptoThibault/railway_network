#include "logger.hpp"
#include <chrono>
#include <ctime>
#include <iomanip>

Logger::LogLine::LogLine(Logger& logger, Level level) : _logger(logger), _level(level) {}

Logger::LogLine::~LogLine()
{
    if (_level < _logger._currentLevel) return;
    threadSafeCout << std::endl;
}

Logger::LogLine& Logger::LogLine::operator<<(std::ostream& (*manip)(std::ostream&))
{
    if (_level < _logger._currentLevel) return *this;
    threadSafeCout << manip;
    return *this;
}

Logger::LogLine Logger::log(Level level) { return LogLine(*this, level); }
Logger::LogLine Logger::trace() { return log(Level::Trace); }
Logger::LogLine Logger::debug() { return log(Level::Debug); }
Logger::LogLine Logger::info() { return log(Level::Info); }
Logger::LogLine Logger::warning() { return log(Level::Warning); }
Logger::LogLine Logger::error() { return log(Level::Error); }
Logger::LogLine Logger::critical() { return log(Level::Critical); }

void Logger::setLevel(Level level) { _currentLevel = level; }
void Logger::setShowLevel(bool show) { _showLevel = show; }
void Logger::setShowTime(bool show) { _showTime = show; }

std::string Logger::toString(Level level)
{
    switch(level)
    {
        case Level::Debug: return "DEBUG";
        case Level::Info: return "INFO";
        case Level::Warning: return "WARNING";
        case Level::Error: return "ERROR";
        case Level::Critical: return "CRITICAL";
        default: return "UNKNOWN";
    }
}

std::string Logger::getCurrentTime() const
{
    static std::mutex timeMutex;

    auto now = std::chrono::system_clock::now();
    auto timeT = std::chrono::system_clock::to_time_t(now);

    std::tm localTime{};
    {
        std::lock_guard<std::mutex> lock(timeMutex);
        localTime = *std::localtime(&timeT);
    }

    std::ostringstream oss;
    oss << std::put_time(&localTime, "%Y-%m-%d %H:%M:%S");
    return oss.str();
}