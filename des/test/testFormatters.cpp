#include "Collections/LinkedList.hpp"
#include "Event.hpp"
#include "LogEngine.hpp"
#include <fmt/core.h>
#include <gtest/gtest.h>

TEST(TestFormatter, test_event_format)
{
    Event e{"A", END, 10, 100, 20, 20, 0};
    auto str = makeformat("{}", e);
    fmt::print("{}", str);
}

TEST(TestFormatter, test_list_format)
{
    DoubleLinkedList<double> linkedList{};
    linkedList.Enqueue(1.0);
    linkedList.Enqueue(2.0);
    linkedList.Enqueue(3.0);
    linkedList.Enqueue(4.0);
    auto str = makeformat("{}", linkedList);
    fmt::print("{}", str);
}