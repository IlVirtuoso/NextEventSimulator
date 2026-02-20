#include "Strategies/RegenerationPoint.hpp"

RegenerationPoint::RegenerationPoint(IScheduler *sched, ISimulator *simulator) : simulator(simulator), scheduler(sched)
{
}

void RegenerationPoint::Trigger()
{
    _called++;
    if (_rulesEnabled)
    {
        for (auto r : _rules)
        {
            if (!r(this))
            {
                return;
            }
        }
    }
    for (auto a : _actions)
    {
        a(this);
    }
    for (auto a : _onTimeActions)
    {
        a(this);
    }
    _onTimeActions.clear();
    _hitted++;
}