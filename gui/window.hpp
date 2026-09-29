#pragma once

#include <memory>

class ApplicationWindow
{
public:
    ApplicationWindow();
    ~ApplicationWindow();
    ApplicationWindow(const ApplicationWindow&) = delete;
    ApplicationWindow& operator=(const ApplicationWindow&) = delete;

    void run();

private:
    struct Implementation;
    std::unique_ptr<Implementation> implementation;
};
