#pragma once
#include "DataWriter.hpp"
#include "Event.hpp"
#include "ISimulator.hpp"
#include "LogEngine.hpp"
#include "Measure.hpp"
#include "OperativeSystem.hpp"
#include "Shell/SimulationShell.hpp"
#include "Station.hpp"
#include "Strategies/RegenerationPoint.hpp"
#include "Strategies/TaggedCustomer.hpp"
#include "SystemParameters.hpp"
#include "rngs.hpp"
#include <cmath>
#include <cstdlib>
#include <fmt/core.h>
#include <functional>
#include <memory>
#include <sstream>
#include <vector>

void SetupEnvironment();
struct BaseScenario;

struct SimulationManager
{
    friend class BaseScenario;
    std::vector<BaseScenario *> _scenarios{};
    std::vector<Accumulator<double>> _acc{};
    std::vector<std::function<void()>> _collectFunctions{};
    std::unique_ptr<OS> os;
    std::unique_ptr<RegenerationPoint> regPoint;
    SimulationShell *shell;
    TaggedCustomer tgt{};
    TraceSource logger{"SIMManager", 4};
    bool hot = false;
    SimulationManager();

    static SimulationManager &Instance()
    {
        static SimulationManager manager{};
        return manager;
    }

    void AddScenario(BaseScenario *s)
    {
        _scenarios.push_back(s);
    }

    void RemoveScenario(BaseScenario *s)
    {
        std::erase_if(_scenarios, [s](auto s1) { return s == s1; });
    }

    void SetupShell(SimulationShell *shell);
    void HReset();
    void CollectMeasures();

    private:
    void AddStationToCollectibles(std::string name);
    void SetupScenario(std::string name);
    void SetupEnvironment();

};

struct BaseScenario
{
    virtual void Setup(SimulationManager* manager) = 0;
    std::string name;
    BaseScenario(std::string name): name(name)
    {
        SimulationManager::Instance().AddScenario(this);
    }
    ~BaseScenario()
    {
        SimulationManager::Instance().RemoveScenario(this);
    }
};