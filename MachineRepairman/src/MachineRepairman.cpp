#include "MachineRepairman.hpp"
#include "Event.hpp"
#include "FCFSStation.hpp"
#include "LogEngine.hpp"
#include "RepairStation.hpp"
#include "Station.hpp"
#include "ToString.hpp"
#include <gtest/gtest.h>
#include <vector>

void MachineRepairman::Initialize()
{
    double arrival = _interArrivalVariable();
    double service = _serviceVariable();
    Schedule(new Event(makeformat("J{}S1", 0), EventType::ARRIVAL, _clock, arrival, service, arrival));
    if (_endTime != 0.0)
        Schedule(new Event(makeformat("END"), EventType::END, _clock, _endTime, 0, 0));
    else
        _autoEnd = true;
}

void MachineRepairman::Execute()
{
    bool isEnd = false;
    while (!isEnd)
    {
        _logger->TraceTransfer("EventList:{}\n", _eventList.ToString());
        auto nextEvt = &_eventList.Dequeue();
        auto type = nextEvt->Type;
        _repairStation->Process(nextEvt);
        _clock = nextEvt->OccurTime;
        if (_autoEnd && (_repairStation->arrivals() - _repairStation->completions()) <= 0 && _clock > 20000)
        {
            Schedule(new Event("END", EventType::END, _clock, _clock, 0, _clock));
        }
        if (type == EventType::ARRIVAL)
        {

            double interArrival = _interArrivalVariable();
            double arrival = interArrival + _clock;
            Schedule(new Event(makeformat("J{}S1", Event::GeneratedNodes), EventType::ARRIVAL, _clock, arrival,
                               0, arrival));
        }
        else if (type == EventType::DEPARTURE)
        {
            delete nextEvt;
        }
        else if (type == EventType::END)
        {
            isEnd = true;
            Report();
        }
    }
}

void MachineRepairman::Report()
{
    auto repstats = _repairStation->GetStatistics();
    _logger->TraceResult("RepairStation Statistics\n{}", repstats.ToString());
    _logger->TraceResult("Last Clock:{}\nEvent Generated:{}\nEvent Deleted:{}", _clock, Event::GeneratedNodes,
                         Event::DeletedNodes);
}

void MachineRepairman::Reset()
{
    _clock = 0.0;
    _repairStation->Reset();
    _eventList.Clear();
    if (Event::DeletedNodes != Event::GeneratedNodes)
        _logger->TraceException("Error Memory leak detected {} events not deleted",
                                Event::GeneratedNodes - Event::DeletedNodes);
    Initialize();
}

std::vector<StationStatistic> MachineRepairman::GetStats() const
{
    return *new std::vector<StationStatistic>({_repairStation->GetStatistics()});
}

void MachineRepairman::Schedule(Event *event)
{
    _eventList.Insert(event, [](const Event &a, const Event &b)
    { return a.OccurTime > b.OccurTime; });
}

MachineRepairman::MachineRepairman(ILogEngine *logger, IGenerator * stream, double serviceLambda,
                                   double interarrivalLambda, double endTime)
        : _repairStation(new RepairStation(logger, this, 1,stream)),  _logger(logger), _endTime(endTime),
          _interArrivalVariable(*new NegExpVariable(interarrivalLambda,stream)), _serviceVariable(*new NegExpVariable(serviceLambda,stream))
{
    Initialize();
    _repairStation->Initialize();
}
