#pragma once
#include "ISimulator.hpp"
#include "LogEngine.hpp"
#include "Shell/SimulationShell.hpp"
#include "Station.hpp"
#include <cstring>
#include <functional>
#include <map>
#include <sstream>
#include <string>
#include <thread>
#include <vector>

class SimulationShell
{

  private:
    TraceSource _logger{"Shell", 4};
    friend class Command;
    struct Command
    {
        const char *command;
        std::function<void(SimulationShell *, const char *)> _fnc;
        SimulationShell *_shell;
        template <typename F>
        Command(const char *command, SimulationShell *shell, F &&fnc) : _fnc(fnc), command(command), _shell(shell)
        {
        }
        virtual void operator()(const char *context);
        bool IsCommandFor(const char *context);
    };
    IScheduler *_scheduler;
    ISimulator *_simulator;
    bool _started = false;
    bool _end = false;
    std::thread _executionThread;
    std::vector<Command> _cmds;

  public:
    SimulationShell()
    {
    }
    void SetControllers(IScheduler *scheduler, ISimulator *simulator)
    {
        _scheduler = scheduler;
        _simulator = simulator;
    }
    ISimulator *Simulator()
    {
        return _simulator;
    }
    IScheduler *Scheduler()
    {
        return _scheduler;
    }
    const std::vector<Command> &Cmds()
    {
        return _cmds;
    }
    void Start();
    void Pause();
    void SetupDefaultCmds();
    void Interactive();
    void ShowLog(bool show);
    void ExecuteCommand(const char *command);
    static SimulationShell &Instance()
    {
        static SimulationShell instance{};
        return instance;
    }
    TraceSource *Log()
    {
        return &_logger;
    }
    template <typename F> void AddCommand(const char *command, F &&fnc)
    {
        struct Command cmd(command, this, fnc);
        _cmds.push_back(cmd);
    }
    void RemoveCommand(const char *command)
    {
        std::erase_if(_cmds, [command](Command &cmd) { return strcmp(cmd.command, command) == 0; });
    }
    void ClearCommands()
    {
        _cmds.clear();
    }
};

namespace shell::detail
{
struct _Auto_Command
{
    const char *name;
    _Auto_Command(const char *name) : name(name)
    {
        SimulationShell::Instance().AddCommand(name, [this](auto s, auto c){(*this)(s,c);});
    }
    void operator()(SimulationShell *shell, const char *context)
    {
        Execution(shell, context);
    }
    virtual void Execution(SimulationShell *shell, const char *context) = 0;
};
} // namespace shell::detail

#define ShellCommand(name)                                                                                             \
    struct __ShellCmd__##name : public shell::detail::_Auto_Command                                                    \
    {                                                                                                                  \
        __ShellCmd__##name(const char *cmdname) : _Auto_Command(cmdname)                                               \
        {                                                                                                              \
        }                                                                                                              \
        virtual void Execution(SimulationShell *shell, const char *context) override;                                  \
    };                                                                                                                 \
    __ShellCmd__##name __cmd__##name = __ShellCmd__##name{#name};                                                      \
    void __ShellCmd__##name::Execution(SimulationShell *shell, const char *context)

#pragma once