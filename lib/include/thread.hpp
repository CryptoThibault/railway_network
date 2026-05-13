#pragma once
#include <string>
#include <functional>
#include <thread>

class Thread
{
public:
    Thread(const std::string& name, std::function<void()> functToExecute);

    void start();
    void stop();

private:
    std::string _name;
    std::function<void()> _functToExecute;
    std::thread _thread;
};