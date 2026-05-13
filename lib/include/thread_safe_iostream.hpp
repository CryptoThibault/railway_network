#pragma once
#include <iostream>
#include <sstream>
#include <mutex>
#include <string>

class ThreadSafeIOStream
{
public:
    template<typename T>
    ThreadSafeIOStream& operator<<(const T& value);

    ThreadSafeIOStream& operator<<(std::ostream& (*manip)(std::ostream&));

    template<typename T>
    ThreadSafeIOStream& operator>>(T& value);

    template<typename T>
    void prompt(const std::string& question, T& dest);
    
    void setPrefix(const std::string& prefix);
    void clearPrefix();
    void setOutput(std::ostream& stream);
    void resetOutput();

private:
    static std::mutex _mutex;
    static std::ostream* _output;
    static thread_local std::string _prefix;
    static thread_local bool _prefixAdded;
};

extern thread_local ThreadSafeIOStream threadSafeCout;

#include "thread_safe_iostream.tpp"