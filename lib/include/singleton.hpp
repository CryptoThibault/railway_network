#pragma once
#include <memory>
#include <mutex>
#include <stdexcept>

template <typename T>
class Singleton
{
public:
    static T* instance();

    template<typename ... TArgs>
    static void instantiate(TArgs&& ... args);

protected:
    Singleton() = default;
    ~Singleton() = default;

private:
    static std::unique_ptr<T> _instance;
    static std::mutex _mutex;

    Singleton(const Singleton&) = delete;
    Singleton& operator=(const Singleton&) = delete;
};

#include "singleton.tpp"