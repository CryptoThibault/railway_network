#pragma once
#include "thread_safe_iostream.hpp"
#include <string>
#include <sstream>
#include <fstream>
#include "singleton.hpp"

class Logger : public Singleton<Logger>
{
public:
    enum Level { Trace, Debug, Info, Warning, Error, Critical };
    
    class LogLine
    {
    public:
        LogLine(Logger& logger, Level level);
        ~LogLine();

        template<typename T>
        LogLine& operator<<(const T& value);

        LogLine& operator<<(std::ostream& (*manip)(std::ostream&));

    private:
        void flush();

        Logger& _logger;
        Level _level;
        std::ostringstream _oss;
        bool _prefixAdded{};
    };

    LogLine log(Level level);
    LogLine trace();
    LogLine debug();
    LogLine info();
    LogLine warning();
    LogLine error();
    LogLine critical();

    void setLevel(Level level);
    void setShowLevel(bool show);
    void setShowTime(bool show);

private:
    std::ofstream _output;
    Level _currentLevel{Level::Info};
    bool _showLevel{};
    bool _showTime{};

    std::string toString(Level level);
    std::string getCurrentTime() const;

    friend class Singleton<Logger>;
};

#include "logger.tpp"