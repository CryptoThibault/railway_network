#pragma once
#include "singleton.hpp"
#include <vector>
#include <mutex>
#include <stdexcept>
#include <functional>
#include <algorithm>

template <typename T>
class Registry : public Singleton<Registry<T>>
{
    friend class Singleton<Registry<T>>;

public:
    void add(T obj);
    void add(const std::vector<T>& vec);
    void remove(const std::function<bool(const T&)>& predicate);
    void clear();

    T* get(size_t index);
    std::vector<T> getAll() const;
    T* find(const std::function<bool(const T&)>& predicate);
    size_t size() const;

    auto begin();
    auto end();
    auto begin() const;
    auto end() const;

private:
    mutable std::mutex _mutex;
    std::vector<T> _objects;
};

#include "registry.tpp"