#include "Chains/CTMC.hpp"
#include "Collections/Matrix.hpp"
#include "Mva.hpp"
#include <cmath>
#include <cstdio>
#include <fmt/core.h>
#include <fmt/format.h>
#include <gtest/gtest.h>
#include "Chains/DTMC.hpp"
#include <vector>

TEST(TestMatrix, test_mat_insert)
{
    std::vector<int> a{};
    Matrix<int> m{100};
    m(0, 1) = 1;
    ASSERT_EQ(1, m(0, 1));
}

TEST(TestMatrix, test_mat_instantiation)
{
    Matrix<int> a({{1, 2, 3}, {2, 3, 4}, {5, 6, 7}});
    auto p = a.ToString();
    printf("%s", p.c_str());
}

TEST(TestMatrix, test_mul)
{
    Matrix<double> a({{2, 2, 2}, {2, 2, 2}, {2, 2, 2}});
    auto b = a * a;
    printf("%s", b.ToString().c_str());
}

TEST(TestMatrix, test_left_mul)
{
    Matrix<double> a({{2, 2, 2}, {2, 2, 2}, {2, 2, 2}});
    std::vector<double> b = {2, 2, 2};
    auto c = b * a;
    ASSERT_EQ(12, c[0]);
}

TEST(TestMatrix, test_mva)
{
    // 0.100000,10.000000,5.500000,3.500000,
    Matrix<double> routings({
        {0, 1, 0, 0}, 
        {0.10, 0, 0.55, 0.35}, 
        {0, 1, 0, 0}, 
        {0, 1, 0, 0}
        });
    auto visits = RouteToVisit(routings);
    double epsilon = 0.001;
    ASSERT_LE(abs(visits[0]-1),epsilon );
    ASSERT_LE(abs(visits[1]-10.0),epsilon );
    ASSERT_LE(abs(visits[2]-5.5),epsilon );
    ASSERT_LE(abs(visits[3]-3.5),epsilon );
    auto v2 = powerMatrixMethod(routings, 100);
    for(int i = 1; i < v2.size(); i++){
        v2[i] = v2[i]*(1/v2[0]);
    }
}


TEST(TestMarkov, test_vector_method){
        Matrix<double> d({{0.80, 0.15, 0.05}, {0.70, 0.20, 0.10}, {0.50, 0.30, 0.20}});
    auto v = vectorMethod(d, 0.001);
    std::string res = "";
    for (auto &e : v)
    {
        res += makeformat("{},", e);
    }
    printf("%s", res.c_str());
}
