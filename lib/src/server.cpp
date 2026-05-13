#include "server.hpp"
#include "thread_safe_iostream.hpp"
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <fcntl.h>

void Server::start(const size_t& port)
{
    if (_started)
    {
        threadSafeCout << "error: Server already started" << std::endl;
        return;
    }

    _socket = socket(AF_INET, SOCK_STREAM, 0);
    if (_socket < 0)
    {
        threadSafeCout << "error: Failed to create socket" << std::endl;
        return;
    }

    sockaddr_in addr{};
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = INADDR_ANY;
    addr.sin_port = htons(port);

    if (bind(_socket, (struct sockaddr*)&addr, sizeof(addr)) < 0)
    {
        close(_socket);
        threadSafeCout << "error: Bind failed" << std::endl;
        return;
    }

    if (listen(_socket, SOMAXCONN) < 0)
    {
        close(_socket);
        threadSafeCout << "error: Listen failed" << std::endl;
        return;
    }

    fcntl(_socket, F_SETFL, O_NONBLOCK);
    _started = true;
    threadSafeCout << "Server started on port " << port << std::endl;
}

void Server::defineAction(const Message::Type& messageType, const Action& action)
{
    _actions[messageType] = action;
}

void Server::sendTo(const Message& message, long long clientID)
{
    int type = message.type();
    uint32_t dataSize = static_cast<uint32_t>(message.data().size());
    std::vector<std::byte> sendBuffer(sizeof(type) + sizeof(dataSize) + dataSize);

    std::memcpy(sendBuffer.data(), &type, sizeof(type));
    std::memcpy(sendBuffer.data() + sizeof(type), &dataSize, sizeof(dataSize));

    if (dataSize > 0)
        std::memcpy(sendBuffer.data() + sizeof(type) + sizeof(dataSize), message.data().data(), dataSize);

    auto it = _clients.find(clientID);
    if (it == _clients.end()) return;

    ssize_t sent = ::send(it->second.socket, sendBuffer.data(), sendBuffer.size(), MSG_DONTWAIT);
    if (sent < 0)
        threadSafeCout << "error: Failed to send to client " << clientID << std::endl;
}

void Server::sendToArray(const Message& message, std::vector<long long> clientIDs)
{
    for (auto id : clientIDs)
        sendTo(message, id);
}

void Server::sendToAll(const Message& message)
{
    for (auto& [clientID, sock] : _clients)
        sendTo(message, clientID);
}

void Server::update()
{
    if (!_started) return;

    sockaddr_in clientAddr;
    socklen_t addrLen = sizeof(clientAddr);
    int clientSocket = accept(_socket, (sockaddr*)&clientAddr, &addrLen);
    if (clientSocket >= 0)
    {
        long long clientID = _nextClientID++;
        _clients[clientID] = {clientSocket, {}};
        threadSafeCout << "New client connected: " << clientID << std::endl;
    }

    std::vector<long long> disconnectedClients;

    for (auto& [clientID, connection] : _clients)
    {
        char temp[4096];
        ssize_t bytes = recv(connection.socket, temp, sizeof(temp), MSG_DONTWAIT);

        if (bytes == 0)
        {
            threadSafeCout << "Client disconnected: " << clientID << std::endl;
            disconnectedClients.push_back(clientID);
            continue;
        }
        else if (bytes < 0)
        {
            continue;
        }

        auto& buffer = _clients[clientID].recvBuffer;
        buffer.insert(buffer.end(), reinterpret_cast<std::byte*>(temp), reinterpret_cast<std::byte*>(temp + bytes));

        while (true)
        {
            if (buffer.size() < sizeof(int) + sizeof(uint32_t))
                break;

            int type;
            uint32_t size;

            std::memcpy(&type, buffer.data(), sizeof(int));
            std::memcpy(&size, buffer.data() + sizeof(int), sizeof(uint32_t));

            if (buffer.size() < sizeof(int) + sizeof(uint32_t) + size)
                break;

            Message msg(type);
            msg.data().append(buffer.data() + sizeof(int) + sizeof(uint32_t), size);
            buffer.erase(buffer.begin(), buffer.begin() + sizeof(int) + sizeof(uint32_t) + size);

            _workerPool.addJob([this, clientID, msg]() mutable {
                auto it = _actions.find(msg.type());
                if (it != _actions.end())
                {
                    long long idCopy = clientID;
                    it->second(idCopy, msg);
                }
            });
        }
    }

    for (long long cid : disconnectedClients)
    {
        close(_clients[cid].socket);
        _clients.erase(cid);
    }
}