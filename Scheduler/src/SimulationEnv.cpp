#include "SimulationEnv.hpp"
#include "Core.hpp"
#include "Event.hpp"
#include "ISimulator.hpp"
#include "Measure.hpp"
#include "MvaSolver.hpp"
#include "OperativeSystem.hpp"
#include "Shell/SimulationShell.hpp"
#include "Strategies/RegenerationPoint.hpp"
#include "SystemParameters.hpp"
#include "Usings.hpp"
#include "rngs.hpp"
#include <algorithm>
#include <cmath>
#include <condition_variable>
#include <cstdlib>
#include <cstring>
#include <fmt/core.h>
#include <fmt/format.h>
#include <map>
#include <memory>
#include <sstream>
#include <string.h>
#include <string>
#include <vector>

void SimulationManager::AddStationToCollectibles(std::string name)
{
    using namespace fmt;
    _acc[name][0] = Accumulator<>{format("throughput", name), "j/s"}.WithConfidence(0.90);
    _acc[name][1] = Accumulator<>{format("utilization", name), ""}.WithConfidence(0.90);
    _acc[name][2] = Accumulator<>{format("meanclients", name), ""}.WithConfidence(0.90);
    _acc[name][3] = Accumulator<>{format("meanwaits", name), "ms"}.WithConfidence(0.90);
    _collectFunctions.push_back([name, this] {
        auto station = os->GetStation(name).value();
        station->Update();
        _acc[name][0](station->throughput());
        _acc[name][1](station->utilization());
        _acc[name][2](station->mean_customer_system());
        _acc[name][3](station->avg_waiting() + station->avg_delay());
    });
}

void SimulationManager::CollectMeasures()
{
    for (auto f : _collectFunctions)
    {
        f();
    }
}

void SimulationManager::SetupScenario(std::string name)
{
    BaseScenario *s = nullptr;
    for (auto rs : _scenarios)
    {
        if (rs->name == name)
        {
            s = rs;
        }
    }
    if (s == nullptr)
    {
        logger.Exception("Cannot find scenario with name {}", name);
        return;
    }
    regPoint->Reset();
    HReset();
    s->Setup(this);
    regPoint->AddAction([this](RegenerationPoint *point) {
        // os->Sync();
        CollectMeasures();
        point->scheduler->Reset();
    });
    regPoint->scheduler = os.get();
    regPoint->simulator = os.get();
    tgt.WithRegPoint(regPoint.get());
    tgt.ConnectEntrance(os->GetStation("SWAP_IN").value().get(), false);
    tgt.ConnectLeave(os->GetStation("SWAP_OUT").value().get(), true);
}

void SimulationManager::HReset()
{
    os = std::unique_ptr<OS>(new OS());
    shell->SetControllers(os.get(), os.get());
    ResetAccumulators();
}

SimulationManager::SimulationManager()
{
    os = std::unique_ptr<OS>(new OS());
    regPoint = std::unique_ptr<RegenerationPoint>(new RegenerationPoint(os.get(), os.get()));
    SetupEnvironment();
}

void SimulationManager::SetupShell(SimulationShell *shell)
{
    this->shell = shell;
    SetupScenario("Default");
    shell->AddCommand("lmeasures", [this](SimulationShell *shell, auto c) {
        for (auto s : _acc)
        {
            std::string result = "";
            for (int i = 0; i < 4; i++)
            {
                auto expected =   results.mva.ExpectedForAccumulator(s.first, s.second[i]);
                result += fmt::format("{}, Expected Value:{}, Diff from conf interval:{}\n", s.second[i],
                                    expected,s.second[i].confidence().tvalDiff(expected));
            }
            shell->Log()->Result("Station:{}\n{}", s.first, result);
        }
    });
    shell->AddCommand("hreset", [this](SimulationShell *shell, auto ctx) mutable { HReset(); });
    shell->AddCommand("exit", [](auto s, auto c) { exit(0); });
    shell->AddCommand("regstats", [this](SimulationShell *s, auto c) {
        logger.Information("Regeneration point -> Hitted:{}, Called:{}", regPoint->hitted(), regPoint->called());
    });
    SystemParameters::Parameters().AddControlCommands(shell);

    shell->AddCommand("lstats", [this](auto s, auto c) {
        char buffer[32]{};
        std::istringstream read{c};
        read >> buffer;
        if (strlen(buffer) == 0)
        {
            for (auto s : os->GetStations())
            {
                os->Sync();
                s->Update();
                logger.Result("S:{},B:{},O:{},A:{},S:{},N:{},W:{}", s->Name(), s->busyTime(), s->observation(),
                              s->arrivals(), s->completions(), s->sysClients(), s->avg_waiting());
            }
        }
        else
        {
            std::string statName{buffer};
            auto ref = os->GetStation(statName);
            if (!ref.has_value())
            {
                logger.Exception("Station : {} not found", statName);
                return;
            }
            auto stat = ref.value();
            logger.Result("Station:{} \n Arrivals:{}\n Completions:{} \n Throughput:{} \n Utilization:{} \n "
                          "Avg_serviceTime:{} \n "
                          "Avg_Delay:{}\n Avg_interarrival:{} \n Avg_waiting:{} \n ",
                          stat->Name(), stat->arrivals(), stat->completions(), stat->throughput(), stat->utilization(),
                          stat->avg_serviceTime(), stat->avg_delay(), stat->avg_interArrival(), stat->avg_waiting());
        }
    });
    tgt.AddShellCommands(shell);
    shell->AddCommand("lqueue", [this](auto s, auto c) {
        logger.Result("Scheduler Queue:{}", os->EventQueue());
        for (auto s : os->GetStations())
        {
            auto p = std::dynamic_pointer_cast<IQueueHolder>(s);
            if (p != nullptr)
            {
                logger.Result("Queue {}: {}", s->Name(), p->GetEventList());
            }
        }
    });
    shell->AddCommand("scenario", [this](SimulationShell *shell, const char *ctx) {
        char buffer[64]{};
        std::stringstream read(ctx);
        read >> buffer;
        if (strcmp(buffer, "list") == 0)
        {
            for (auto s : _scenarios)
            {
                fmt::println("Scenario: {}", s->name);
            }
        }
        else
        {
            std::string sc{buffer};
            SetupScenario(sc);
            fmt::println("Setup for scenario:{} completed", sc);
        }
    });
    shell->AddCommand("activetimes", [this](auto s, auto ctx) {
        double sum = os->GetStation("CPU").value()->avg_waiting() + os->GetStation("IO1").value()->avg_waiting() +
                     os->GetStation("IO2").value()->avg_waiting();
        logger.Result("Sum of active times {}", sum);
    });

    shell->AddCommand("nr", [this](SimulationShell *shell, auto ctx) {
        char buffer[12]{};
        std::stringstream stream{ctx};
        stream >> buffer;
        int m = 1;
        if (strlen(buffer) > 0)
            m = atoi(buffer);
        CollectSamples(m);
    });

    auto l = [this](SimulationShell *shell, const char *context, bool arrival) {
        char buffer[36]{};
        std::stringstream read{context};
        read >> buffer;
        if (strlen(buffer) == 0)
            shell->Log()->Exception("Must select a station to let the event arrive");
        std::string stat{buffer};
        memset(buffer, 0, sizeof(buffer));
        read >> buffer;
        bool end = false;
        if (strlen(buffer) == 0)
        {
            // first event to arrive
            if (arrival)
                os->GetStation(stat).value()->OnArrivalOnce([&end](auto s, auto e) { end = true; });
            else
                os->GetStation(stat).value()->OnDepartureOnce([&end](auto s, auto e) { end = true; });
        }
        else
        {
            std::string capName{buffer};
            if (arrival)
                os->GetStation(stat).value()->OnArrivalOnce([&end, capName](auto s, Event &e) {
                    if (e.Name == capName)
                    {
                        end = true;
                    }
                });
            else
                os->GetStation(stat).value()->OnDepartureOnce([&end, capName](auto s, Event &e) {
                    if (e.Name == capName)
                    {
                        end = true;
                    }
                });
        }
        while (!end)
        {
            os->Execute();
        }
        logger.Information("{} of event detected", arrival ? "Arrival" : "Departure");
    };

    shell->AddCommand("na", [this, l](SimulationShell *shell, const char *context) { l(shell, context, true); });
    shell->AddCommand("nd", [l](auto s, auto ctx) { l(s, ctx, false); });
    shell->AddCommand("ns", [this](SimulationShell *shell, const char *ctx) {
        char buffer[36]{};
        std::stringstream stream{ctx};
        stream >> buffer;
        if (sizeof(buffer) == 0)
        {
            shell->Log()->Exception("Must specify a starter seed to init");
            return;
        }

        long seed = atol(buffer);
        memset(buffer, 0, sizeof(buffer));
        stream >> buffer;
        if (strlen(buffer) == 0)
        {
            shell->Log()->Exception("Must specify a number of simulation (-1 for use a seq stopping rule)");
            return;
        }

        int nsim = atoi(buffer);
        memset(buffer, 0, sizeof(buffer));

        stream >> buffer;
        if (strlen(buffer) == 0)
        {
            shell->Log()->Exception("Must specify a number of samples to collect");
            return;
        }
        int samples = atoi(buffer);
        for (int i = 0; i < nsim; i++)
        {
            RandomStream::Global().PlantSeeds(seed);
            CollectSamples(samples);
            results.AccumulateResult(_acc, tgt._meanTimes, seed);
            seed++;
            ResetAccumulators();
            tgt._meanTimes.Reset();
            shell->Log()->Debug("End of simulation {}", i);
        }
    });
    results.AddShellCommands(shell);
};

void SimulationManager::SetupEnvironment()
{
    AddStationToCollectibles("CPU");
    AddStationToCollectibles("IO1");
    AddStationToCollectibles("IO2");
    AddStationToCollectibles("SWAP_IN");
    AddStationToCollectibles("SWAP_OUT");
}
void SimulationManager::ResetAccumulators()
{
    for (auto a : _acc)
    {
        for (auto m : a.second)
        {
            m.Reset();
        }
    }
}

void SimulationManager::CollectSamples(int samples)
{
    auto p = [this](int i) {
        bool end = false;
        regPoint->AddOneTimeAction([&end](auto regPoint) { end = true; });
        while (!end)
        {
            os->Execute();
        }
        logger.Information("Collected {} samples (RegPoint end)", i);
    };
    if (samples == -1)
    {
        int i = 0;
        while (!AreAccReady(0.005) || i < 40)
        {
            p(i);
            i++;
        }
    }
    for (int i = 0; i < samples; i++)
    {
        p(i);
    }
}

bool SimulationManager::AreAccReady(double precision)
{
    for (auto s : _acc)
    {
        for (int i = 0; i < 4; i++)
            if (s.second[i].confidence().precision() > precision)
                return false;
    }
    return true;
}
