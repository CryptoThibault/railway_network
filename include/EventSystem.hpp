#pragma once
#include <vector>
#include <memory>
#include "EventManager.hpp"

class EventSystem
{
private:
    std::vector<EventManager*> observers;

public:
    void subscribe(EventManager* obs)
    {
        observers.push_back(obs);
    }

    void trigger(Event e)
    {
        for (auto& obs : observers) {
            obs->onEvent(e);
        }
    }
};