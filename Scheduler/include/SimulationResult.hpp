#pragma once
#include "Measure.hpp"
#include "MvaSolver.hpp"
#include "Shell/SimulationShell.hpp"
#include "Strategies/TaggedCustomer.hpp"
#include <fmt/core.h>
#include <map>
#include <string>
#include <vector>

struct ConfidenceHits
{
    int throughput = 0;
    int utilization = 0;
    int meanwaits = 0;
    int meanclients = 0;
    int activeTimes = 0;
    bool throughput_in = false;
    bool utilization_in = false;
    bool meanClients_in = false;
    bool meanWaits_in = false;
    bool activeTime_in=false;
    void Accumulate(bool x_in, bool u_in, bool n_in, bool w_in, bool activeTime_in);
};

auto format_as(ConfidenceHits b);

struct SimulationResult
{
    MVASolver mva{};
    std::map<std::string, ConfidenceHits> _confidenceHits{};
    std::vector<int> seeds;
    void AccumulateResult(std::map<std::string, Accumulator<>[4]> accumulators, Accumulator<> activeTime ,int seed);
    SimulationResult();
    void AddShellCommands(SimulationShell *shell);
};