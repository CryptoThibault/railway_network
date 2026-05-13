template<typename T>
Memento::Snapshot& Memento::Snapshot::operator<<(const T& value)
{
    write(value);
    return *this;
}

template<typename T>
Memento::Snapshot& Memento::Snapshot::operator>>(T& value)
{
    read(value);
    return *this;
}

template<typename T>
void Memento::Snapshot::write(const T& value)
{
    buffer << value;
}

template<typename T>
void Memento::Snapshot::read(T& value)
{
    buffer >> value;
}