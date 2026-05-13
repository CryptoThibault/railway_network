template<typename T>
Message& Message::operator<<(const T& value)
{
    _data << value;
    return *this;
}

template<typename T>
Message& Message::operator>>(T& value) const
{
    _data >> value;
    return const_cast<Message&>(*this);
}