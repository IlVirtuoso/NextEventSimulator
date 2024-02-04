#include "CPU.hpp"
#include "Core.hpp"
#include "Enums.hpp"
#include "Event.hpp"
#include "Station.hpp"
#include "SystemParameters.hpp"
#include "Usings.hpp"
#include "rngs.hpp"
#include "rvgs.h"
#include <fmt/core.h>
#include <vector>

// burst is written this way because of this
// https://rossetti.github.io/RossettiArenaBook/app-rnrv-rvs.html#AppRNRV:subsec:MTSRV
Cpu::Cpu(IScheduler *scheduler) : Station("CPU", Stations::CPU), _scheduler(scheduler)
{
    _logger.verbosity = 4;
}

void Cpu::ProcessArrival(Event &evt)
{

    // it's a new process
    if (evt.SubType != 'E')
    {
        _logger.Transfer("New process joined: {}", evt);
        evt.SubType = 'E';
        evt.ServiceTime = Burst();
        if (_eventUnderProcess.has_value()) // there are other process in ready queue
        {
            _eventList.Push(evt);
            return;
        }
    }

    // it was in the ready queue
    else
    {
        core_assert(!_eventUnderProcess.has_value(), "Expected empty underprocess but was {}",
                    _eventUnderProcess.value());
        _logger.Transfer("Now Processing:{}, Remaining service time:{}", evt, evt.ServiceTime);
    }
    Station::ProcessArrival(evt);

    auto quantum = SystemParameters::Parameters().cpuQuantum;
    auto slice = evt.ServiceTime > quantum ? quantum : evt.ServiceTime;
    evt.Type = DEPARTURE;
    evt.OccurTime = _clock + slice;
    evt.ServiceTime -= slice;
    _eventUnderProcess.emplace(evt);
    _scheduler->Schedule(evt);
}

void Cpu::ProcessDeparture(Event &evt)
{

    static Router router(4, SystemParameters::Parameters().cpuChoice, {IO_1, IO_2, SWAP_OUT, CPU});
    // process has finished
    core_assert(_eventUnderProcess.has_value(), "Event {} in departure but no event under process", evt);
    core_assert(evt == _eventUnderProcess.value(), "Event {} scheduled for departure but other event in process {}",
                evt, _eventUnderProcess.value());
    _eventUnderProcess.reset();
    Station::ProcessDeparture(evt);

    if (evt.ServiceTime == 0)
    {
        evt.Type = ARRIVAL;
        evt.Station = router();
        evt.SubType = NO_EVENT;
        evt.OccurTime = clock();
        _logger.Debug("CPU Departure, send event to {}", _scheduler->GetStation(evt.Station).value()->Name());
        _scheduler->Schedule(evt);
    }
    else
    {
        evt.Type = ARRIVAL;
        _eventList.Enqueue(evt);
    }
    if (_eventList.Count() > 0)
    {
        auto newEvt = _eventList.Dequeue();
        newEvt.Type = ARRIVAL;
        newEvt.OccurTime = _clock;
        _scheduler->Schedule(newEvt);
    }
}

double Cpu::Burst()
{
    static VariableStream negExp(3, [](auto rng) { return Exponential(27); });
    static CompositionStream hyperExp{
        3, {0.95, 0.05}, [](auto rng) { return Exponential(10); }, [](auto rng) { return Exponential(350); }};

    switch (SystemParameters::Parameters().cpumode)
    {
    case SystemParameters::NEG_EXP:
        return negExp();
    case SystemParameters::HYPER_EXP:
        return hyperExp();
    case SystemParameters::FIXED:
        return SystemParameters::Parameters().cpuQuantum;
    default:
        return 0.0;
    };
}
