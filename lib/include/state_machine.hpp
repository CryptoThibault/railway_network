#pragma once
#include <unordered_set>
#include <unordered_map>
#include <utility>
#include <functional>
#include <stdexcept>

template <typename TState>
class StateMachine
{
public:
    void addState(const TState& state);
    void addTransition(const TState& startState, const TState& finalState, const std::function<void()>& lambda);
    void addAction(const TState& state, const std::function<void()>& lambda);
    void transitionTo(const TState& state);
    void update();

private:
    TState _currentState{};

    std::unordered_set<TState> _states;
    std::unordered_map<TState, std::pair<TState, std::function<void()>>> _transitions;
    std::unordered_map<TState, std::function<void()>> _actions;
};

#include "state_machine.tpp"