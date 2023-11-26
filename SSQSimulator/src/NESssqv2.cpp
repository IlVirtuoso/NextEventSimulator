#include "NESssqv2.hpp"
#include "DataWriter.hpp"
#include "Event.hpp"
#include "LogEngine.hpp"
#include "Scheduler.hpp"
#include "Station.hpp"
#include "rngs.hpp"
#include "rvgs.h"
#include "rvms.h"
#include <cstdint>
#include <ostream>
#include <string>

NESssq::NESssq()
    : Scheduler("scheduler"), _serviceTimes(VariableStream{2, [](auto generator) { return Exponential(1 / 0.14); }}),
      _interArrivals(VariableStream{1, [](RandomStream generator) { return Exponential(1 / 0.1); }})
{
    Scheduler::_stations.push_back(sptr<Station>(new FCFSStation{this, "server", 1}));
}

void NESssq::Initialize()
{
    RandomStream::Global().PlantSeeds(123456789);
}

void NESssq::Execute()
{
    while (!_end)
    {
        auto nextEvt = Create(_interArrivals(), _serviceTimes());
        _logger.Information("Created next event {} occur time {}", nextEvt.Name, nextEvt.OccurTime);
        Schedule(nextEvt);
        auto inProcess = _eventList.Dequeue();
        Process(inProcess);
        if (_end)
            return;
        inProcess.Station = 1;
        _clock = inProcess.OccurTime;
        Route(inProcess);
        if ((int)_clock % 10000 == 0)
        {
            DataWriter::Instance().SnapShot();
        }
    }
}

void NESssq::Reset()
{
    Station::Reset();
}

void NESssq::ProcessEnd(Event &evt)
{
    _end = true;
    _logger.Result("End of simulation time {}", _clock);
    _logger.Result("Arrived customers:{}", _stations.at(0)->arrivals());
    _logger.Result("Departures:{}", _stations.at(0)->completions());
}
