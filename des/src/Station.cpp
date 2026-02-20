#include "Station.hpp"
#include "Core.hpp"
#include "DataCollector.hpp"
#include "DataWriter.hpp"
#include "Event.hpp"
#include "LogEngine.hpp"
#include "Measure.hpp"
#include "Scheduler.hpp"
#include <cassert>
#include <fmt/core.h>
#include <iterator>
#include <regex>
#include <sstream>

enum Measures
{
    Completions,
    Arrivals,
    sysclients,
    maxclients,
    avgInterArrival,
    avgServiceTime,
    avgDelay,
    avgWaiting,
    utilizations,
    throughputs,
    inputRate,
    arrivalRate,
    serviceRate,
    traffics,
    meanCustomerInQueue,
    meanCustomerInService,
    meanCustomerInSystem,
};

void BaseStation::ProcessArrival(Event &evt)
{
    _arrivals++;
    _sysClients++;
    _lastArrival = evt.OccurTime;
    if (_sysClients > _maxClients)
        _maxClients = _sysClients;
}

void BaseStation::ProcessDeparture(Event &evt)
{
    _sysClients--;
    _completions++;
}

void BaseStation::ProcessEnd(Event &evt)
{
}

void BaseStation::Reset()
{
    _arrivals = 0;
    _completions = 0;
    _maxClients = 0;
    _busyTime = 0.0;
    _observationPeriod = 0.0;
    _lastArrival = 0.0;
    _areaN = 0.0;
    _areaS = 0.0;
}

BaseStation::BaseStation(std::string name) : _logger(name), _name(name)
{
    _logger.Transfer("Station:{} constructed", name);
}

void BaseStation::ProcessProbe(Event &evt)
{
}

void Station::ProcessProbe(Event &evt)
{
    BaseStation::ProcessProbe(evt);
}

void BaseStation::Process(Event &event)
{
    core_assert(event.OccurTime >= _clock, "Event {} occur at a lesser time of {} in station {}", event, _clock, _name);
    _clock = event.OccurTime;
    _logger.Transfer("Processing:{}", event);
    double interval = _clock - _oldclock;
    _oldclock = _clock;
    if (_sysClients > 0)
    {
        _busyTime += interval;
        _areaN += _sysClients * interval;
        _areaS += (_sysClients - 1) * interval;
    }
    _observationPeriod += interval;
    switch (event.Type)
    {
    case EventType::ARRIVAL:
        _logger.Transfer("Arrival:{}", event);
        for (auto &h : _onArrival)
            h(this, event);
        for (auto &j : _onArrivalOnce)
            j(this, event);
        _onArrivalOnce.clear();
        ProcessArrival(event);
        break;
    case EventType::DEPARTURE:
        ProcessDeparture(event);
        for (auto &h : _onDeparture)
            h(this, event);
        for (auto &j : _onDepartureOnce)
            j(this, event);
        _onDepartureOnce.clear();
        _logger.Transfer("Departure:{}", event);
        break;
    case EventType::NO_EVENT:
        break;
    case EventType::END:
        _logger.Transfer("End:{}", event);
        ProcessEnd(event);
        break;
    case EventType::PROBE:
        ProcessProbe(event);
    }
}

void Station::Reset()
{
    BaseStation::Reset();
}

Station::Station(std::string name, int station) : BaseStation(name), _stationIndex(station)
{
}

void Station::Initialize()
{
}
