#pragma once
#include "DataCollector.hpp"
#include "Event.hpp"
#include "LogEngine.hpp"
#include <functional>
#include <optional>
#include <vector>

class BaseStation
{
  protected:
    virtual void ProcessArrival(Event &evt);
    virtual void ProcessDeparture(Event &evt);
    virtual void ProcessEnd(Event &evt);
    virtual void ProcessProbe(Event &evt);
    TraceSource _logger;
    std::string _name;

    int _arrivals{};
    int _completions{};
    int _sysClients{};
    int _maxClients{};
    double _busyTime{};
    double _observationPeriod{};
    double _lastArrival{};
    double _areaN{};
    double _areaS{};
    double _oldclock{};
    double _clock{};
    std::vector<std::function<void(BaseStation *, Event &)>> _onArrival;
    std::vector<std::function<void(BaseStation *, Event &)>> _onDeparture;

    std::vector<std::function<void(BaseStation *, Event &)>> _onArrivalOnce;
    std::vector<std::function<void(BaseStation *, Event &)>> _onDepartureOnce;

  public:
    BaseStation(std::string name);
    virtual void Process(Event &event);
    virtual void Reset();
    std::string Name() const
    {
        return _name;
    }

    int arrivals() const
    {
        return _arrivals;
    }

    int completions() const
    {
        return _completions;
    }

    int sysClients() const
    {
        return _sysClients;
    }
    double observation() const
    {
        return _observationPeriod;
    }

    double busyTime() const
    {
        return _busyTime;
    }

    double clock() const
    {
        return _clock;
    }

    template <typename F> void OnDeparture(F &&fnc)
    {
        _onDeparture.push_back(fnc);
    }

    template <typename F> void OnArrival(F &&fnc)
    {
        _onArrival.push_back(fnc);
    }

    template <typename F> void OnArrivalOnce(F &&fnc)
    {
        _onArrivalOnce.push_back(fnc);
    }

    template <typename F> void OnDepartureOnce(F &&fnc)
    {
        _onDepartureOnce.push_back(fnc);
    }

    double avg_interArrival() const
    {
        return _observationPeriod / _arrivals;
    }

    double avg_serviceTime() const
    {
        return _busyTime / _completions;
    }

    double avg_delay() const
    {
        return _areaS / _completions;
    }

    double avg_waiting() const
    {
        return _areaN / _completions;
    }

    double utilization() const
    {
        return _busyTime / _observationPeriod;
    }

    int max_sys_clients()
    {
        return _maxClients;
    }

    double throughput() const
    {
        return _completions / _observationPeriod;
    }

    double input_rate() const
    {
        return _arrivals / _observationPeriod;
    }

    double arrival_rate() const
    {
        return _arrivals / _observationPeriod;
    }

    double service_rate() const
    {
        return _completions / _busyTime;
    }

    double traffic() const
    {
        return _busyTime / _lastArrival;
    }

    double mean_customer_queue() const
    {
        return _areaS / _observationPeriod;
    }
    double mean_customer_service() const
    {
        return _busyTime / _completions;
    }

    double mean_customer_system() const
    {
        return _areaN / _observationPeriod;
    }

    double areaN() const
    {
        return _areaN;
    }
    double areaS() const
    {
        return _areaS;
    }
};

class Station : public BaseStation
{
  protected:

    int _stationIndex{};

  public:
    virtual void Initialize();
    virtual void Reset() override;
    Station(std::string name, int station);

    int stationIndex() const
    {
        return _stationIndex;
    }

    void ProcessProbe(Event &evt) override;
};
