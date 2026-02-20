#pragma once
#include "Event.hpp"
#include "Measure.hpp"
#include "Shell/SimulationShell.hpp"
#include "Station.hpp"
#include "Strategies/RegenerationPoint.hpp"
#include <functional>
#include <map>
#include <optional>
#include <utility>
struct TaggedCustomer
{
    CovariatedMeasure _mean{"cycleTime", "ms"};
    Accumulator<> _acc{"regTime", "ms"};
    std::optional<std::function<void(Event &)>> _onEntrance;
    std::optional<std::function<void(Event &)>> _onLeave;
    std::string target_client = "";
    TaggedCustomer()
    {
    }

    TaggedCustomer &WithRegPoint(RegenerationPoint *regPoint)
    {
        regPoint->AddAction([this](RegenerationPoint *pt) { CompleteRegCycle(pt->scheduler->GetClock()); });
        return *this;
    }

    double time = 0;
    void ConnectEntrance(BaseStation *station, bool arrival = false);
    void ConnectLeave(BaseStation *station, bool arrival = false);
    void CompleteRegCycle(double actualClock);
    void CompleteSimulation();

    template <typename F>
        requires(has_return_value<F, void, Event &>)
    void OnEntrance(F &&fnc)
    {
        _onEntrance = fnc;
    }

    template <typename F>
        requires(has_return_value<F, void, Event &>)
    void OnLeave(F &&fnc)
    {
        _onLeave = fnc;
    }
};