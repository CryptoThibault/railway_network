#include "memento.hpp"

Memento::Snapshot Memento::save()
{
    Snapshot snapshot;
    _saveToSnapshot(snapshot);
    return snapshot;
}

void Memento::load(const Memento::Snapshot& state)
{
    Snapshot copy = state;
    _loadFromSnapshot(copy);
}