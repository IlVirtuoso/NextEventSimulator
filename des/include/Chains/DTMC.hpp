#pragma once
#include "Collections/Matrix.hpp"
#include <vector>
std::vector<double> vectorMethod(Matrix<double> dtmc, double threshold);
std::vector<double> powerMatrixMethod(Matrix<double> dtmc, int iterations);

struct DTMC : public Matrix<double>
{
    using Base = Matrix<double>;
    enum class Result
    {
        NO_SQUARE,
        INVALID_SUM,
        ABSORBING_STATES,
        OK
    };

    Result Validate();
};