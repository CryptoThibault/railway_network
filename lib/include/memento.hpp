#pragma once
#include "data_buffer.hpp"

class Memento {
public:
    class Snapshot
    {
    public:
        Snapshot() = default;

        template<typename T>
        Snapshot& operator<<(const T& value);

        template<typename T>
        Snapshot& operator>>(T& value);

    private:
        DataBuffer buffer;

        template<typename T>
        void write(const T& value);

        template<typename T>
        void read(T& value);

        friend class Memento;
    };

    Snapshot save();
    void load(const Memento::Snapshot& state);

private:
    virtual void _saveToSnapshot(Snapshot& snapshot) const = 0;
    virtual void _loadFromSnapshot(Snapshot& snapshot) = 0;
};

#include "memento.tpp"