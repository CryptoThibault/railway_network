#include "thread_safe_iostream.hpp"

std::mutex ThreadSafeIOStream::_mutex;
std::ostream* ThreadSafeIOStream::_output{&std::cout};
thread_local std::string ThreadSafeIOStream::_prefix;
thread_local bool ThreadSafeIOStream::_prefixAdded{};
thread_local ThreadSafeIOStream threadSafeCout;

ThreadSafeIOStream& ThreadSafeIOStream::operator<<(std::ostream& (*manip)(std::ostream&))
{
    std::lock_guard<std::mutex> lock(_mutex);
    (*_output) << manip << std::flush;

    if (manip == static_cast<std::ostream& (*)(std::ostream&)>(std::endl))
        _prefixAdded = false;

    return *this;
}

void ThreadSafeIOStream::setPrefix(const std::string& prefix)
{
    _prefix = prefix;
    _prefixAdded = false;
}

void ThreadSafeIOStream::clearPrefix()
{   
    _prefix.clear();
    _prefixAdded = false;
}

void ThreadSafeIOStream::setOutput(std::ostream& stream)
{
    std::lock_guard<std::mutex> lock(_mutex);
    _output = &stream;
}

void ThreadSafeIOStream::resetOutput()
{
    std::lock_guard<std::mutex> lock(_mutex);
    _output = &std::cout;
}