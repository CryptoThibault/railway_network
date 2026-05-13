#pragma once
#include "message.hpp"
#include "client.hpp"
#include "worker_pool.hpp"
#include <functional>
#include <vector>
#include <unordered_map>

class Server
{
public:
    using Action = std::function<void(long long& clientID, const Message& msg)>;

    void start(const size_t& port);
    void defineAction(const Message::Type& messageType, const Action& action);
    void sendTo(const Message& message, long long clientID);
    void sendToArray(const Message& message, std::vector<long long> clientIDs);
    void sendToAll(const Message& message);
    void update();

private:
    struct Connection { int socket; std::vector<std::byte> recvBuffer; };

    int _socket;
    bool _started{};
    long long _nextClientID{1};
    std::unordered_map<long long, Connection> _clients;
    std::unordered_map<Message::Type, Action> _actions;
    WorkerPool _workerPool{std::thread::hardware_concurrency()};
};