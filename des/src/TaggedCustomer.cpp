#include "Core.hpp"
#include "Event.hpp"
#include "Measure.hpp"
#include "Shell/SimulationShell.hpp"
#include "rngs.hpp"
#include <Strategies/TaggedCustomer.hpp>
#include <algorithm>
#include <fmt/core.h>
#include <functional>

void TaggedCustomer::ConnectEntrance(BaseStation *station, bool arrival)
{
    auto l = [this, station](auto s, Event &e) {
        if (target_client == "")
            target_client = e.Name;
        if (e.Name == target_client)
        {
            time = e.OccurTime;
            if (_onEntrance.has_value())
                _onEntrance.value()(e);
        }
    };
    if (arrival)
        station->OnArrival(l);
    else
        station->OnDeparture(l);
}

void TaggedCustomer::ConnectLeave(BaseStation *station, bool arrival)
{
    auto l = [this, station](auto s, Event &e) {
        if (e.Name == target_client)
        {
            double interval = e.OccurTime - time;
            _acc.Accumulate(interval);
            if (_onLeave.has_value())
                _onLeave.value()(e);
        }
    };
    if (arrival)
        station->OnArrival(l);
    else
        station->OnDeparture(l);
}

void TaggedCustomer::CompleteRegCycle(double actualclock)
{

        _mean(_acc.sum(), _acc.Count());
        _acc.Reset();
}

void TaggedCustomer::CompleteSimulation()
{
    _mean.Reset();
}
