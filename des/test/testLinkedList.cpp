#include "Collections/LinkedList.hpp"
#include "LogEngine.hpp"
#include "rngs.hpp"
#include "rvgs.h"
#include <cassert>
#include <cstdio>
#include <functional>
#include <gtest/gtest.h>
#include <queue>

TEST(test_linked_list, test_enqueue)
{
    DoubleLinkedList<int> linkedList{};
    linkedList.Enqueue(1);
    linkedList.Enqueue(2);
    linkedList.Enqueue(3);

    auto itr = linkedList.begin();
    itr++;
    assert(*itr == 2);
    assert(*linkedList.begin() == 1);
    assert(*linkedList.end() == 3);

    for (int i = 1; i < 4; i++)
        assert(linkedList.Dequeue() == i);
}

TEST(test_linked_list, test_push)
{
    DoubleLinkedList<int> linkedList{};
    linkedList.Push(1);
    linkedList.Push(2);
    linkedList.Push(3);

    auto itr = linkedList.begin();
    itr++;
    assert(*itr == 2);
    assert(*linkedList.begin() == 3);
    assert(*linkedList.end() == 1);

    for (int i = 0; i < 3; i++)
        assert(linkedList.Pull() == 3 - i);
}

TEST(test_linked_list, test_insertion)
{
    DoubleLinkedList<int> linkedList{};
    std::function<bool(int, int)> comparator = [](int a, int b) { return a > b; };
    linkedList.Insert(2, comparator);
    linkedList.Insert(3, comparator);
    linkedList.Insert(1, comparator);
    linkedList.Insert(5, comparator);
    linkedList.Insert(4, comparator);

    auto itr = linkedList.begin();
    itr++;
    assert(*itr == 2);
    itr++;
    assert(*itr == 3);
    assert(*linkedList.begin() == 1);
    assert(*linkedList.end() == 5);
}

TEST(test_linked_list, test_random_comparator)
{
    DoubleLinkedList<double> linkedList{};
    std::function<bool(double, double)> comparator = [](double a, double b) { return a > b; };
    for (int i = 0; i < 1000; i++)
    {
        linkedList.Insert(Exponential(1 / 100), comparator);
    }
    double lastVal = linkedList.Dequeue();
    while (linkedList.Count() > 0)
    {
        double dequed = linkedList.Dequeue();
        assert(lastVal <= dequed);
        lastVal = dequed;
    }
}

TEST(test_linked_list, test_random_enqueuement)
{
    DoubleLinkedList<double> linkedList{};
    std::queue<double> queue{};
    for (int i = 0; i < 1000; i++)
    {
        double val = Exponential(1 / 20);
        queue.emplace(val);
        linkedList.Enqueue(val);
    }

    while (linkedList.Count() > 0)
    {
        auto v = linkedList.Dequeue();
        assert(v == queue.front());
        queue.pop();
    }

    for (int i = 0; i < 1000; i++)
    {
        double val = Exponential(1 / 20);
        queue.emplace(val);
        linkedList.Enqueue(val);
    }

    while (linkedList.Count() > 0)
    {
        auto v = linkedList.Dequeue();
        assert(v == queue.front());
        queue.pop();
    }
}

TEST(test_linked_list, test_clean)
{
    DoubleLinkedList<double> linkedList{};
    std::queue<double> queue{};
    for (int i = 0; i < 1000; i++)
    {
        double val = Exponential(1 / 20);
        queue.emplace(val);
        linkedList.Enqueue(val);
    }
    linkedList.Clear();
    assert(linkedList.Count() == 0);
}

TEST(test_linked_list, test_last)
{
    DoubleLinkedList<double> linkedList{};
    linkedList.Enqueue(1.0);
    linkedList.Enqueue(2.0);
    linkedList.Enqueue(3.0);
    linkedList.Enqueue(4.0);
    auto itr = linkedList.begin().Last([](auto &e) { return e == 3.0; });
    ASSERT_EQ(*itr, 3.0);
}

TEST(test_linked_list, test_next)
{

    DoubleLinkedList<double> linkedList{};
    linkedList.Enqueue(1.0);
    linkedList.Enqueue(2.0);
    linkedList.Enqueue(3.0);
    linkedList.Enqueue(4.0);
    auto itr = linkedList.begin().Next([](auto e) { return e > 3.0; });
    ASSERT_EQ(*itr, 4.0);
}

TEST(test_linked_list, test_any)
{
    DoubleLinkedList<double> list{};
    list.Enqueue(3);
    ASSERT_FALSE(list.Any([](const int &a) -> bool { return a == 2; }));
    ASSERT_TRUE(list.Any([](const int &a) { return a == 3; }));
}

TEST(test_linked_list, test_all)
{
    DoubleLinkedList<char> list{};
    list.Enqueue('c');
    list.Enqueue('c');
    ASSERT_TRUE(list.All([](const char &c) { return c == 'c'; }));
    list.Enqueue('a');
    ASSERT_FALSE(list.All([](const char &c) { return c == 'c'; }));
}