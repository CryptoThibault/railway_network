template <typename T>
void Registry<T>::add(T obj)
{
    std::lock_guard<std::mutex> lock(_mutex);
    _objects.push_back(std::move(obj));
}

template <typename T>
void Registry<T>::add(const std::vector<T>& vec)
{
    std::lock_guard<std::mutex> lock(_mutex);
    _objects.insert(_objects.end(), vec.begin(), vec.end());
}

template <typename T>
void Registry<T>::remove(const std::function<bool(const T&)>& predicate)
{
    std::lock_guard<std::mutex> lock(_mutex);
    _objects.erase(std::remove_if(_objects.begin(), _objects.end(), predicate), _objects.end());
}

template <typename T>
void Registry<T>::clear()
{
    std::lock_guard<std::mutex> lock(_mutex);
    _objects.clear();
}

template <typename T>
T* Registry<T>::get(size_t index)
{
    std::lock_guard<std::mutex> lock(_mutex);
    if (index >= _objects.size()) return nullptr;
    return &_objects[index];
}

template <typename T>
std::vector<T> Registry<T>::getAll() const
{
    std::lock_guard<std::mutex> lock(_mutex);
    return _objects;
}

template <typename T>
T* Registry<T>::find(const std::function<bool(const T&)>& predicate)
{
    std::lock_guard<std::mutex> lock(_mutex);
    auto it = std::find_if(_objects.begin(), _objects.end(), predicate);
    return it != _objects.end() ? &(*it) : nullptr;
}

template <typename T>
size_t Registry<T>::size() const
{
    std::lock_guard<std::mutex> lock(_mutex);
    return _objects.size();
}