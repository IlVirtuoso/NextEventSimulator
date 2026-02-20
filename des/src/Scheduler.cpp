#include "Scheduler.hpp"
#include "Core.hpp"
#include "Event.hpp"
#include "LogEngine.hpp"
#include "Station.hpp"
#include <fmt/core.h>

bool Scheduler::Route(Event evt)
{
    for (auto station : _stations)
    {
        if (evt.Station == station->stationIndex())
        {
            station->Process(evt);
            return true;
        }
    }
    return false;
}
void Scheduler::Schedule(Event event)
{
    if (event.OccurTime < _clock)
    {
        panic(fmt::format("Scheduling of event {} break the rules since clock is {}", event, _clock));
    }
    _logger.Transfer("Scheduling:{}", event);
    _eventList.Insert(event, [](const Event &a, const Event &b) { return a.OccurTime > b.OccurTime; });
}

void Scheduler::Initialize()
{
}

Event Scheduler::ProcessNext()
{
    auto evt = _eventList.Dequeue();
    Process(evt);
    Route(evt);
    return evt;
}

Event Scheduler::Create(double interArrival, double serviceTime, int stationTarget, EventType type)
{
    std::string name = fmt::format("J{}", Event::GeneratedNodes);
    return Event{name, type, _clock, _clock + interArrival, serviceTime, _clock + interArrival, stationTarget};
}

void Scheduler::Reset()
{
    for (auto &s : _stations)
        s->Reset();
}