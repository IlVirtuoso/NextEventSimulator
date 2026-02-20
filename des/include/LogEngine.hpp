#pragma once

#include "FormatParser.hpp"
#include <fmt/core.h>
#include <fmt/format.h>
#include <iostream>
#include <sstream>
#include <string>
#include <type_traits>
#include <vector>

enum class LogType : int
{
    EXCEPTION,
    RESULT,
    INFORMATION,
    DEBUG,
    TRANSFER
};

template <> struct fmt::formatter<enum LogType> : formatter<string_view>
{
    auto format(const LogType &type, format_context &ctx) const -> format_context::iterator
    {
        switch (type)
        {
        case LogType::EXCEPTION:
            return fmt::format_to(ctx.out(), "Exception");
        case LogType::RESULT:
            return fmt::format_to(ctx.out(), "Result");
        case LogType::INFORMATION:
            return fmt::format_to(ctx.out(), "Information");
        case LogType::TRANSFER:
            return fmt::format_to(ctx.out(), "Transfer");
        case LogType::DEBUG:
            return fmt::format_to(ctx.out(), "Debug");
        }
        return fmt::format_to(ctx.out(), "");
    }
};

// use for dynamic format, else use fmt::format
template <typename... Args> std::string makeformat(const char *format, Args... args)
{
    return std::move(std::string(fmt::vformat(std::string(format), fmt::make_format_args(args...))));
}

struct TraceSource;

struct LogLocker;
class LogEngine
{
    friend struct TraceSource;
    friend struct LogLocker;

  private:
    bool _printStdout = true;
    bool _pauseStdout = false;
    std::stringstream _buffer;
    std::string _logFile;
    static LogEngine *_instance;
    std::vector<TraceSource *> _sources;
    LogEngine()
    {
    }
    LogEngine(std::string logFile) : _logFile{logFile}
    {
    }

  public:
    virtual void Flush();
    virtual void Trace(LogType type, std::string message);
    void AddSource(TraceSource *source)
    {
        _sources.push_back(source);
    }
    static void CreateInstance(std::string logFile)
    {
        _instance = new LogEngine(logFile);
    }

    const std::vector<TraceSource *> GetSources() const
    {
        return _sources;
    }

    void RemoveSource(TraceSource *src)
    {
        for (int i = 0; i < _sources.size(); i++)
        {
            if (_sources[i] == src)
                _sources.erase(_sources.begin() + i);
        }
    }
    static LogEngine *Instance()
    {
        return _instance;
    }

    void PrintStdout(bool value)
    {
        _printStdout = value;
    }
};

struct LogLocker
{
  private:
    LogEngine *_instance;

  public:
    LogLocker()
    {
        _instance = LogEngine::Instance();
        _instance->_pauseStdout = true;
    }

    ~LogLocker()
    {
        _instance->_pauseStdout = false;
    }
};

struct TraceSource
{
    std::string sourceName;
    int verbosity;
    LogEngine *engine = nullptr;
    TraceSource(std::string sourceName, int verbosity = 0) : sourceName(sourceName), verbosity(verbosity)
    {
    }

    void Init()
    {
        if (LogEngine::_instance != nullptr)
        {
            engine = LogEngine::_instance;
            engine->AddSource(this);
        }
        else
        {
            engine = nullptr;
        }
    }
    void Trace(LogType type, std::string message)
    {
        if (engine == nullptr || LogEngine::_instance != engine)
            Init();
        if (verbosity >= (int)type && engine != nullptr)
        {
            engine->Trace(type, fmt::format("({}){}", sourceName, message));
        }
    }

    template <typename... Args> void Exception(const char *format, Args... args)
    {
        Trace(LogType::EXCEPTION, makeformat(format, args...));
    }

    template <typename... Args> void Information(const char *format, Args... args)
    {
        Trace(LogType::INFORMATION, makeformat(format, args...));
    }

    template <typename... Args> void Transfer(const char *format, Args... args)
    {
        Trace(LogType::TRANSFER, makeformat(format, args...));
    }

    template <typename... Args> void Result(const char *format, Args... args)
    {
        Trace(LogType::RESULT, makeformat(format, args...));
    }

    template <typename... Args> void Debug(const char *format, Args... args)
    {
        Trace(LogType::DEBUG, makeformat(format, args...));
    }
    ~TraceSource()
    {
        if (engine != nullptr)
            engine->RemoveSource(this);
    }
};