#include "Event.hpp"
#include "ISimulator.hpp"
#include "Station.hpp"
#include <functional>

struct DelayStation : public Station
{
  protected:
    IScheduler *_scheduler;
    std::function<double()> _delayTime;
    std::function<int()> _numclients;
  public:
    template <typename RDouble, typename RInt>
    DelayStation(IScheduler *scheduler, std::string name, RDouble &&delayFnc, RInt && numclients)
        : Station(name, 0), _scheduler(scheduler), _delayTime(delayFnc), _numclients(numclients)
    {
    }
    void Initialize() override;
    void ProcessArrival(Event &evt) override;
    void ProcessDeparture(Event &evt) override;
};