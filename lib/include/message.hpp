#pragma once
#include "data_buffer.hpp"

class Message {
public:
    using Type = int;
    
    Message(int type);

    template<typename T>
    Message& operator<<(const T& value);

    template<typename T>
    Message& operator>>(T& value) const;

    DataBuffer& data();
    const DataBuffer& data() const;
    int type() const;

private:
    int _type;
    mutable DataBuffer _data;
};

#include "message.tpp"