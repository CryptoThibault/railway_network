template<typename T>
Logger::LogLine& Logger::LogLine::operator<<(const T& value)
{
    if (_level < _logger._currentLevel) return *this;

    if (!_prefixAdded)
    {
        std::string prefix;
        if(_logger._showTime)
            prefix += "[" + _logger.getCurrentTime() + "] ";
        if (_logger._showLevel)
            prefix += "[" + _logger.toString(_level) + "] ";
        threadSafeCout.setPrefix(prefix);
        _prefixAdded = true;
    }

    threadSafeCout << value;
    return *this;
}