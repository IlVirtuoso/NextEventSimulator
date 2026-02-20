#pragma once
#include "Collections/LinkedList.hpp"
#include "Event.hpp"
#include "ISimulator.hpp"
#include "LogEngine.hpp"
#include "Station.hpp"
#include "Usings.hpp"
#include <memory>
#include <optional>
#include <vector>

class Scheduler : public IScheduler, public BaseStation
{
  protected:
    std::vector<sptr<Station>> _stations{};
    DoubleLinkedList<Event> _eventList{};
    virtual bool Route(Event event);
    size_t processedEvents = 0;

  public:
    virtual void Schedule(Event event) override;
    virtual void Initialize();

    template <typename STAT> inline Scheduler &AddStation(STAT *station)
    {
        _stations.push_back(sptr<STAT>(station));
        return *this;
    };

    Scheduler(std::string name) : BaseStation(name)
    {
    }

    std::optional<sptr<Station>> GetStation(std::string name) override
    {
        for (auto &st : _stations)
        {
            if (st->Name() == name)
            {
                return st;
            }
        }
        return {};
    }

    const DoubleLinkedList<Event> EventQueue()
    {
        return _eventList;
    }

    std::optional<sptr<Station>> GetStation(int stationIndex) override
    {
        for (auto s : _stations)
        {
            if (s->stationIndex() == stationIndex)
            {
                return s;
            }
        }
        return {};
    }

    const std::vector<sptr<Station>> GetStations() const override
    {
        return _stations;
    }

    std::optional<sptr<Station>> operator[](std::string name)
    {
        return GetStation(name);
    }

    std::optional<sptr<Station>> operator[](int index)
    {
        return GetStation(index);
    }

    void Sync() override
    {
        LogLocker l{};
        for (auto s : _stations)
        {
            auto e = Event("SYNC", PROBE, _clock, _clock, 0, _clock, s->stationIndex());
            s->Process(e);
        }
    }
    Event ProcessNext();
    bool HasEvents() const
    {
        return _eventList.Count() > 0;
    }
    double GetClock() override
    {
        return _clock;
    }

    void Reset() override;
    virtual Event Create(double interArrival, double serviceTime, int stationTarget = -1, EventType type = ARRIVAL);
};
