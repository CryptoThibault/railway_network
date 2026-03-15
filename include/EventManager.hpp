#pragma once

enum class Event { PowerOutage, BombAlert, TrackMaintenance, SignalFailure };

class EventManager
{
public:
	virtual ~EventManager() {}
    virtual void onEvent(Event e) = 0;
};
