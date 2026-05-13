#pragma once
#include <map>
#include <vector>
#include <functional>

template <typename TEvent>
class Observer
{
public:
	virtual ~Observer() {}
    void subscribe(const TEvent& event, const std::function<void()>& lambda);

    void notify(const TEvent& event);

private:
    std::map<TEvent, std::vector<std::function<void()>>> _subscribers;
};

#include "observer.tpp"