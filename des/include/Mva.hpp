#pragma once
#include "Collections/Matrix.hpp"
#include <vector>

enum StationType : bool
{
    D = true,
    I = false
};

struct MVAResult
{
    Matrix<double> meanClients;
    Matrix<double> meanWaits;
    Matrix<double> throughputs;
    Matrix<double> utilizations;
    MVAResult(int mStations, int nClients);
};

std::vector<double> RouteToVisit(Matrix<double> &routings);

MVAResult MVALID(const std::vector<double> &visits, const std::vector<double> &serviceTimes,
                 const std::vector<StationType> &types, int N);