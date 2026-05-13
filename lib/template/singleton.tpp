template <typename T>
T* Singleton<T>::instance()
{
    if (!_instance)
        throw std::runtime_error("Singleton not instantiated yet.");
    return _instance.get();
}

template <typename T>
template<typename ... TArgs>
void Singleton<T>::instantiate(TArgs&& ... args)
{
    std::lock_guard<std::mutex> lock(_mutex);
    if (_instance)
        throw std::runtime_error("Singleton already instantiated.");
    _instance.reset(new T(std::forward<TArgs>(args)...));
}

template <typename T>
std::unique_ptr<T> Singleton<T>::_instance{};

template <typename T>
std::mutex Singleton<T>::_mutex;