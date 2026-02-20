#include "Event.hpp"
#include "FCFSStation.hpp"
#include "LogEngine.hpp"
#include "Measure.hpp"
#include "Station.hpp"
#include "rngs.hpp"
#include "TestEnv.hpp"
#include "gtest/gtest.h"
#include <fmt/core.h>
#include <memory>

TEST(TestStation, test_onarrival)
{
    LogEngine::CreateInstance("test");
    int a = 0;
    BaseStation s{"test"};
    s.OnArrival([&a](auto s, Event &e) { a++; });
    auto evt = Event("test", ARRIVAL, 0, 0, 0, 0, 0);
    s.Process(evt);
    ASSERT_EQ(1, a);
}

TEST(TestStation, test_fcfs_arrival)
{
    LogEngine::CreateInstance("test.txt");
    MockScheduler sched{};
    auto stat = new FCFSStation(&sched, "mockStation", 0);
    sched.AddStation(stat);
    sched.Schedule(sched.GenEvent(100, 10, ARRIVAL));
    sched.Schedule(sched.GenEvent(10, 20, ARRIVAL));
    sched.Schedule(sched.GenEvent(10, 30, ARRIVAL));
    for (int i = 0; i < 3; i++)
        sched.ProcessNext();
    ASSERT_EQ(2, stat->GetEventList().Count());
}



