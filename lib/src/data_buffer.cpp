#include "data_buffer.hpp"

void DataBuffer::clear()
{
    _data.clear();
    _readPos = 0;
}

size_t DataBuffer::size() const
{
    return _data.size();
}

const std::byte* DataBuffer::data() const
{
    return _data.data();
}

void DataBuffer::append(const void* data, size_t size)
{
    const std::byte* ptr = reinterpret_cast<const std::byte*>(data);
    _data.insert(_data.end(), ptr, ptr + size);
}

DataBuffer& DataBuffer::operator<<(const std::string& str)
{
    size_t size = str.size();
    *this << size;

    const std::byte* ptr = reinterpret_cast<const std::byte*>(str.data());
    _data.insert(_data.end(), ptr, ptr + size);

    return *this;
}

DataBuffer& DataBuffer::operator>>(std::string& str)
{
    size_t size;
    *this >> size;

    if (_readPos + size > _data.size())
        throw std::out_of_range("Buffer underflow");

    str.assign(reinterpret_cast<char*>(_data.data() + _readPos), size);
    _readPos += size;

    return *this;
}