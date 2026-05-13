template <typename T>
DataBuffer& DataBuffer::operator<<(const T& value)
{
    const std::byte* raw = reinterpret_cast<const std::byte*>(&value);
    _data.insert(_data.end(), raw, raw + sizeof(T));

    return *this;
}

template <typename T>
DataBuffer& DataBuffer::operator>>(T& value)
{
    if (_readPos + sizeof(T) > _data.size())
        throw std::out_of_range("Buffer underflow");

    std::memcpy(&value, _data.data() + _readPos, sizeof(T));
    _readPos += sizeof(T);

    return *this;
}