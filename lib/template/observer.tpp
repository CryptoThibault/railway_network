template <typename TEvent>
void Observer<TEvent>::subscribe(const TEvent& event, const std::function<void()>& lambda)
{
    _subscribers[event].push_back(lambda);
}

template <typename TEvent>
void Observer<TEvent>::notify(const TEvent& event)
{
    auto it = _subscribers.find(event);
    if (it != _subscribers.end())
        for (auto& lambda : it->second)
            lambda();
}