#pragma once
#include <vector>
#include <utility>

template <typename T>
class Pool
{
public:
    class Object
    {
    public:
        Object() = default;
        Object(const Object&) = delete;
        Object(Object&& other) noexcept;
        ~Object();

        T* operator->();

    private:
        Pool<T>* _pool{};
        size_t _index{};
        friend class Pool<T>;
        Object(Pool<T>* pool, size_t index) : _pool(pool), _index(index) {}
    };

    ~Pool();

    void resize(const size_t numberOfObjectStored);

    template<typename... TArgs>
    Object acquire(TArgs&&... args);

private:
    struct InternalObject
    {
        T* _ptr{};
        bool _inUse{};
    };
    std::vector<InternalObject> _objects;
};
#include "pool.tpp"