#pragma once
#include "FormatParser.hpp"
#include <cstddef>
#include <fmt/core.h>
#include <fmt/format.h>
#include <iostream>

enum EventType : char
{
    NO_EVENT = 'N',
    ARRIVAL = 'A',
    DEPARTURE = 'D',
    END = 'E',
    PROBE = 'P',
    MAINTENANCE = 'M',
    RESET= 'R'
};

struct Event
{
    int Station = 0;
    int *copyCounter = nullptr;
    static int GeneratedNodes;
    static int DeletedNodes;
    double CreateTime = 0.0;
    double OccurTime = 0.0;
    double ServiceTime = 0.0;
    double ArrivalTime = 0.0;
    std::string Name{};
    char Type = EventType::NO_EVENT;
    char SubType = EventType::NO_EVENT;
    Event()
    {
        copyCounter = new int();
    }
    Event(std::string name, char type, double createTime, double occurTime, double serviceTime, double arrivalTime,
          int stationTarget = 0)
        : Name{name}, Type{type}, CreateTime{createTime}, OccurTime{occurTime}, ServiceTime{serviceTime},
          ArrivalTime{arrivalTime}, Station(stationTarget)
    {
        Event::GeneratedNodes++;
        copyCounter = new int();
    }
    ~Event()
    {
        if (*copyCounter > 0)
            (*copyCounter)--;
        else
            DeletedNodes++;
        Name.clear();
    }

    Event(const Event &evt)
        : Name(evt.Name), Type(evt.Type), CreateTime(evt.CreateTime), OccurTime(evt.OccurTime),
          ServiceTime(evt.ServiceTime), ArrivalTime(evt.ArrivalTime), Station(evt.Station), SubType(evt.SubType),
          copyCounter(evt.copyCounter)
    {
        (*copyCounter)++;
    }

    bool operator==(Event &oth);
};

template <> struct fmt::formatter<Event> : formatter<string_view>
{
    auto format(const Event& evt, format_context& ctx) const -> format_context::iterator
    {
        return fmt::format_to(ctx.out(), "J:{},OC:{:2f},Tp:{},Station:{}", evt.Name, evt.OccurTime, evt.Type,
                              evt.Station);
    }
};
