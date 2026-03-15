#pragma once
#include <string>
#include <vector>

class Segment;

class Station
{
private:
    std::string name;
    std::vector<Segment*> connections;
    bool closed;
    int closeTicks = 0;

public:
    Station(const std::string& n) : name(n) {}
    
    void addSegment(Segment* segment) { connections.push_back(segment); }

    void setClosed(bool c) { closed = c; }
    void setCloseTicks(int ticks) { closeTicks = ticks; }
    
    std::string getName() const { return name; }
    const std::vector<Segment*>& getConnections() const { return connections; }
    bool isClosed() const { return closed; }
    int getCloseTicks() const { return closeTicks; }

};