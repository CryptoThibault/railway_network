#include "client.hpp"
#include "thread_safe_iostream.hpp"
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <fcntl.h>
#include <cerrno>

void Client::connect(const std::string& address, const size_t& port)
{
    if (_connected)
    {
        threadSafeCout << "Already connected, disconnecting first\n";
        disconnect();
    }

    _socket = socket(AF_INET, SOCK_STREAM, 0);
    if (_socket < 0)
    {
        threadSafeCout << "error: Failed to create socket" << std::endl;
        _connected = false;
        return;
    }

    sockaddr_in serverAddr{};
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(static_cast<uint16_t>(port));

    std::string addr = (address == "localhost") ? "127.0.0.1" : address;
    if (inet_pton(AF_INET, addr.c_str(), &serverAddr.sin_addr) <= 0)
    {
        threadSafeCout << "error: Invalid address: " << address << std::endl;
        close(_socket);
        _connected = false;
        return;
    }

    if (::connect(_socket, (struct sockaddr*)&serverAddr, sizeof(serverAddr)) < 0)
    {
        threadSafeCout << "error: Failed to connect to " << address << ":" << port << std::endl;
        close(_socket);
        _connected = false;
        return;
    }

    fcntl(_socket, F_SETFL, O_NONBLOCK);
    _connected = true;
    threadSafeCout << "Connected to " << address << ":" << port << std::endl;
}

void Client::disconnect()
{
    if (_connected)
    {
        close(_socket);
        _connected = false;
    }
}

void Client::defineAction(const Message::Type& messageType, const std::function<void(const Message& msg)>& action)
{
    _actions[messageType] = action;
}

void Client::send(const Message& message)
{
    if (!_connected)
    {
        threadSafeCout << "error: Cannot send, client not connected" << std::endl;
        return;
    }

    int type = message.type();
    uint32_t dataSize = static_cast<uint32_t>(message.data().size());

    std::vector<std::byte> sendBuffer(sizeof(type) + sizeof(dataSize) + dataSize);

    std::memcpy(sendBuffer.data(), &type, sizeof(type));
    std::memcpy(sendBuffer.data() + sizeof(type), &dataSize, sizeof(dataSize));

    if (dataSize > 0)
        std::memcpy(sendBuffer.data() + sizeof(type) + sizeof(dataSize), message.data().data(), dataSize);

    ssize_t sent = ::send(_socket, sendBuffer.data(), sendBuffer.size(), MSG_DONTWAIT);
    if (sent < 0)
        threadSafeCout << "error: Failed to send message" << std::endl;
}

void Client::update()
{
    if (!_connected) return;

    char temp[4096];
    ssize_t bytes = recv(_socket, temp, sizeof(temp), MSG_DONTWAIT);

    if (bytes <= 0)
    {
        if (bytes < 0)
        {
            if (errno == EAGAIN || errno == EWOULDBLOCK)
                return;
            threadSafeCout << "error: Socket recv failed" << std::endl;
        }
        else
        {
            threadSafeCout << "Server disconnected" << std::endl;
        }
        _connected = false;
        close(_socket);
        return;
    }

    _recvBuffer.insert(_recvBuffer.end(), reinterpret_cast<std::byte*>(temp), reinterpret_cast<std::byte*>(temp + bytes));

    constexpr size_t HEADER_SIZE = sizeof(int) + sizeof(uint32_t);

    while (true)
    {
        if (_recvBuffer.size() < HEADER_SIZE) break;

        int type;
        uint32_t size;
        std::memcpy(&type, _recvBuffer.data(), sizeof(int));
        std::memcpy(&size, _recvBuffer.data() + sizeof(int), sizeof(uint32_t));

        if (_recvBuffer.size() < HEADER_SIZE + size) break;

        Message msg(type);
        if (size > 0)
            msg.data().append(_recvBuffer.data() + HEADER_SIZE, size);

        auto it = _actions.find(type);
        if (it != _actions.end())
            it->second(msg);

        _recvBuffer.erase(_recvBuffer.begin(), _recvBuffer.begin() + HEADER_SIZE + size);
    }
}