#pragma once

#include "ISimulator.hpp"
#include <concepts>
#include <functional>
#include <vector>

template <typename F, typename R, typename... Args>
concept has_return_value = requires(F a, Args... args) {
    {
        a(args...)
    } -> std::same_as<R>;
};

struct RegenerationPoint
{
  private:
    int _called = 0;
    int _hitted = 0;
    bool _rulesEnabled = true;
  public:
    IScheduler *scheduler;
    ISimulator *simulator;
    std::vector<std::function<bool(RegenerationPoint *)>> _rules;
    std::vector<std::function<void(RegenerationPoint *)>> _actions;
    std::vector<std::function<void(RegenerationPoint *)>> _onTimeActions;
    RegenerationPoint(IScheduler *sched, ISimulator *simulator);

    template <typename F>
        requires(has_return_value<F, bool, RegenerationPoint *>)
    RegenerationPoint& AddRule(F &&fnc)
    {
        _rules.push_back(fnc);
        return *this;
    }

    template <typename F>
        requires(has_return_value<F, void, RegenerationPoint *>)
    void AddAction(F &&fnc)
    {
        _actions.push_back(fnc);
    }

    template <typename F>
        requires(has_return_value<F, void, RegenerationPoint *>)
    void AddOneTimeAction(F &&fnc)
    {
        _onTimeActions.push_back(fnc);
    }

    void Trigger();
    void operator()()
    {
        Trigger();
    }

    int hitted() const
    {
        return _hitted;
    }
    int called() const
    {
        return _called;
    }
    void Reset()
    {
        _rules.clear();
        _actions.clear();
        _hitted = 0;
        _called = 0;
    }
    void SetRules(bool enable){_rulesEnabled = enable;}
};