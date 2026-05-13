template <typename T>
void ThreadSafeQueue<T>::push_back(const T& newElement)
{
    std::lock_guard<std::mutex> lock(_mutex);
    _queue.push_back(newElement);
}

template <typename T>
void ThreadSafeQueue<T>::push_front(const T& newElement)
{
    std::lock_guard<std::mutex> lock(_mutex);
    _queue.push_front(newElement);
}

template <typename T>
T ThreadSafeQueue<T>::pop_back()
{
    std::lock_guard<std::mutex> lock(_mutex);
    if (_queue.empty()) throw std::runtime_error("Queue is empty");
    T element = std::move(_queue.back());
    _queue.pop_back();
    return element;
}

template <typename T>
T ThreadSafeQueue<T>::pop_front()
{
    std::lock_guard<std::mutex> lock(_mutex);
    if (_queue.empty()) throw std::runtime_error("Queue is empty");
    T element = std::move(_queue.front());
    _queue.pop_front();
    return element;
}

template <typename T>
bool ThreadSafeQueue<T>::empty() const
{
    std::lock_guard<std::mutex> lock(_mutex);
    return _queue.empty();
}

template <typename T>
size_t ThreadSafeQueue<T>::size() const
{
    std::lock_guard<std::mutex> lock(_mutex);
    return _queue.size();
}