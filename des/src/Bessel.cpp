#include "Chains/Bessel.hpp"

#include <cmath>

double fact(int n)
{
    if (n == 0)
        return 1;
    double res = 1;
    for (int i = 2; i <= n; i++)
    {
        res *= i;
    }
    return res;
}

double Bessel(int order, int limit, int pos)
{
    auto term1 = [](int x, int m, int k) {
        double ex = k + 2 * m;
        return std::pow(x / 2, ex);
    };
    auto term2 = [](int k, int m) { return fact(k + m) * fact(m); };
    auto sum = 0.0;
    for (int m = 0; m < limit; m++)
    {
        sum += term1(pos, m, order) / term2(order, m);
    }
    return sum;
}
