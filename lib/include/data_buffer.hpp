#pragma once
#include <vector>
#include <string>
#include <cstddef>
#include <cstring>
#include <stdexcept>

class DataBuffer
{
public:
    template <typename T>
    DataBuffer& operator<<(const T& value);

    template <typename T>
    DataBuffer& operator>>(T& value);

    DataBuffer& operator<<(const std::string& str);
    DataBuffer& operator>>(std::string& str);

    void clear();
    size_t size() const;
    const std::byte* data() const;
    void append(const void* data, size_t size);

private:
    std::vector<std::byte> _data;
    size_t _readPos{};
};

#include "data_buffer.tpp"