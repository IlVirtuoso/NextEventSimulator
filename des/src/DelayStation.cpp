#include "Event.hpp"
#include "Scheduler.hpp"
#include "Station.hpp"
#include <DelayStation.hpp>

void DelayStation::Initialize()
{
    Station::Initialize();
    _sysClients = _numclients();
    for (int i = 0; i < _numclients(); i++)
    {
        auto evt = Event(fmt::format("{}", Event::GeneratedNodes), DEPARTURE, _clock, _delayTime(), 0, 0, 0);
        _scheduler->Schedule(evt);
    }
}

void DelayStation::ProcessArrival(Event &evt)
{
    Station::ProcessArrival(evt);
    evt.OccurTime = _delayTime() + _clock;
    evt.Type = DEPARTURE;
    evt.Station = 0;
    _scheduler->Schedule(evt);
}

void DelayStation::ProcessDeparture(Event &evt)
{
    Station::ProcessDeparture(evt);
}
