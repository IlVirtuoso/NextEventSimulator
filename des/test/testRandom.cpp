#include "Measure.hpp"
#include "rngs.hpp"
#include "rvgs.h"
#include <cmath>
#include <cstdio>
#include <fmt/core.h>
#include <fmt/format.h>
#include <fmt/ranges.h>
#include <gtest/gtest.h>
#include <stdio.h>
#include <vector>

double mean(std::vector<double> &vals)
{
    double sum = vals[0];
    for (int i = 1; i < vals.size(); i++)
        sum += vals[i];
    return sum / vals.size();
}

TEST(TestRandom, test_poisson)
{
    VariableStream stream{0, [](RandomStream generator) { return Exponential(1 / 0.14); }};
    std::vector<double> values{};
    for (int i = 0; i < 1000; i++)
    {
        values.push_back(stream());
    }
    ASSERT_GE(0.1, abs(mean(values) - 1 / 0.14));
}

// https://rossetti.github.io/RossettiArenaBook/app-rnrv-rvs.html#AppRNRV:subsec:MTSRV
TEST(TestRandom, test_composition)
{
    RandomStream::Global().PlantSeeds(123456789);
    CompositionStream stream{0,
                             {0.5, 0.5},
                             [](RandomStream &rng) { return Exponential(100); },
                             [](RandomStream &rng) { return Exponential(200); }};
    Accumulator<> _mean{"mean", ""};
    for (int i = 0; i < 100000; i++)
    {
        _mean(stream());
    }
    printf("Mean: %lf", _mean.mean());
}

TEST(TestRandom, test_routing)
{
    RandomStream::Global().PlantSeeds(123456789);
    Router router(2, {0.33, 0.33, 0.34}, {0, 1, 2});
    std::vector<double> hits{0, 0, 0};
    for (int i = 0; i < 1000000; i++)
        hits[router()]++;
    hits.push_back(hits[0] / 1000000);
    hits.push_back(hits[1] / 1000000);
    hits.push_back(hits[2] / 1000000);
    fmt::print("Hits: {}", fmt::join(hits, ","));
}

TEST(TestRandom, test_measure_esemble)
{
    int seed = 123456789;
    EsembledMeasure<> measure{"someMeasure", ""};
    for (int i = 0; i < 100; i++)
    {
        for (int k = 0; k < 120; k++)
        {
            measure(Exponential(20));
        }
        measure.MoveEsemble(1);
        measure[0].Reset();
    }
    fmt::print("{}", measure);
}

TEST(TestRandom, test_covariated)
{
    int seed = 123456789;
    CovariatedMeasure m{};
    m(3,1);
    m(2,2);
    m(3,1);
    ASSERT_EQ(3, m.variance());
    ASSERT_EQ(2, m.R());
    
    fmt::println("{:csv}",m);
}