template <typename T>
Pool<T>::Object::Object(Object&& other) noexcept
    : _pool(other._pool), _index(other._index)
{
    other._pool = nullptr;
}

template <typename T>
Pool<T>::Object::~Object()
{
    if (_pool)
        _pool->_objects[_index]._inUse = false;
}

template <typename T>
T* Pool<T>::Object::operator->()
{
    return _pool->_objects[_index]._ptr;
}

template <typename T>
Pool<T>::~Pool()
{
    for (auto& obj : _objects)
        delete obj._ptr;
}

template <typename T>
void Pool<T>::resize(const size_t numberOfObjectStored)
{
    _objects.reserve(numberOfObjectStored);
    size_t currentSize = _objects.size();
    for (size_t i = currentSize; i < numberOfObjectStored; ++i)
    {
        _objects.push_back({});
        _objects.back()._ptr = new T();
    }
}

template <typename T>
template<typename... TArgs>
typename Pool<T>::Object Pool<T>::acquire(TArgs&&... args)
{
    for (size_t i = 0; i < _objects.size(); ++i)
    {
        if (!_objects[i]._inUse)
        {
            if constexpr (sizeof...(TArgs) > 0)
            {
                _objects[i]._ptr->~T();
                ::new (_objects[i]._ptr) T(std::forward<TArgs>(args)...);
            }
            _objects[i]._inUse = true;
            return Object(this, i);
        }
    }

    _objects.push_back({});
    _objects.back()._ptr = new T(std::forward<TArgs>(args)...);
    _objects.back()._inUse = true;
    return Object(this, _objects.size() - 1);
}