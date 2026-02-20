#include "Shell/SimulationShell.hpp"
#include "LogEngine.hpp"
#include "rngs.hpp"
#include <algorithm>
#include <any>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <ctype.h>
#include <fmt/core.h>
#include <fmt/printf.h>
#include <iostream>
#include <sstream>
#include <stdlib.h>
#include <thread>
#include <tuple>
#include <variant>

using namespace fmt;

void SimulationShell::Start()
{
    if (_started)
        return;
    _executionThread = std::thread{[this]() {
        _end = false;
        _started = true;
        while (!_end)
        {
            _simulator->Execute();
        }
    }};

    _executionThread.detach();
}

void SimulationShell::Pause()
{
    _end = true;
    _started = false;
}

void SimulationShell::Interactive()
{
    bool execend = false;
    while (!execend)
    {
        print("<SimulationShell> ");
        char buffer[256]{};
        std::cin.getline(buffer, 256);
        ExecuteCommand(buffer);
    }
}

void SimulationShell::ExecuteCommand(const char *command)
{
    char buffer[36]{};
    char args[128]{};
    std::istringstream ctx{command};
    ctx >> buffer;
    ctx.get(args, 128);
    for (auto c : _cmds)
    {
        if (c.IsCommandFor(buffer))
        {
            c(args);
            return;
        }
    }
    _logger.Exception("Command {} not found", command);
}

bool SimulationShell::Command::IsCommandFor(const char *context)
{
    char buffer[36]{};
    std::stringstream ctx{context};
    ctx.get(buffer, 36, ' ');
    auto size = strlen(command);
    if (strlen(buffer) != size)
        return false;
    return strcmp(buffer, command) == 0;
}

void SimulationShell::Command::operator()(const char *context)
{
    char buffer[256]{};
    std::stringstream stream{context};
    stream.ignore();
    stream.read(buffer, 256);
    _fnc(_shell, buffer);
}

ShellCommand(verbosity)
{
    char buffer[64]{};
    std::stringstream buf{context};
    buf.get(buffer, sizeof(buffer), ' ');
    if (isdigit(buffer[0]))
    {
        int verbosity = atoi(buffer);
        for (auto s : LogEngine::Instance()->GetSources())
        {
            s->verbosity = verbosity;
        }
        shell->Log()->Information("Setted all sources to {} verbosity", verbosity);
    }
    else
    {
        std::string srcName{buffer};
        if (srcName == "list")
        {
            for (auto s : LogEngine::Instance()->GetSources())
            {
                shell->Log()->Information("SourceName:{}, Verbosity:{}", s->sourceName, s->verbosity);
            }
            return;
        }
        buf.ignore();
        buf.get(buffer, sizeof(buffer), ' ');
        int verbosity = atoi(buffer);
        for (auto s : LogEngine::Instance()->GetSources())
        {
            if (s->sourceName == srcName)
            {
                s->verbosity = verbosity;
                shell->Log()->Information("Setted {} to verbosity {}", s->sourceName, verbosity);
                return;
            }
        }
        shell->Log()->Exception("SourceName {} not found", srcName);
    }
};

ShellCommand(start)
{
    shell->Start();
};

ShellCommand(stop)
{
    shell->Pause();
};

ShellCommand(init)
{
    shell->Simulator()->Initialize();
};

ShellCommand(clock)
{
    shell->Log()->Result("Clock is {}", shell->Scheduler()->GetClock());
}

ShellCommand(ne)
{
    std::stringstream stream{context};
    char arg[12];
    stream.get(arg, 12, ' ');
    int value = atoi(arg);
    if (value == 0)
        value = 1;
    for (int i = 0; i < value; i++)
        shell->Simulator()->Execute();
};

ShellCommand(help)
{
    fmt::print("List of available commands:");
    for (auto c : shell->Cmds())
    {
        fmt::println("Command :{}", c.command);
    }
};
ShellCommand(seed)
{
    char buffer[36]{};
    std::istringstream stream{context};
    stream >> buffer;
    if (strlen(buffer) == 0)
    {
        shell->Log()->Exception("Seed must be called with argument");
        return;
    }
    int seed = atoi(buffer);
    if (seed == -1)
        RandomStream::Global().SetAntitetich(true);
    else
    {
        RandomStream::Global().SetAntitetich(false);
        RandomStream::Global().PlantSeeds(seed);
    }
};