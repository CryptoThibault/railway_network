#pragma once
#include "singleton.hpp"
#include <deque>
#include <vector>
#include <functional>
#include <mutex>
#include <optional>

template <typename T>
class Registry : public Singleton<Registry<T>>
{
    friend class Singleton<Registry<T>>;

public:
    void add(const T& obj);
    void add(const std::vector<T>& vec);
    
    void remove(const std::function<bool(const T&)>& predicate);
    void clear();

    T* get(size_t index);
    T* find(const std::function<bool(const T&)>& predicate);
    size_t size() const;

    auto begin();
    auto end();
    auto begin() const;
    auto end() const;

private:
    mutable std::mutex _mutex;
    std::deque<T> _objects;
};

#include "registry.tpp"