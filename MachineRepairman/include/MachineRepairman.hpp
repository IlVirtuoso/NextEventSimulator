#pragma once
#include "Event.hpp"
#include "ISimulator.hpp"
#include "LogEngine.hpp"
#include "Scheduler.hpp"
#include "Station.hpp"
#include "rngs.hpp"
#include <vector>

class MachineRepairman : public Scheduler, public ISimulator
{
  private:
    VariableStream _interArrival;
    VariableStream _serviceTimes;
    const int _nominalWorkshift = 480;
    const int _nominalRests = 960;

  public:
    MachineRepairman();
};