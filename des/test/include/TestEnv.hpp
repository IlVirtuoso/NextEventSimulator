#pragma once
#include "Event.hpp"
#include "Scheduler.hpp"
#include "Usings.hpp"
#include <fmt/core.h>
struct MockScheduler : public Scheduler
{
    void ProcessNext()
    {
        auto evt = _eventList.Dequeue();
        Route(evt);
    }

    Event GenEvent(double serviceTime, double occurTime, EventType type)
    {
        return Event(fmt::format("{}", Event::GeneratedNodes), type, _clock, occurTime, serviceTime, _clock, 0);
    }

    MockScheduler() : Scheduler("mockScheduler")
    {
    }
};
