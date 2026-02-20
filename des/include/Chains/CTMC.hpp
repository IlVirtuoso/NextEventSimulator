#pragma once
#include "Collections/Matrix.hpp"
#include "DTMC.hpp"

struct CTMC : public Matrix<double>
{
    void Rasterize();
    DTMC ToDtmc();
};
