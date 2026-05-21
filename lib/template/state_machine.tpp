template <typename TState>
void StateMachine<TState>::addState(const TState& state)
{
    _states.insert(state);
}

template <typename TState>
void StateMachine<TState>::addTransition(const TState& startState, const TState& finalState, const std::function<void()>& lambda)
{
    if (_states.find(startState) == _states.end() || _states.find(finalState) == _states.end())
        throw std::invalid_argument("State not registered");

    _transitions[startState].push_back({finalState, lambda});
}

template <typename TState>
void StateMachine<TState>::addAction(const TState& state, const std::function<void()>& lambda)
{
    if (_states.find(state) == _states.end())
        throw std::invalid_argument("State not registered");

    _actions[state] = lambda;
}

template <typename TState>
void StateMachine<TState>::transitionTo(const TState& state)
{
    auto it = _transitions.find(_currentState);
    if (it == _transitions.end())
        throw std::invalid_argument("Transition not allowed");

    for (const auto& transition : it->second)
    {
        if (transition.first == state)
        {
            if (transition.second)
                transition.second();

            _currentState = state;

            return;
        }
    }

    throw std::invalid_argument("Transition not allowed");
}

template <typename TState>
void StateMachine<TState>::update()
{
    auto it = _actions.find(_currentState);
    if (it != _actions.end())
        it->second();
}