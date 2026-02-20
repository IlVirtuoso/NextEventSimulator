//
// Created by drfaust on 03/02/23.
//

#include "Measure.hpp"
#include "Core.hpp"
#include "rvms.h"
#include <cmath>
#include <numeric>
#include <vector>

std::vector<double> slice(std::vector<double>::iterator begin, std::vector<double>::iterator end)
{
    std::vector<double> res{};
    auto itr = begin;
    while (itr != end)
    {
        res.push_back(*itr);
    }
    return res;
}

void CovariatedMeasure::Accumulate(double value, double time)
{
    _count++;
    _current[0] = value;
    _current[1] = time;
    _sum[0] += value;
    _sum[1] += value * value;
    _times[0] += time;
    _times[1] += time * time;
    _weightedsum += (value * time);
}

double CovariatedMeasure::R() const
{
    return _sum[0] / _times[0];
}

double CovariatedMeasure::variance() const
{

    auto a = _sum[1] - 2 * R() * _weightedsum + pow(R(), 2) * _times[1];
    return a * (1.0 / (_count - 1));
}

Interval CovariatedMeasure::confidence() const
{
    double alpha = 1 - _confidence;
    double r = _sum[0] / _times[0];
    double a = sqrt(((double)_count / (_count - 1.0)));
    double b = (sqrt(_sum[1] - (2 * R() * _weightedsum) + (pow(R(), 2) * _times[1])));
    double delta = a * (b / _times[0]);
    double t = idfNormal(0, 1, 1 - (alpha / 2));
    return Interval(R(), delta * t);
}

int CovariatedMeasure::SampleNeedsForPrecision()
{
    auto alpha = 1 - _confidence;
    auto t = idfNormal(0, 1, 1 - (alpha / 2));
    auto a = t * variance();
    auto b = R() * _precision;
    return floor(pow(a / b, 2));
}

void MobileMeanMeasure::push(double value)
{
    _buffer[_bufferPtr] = value;
    if (_bufferPtr == _buffer.size() - 1)
    {
        double mean = 0;
        // calculate mean in _buffer

        for (auto it = _buffer.begin(); it != _buffer.end(); ++it)
        {
            mean += *it;
        }
        mean /= _buffer.size();
        // push mean in _means
        _means[_meansPtr] = mean;
        _meansPtr = (_meansPtr + 1) % _means.size();
        _bufferPtr = 0;
    }
    else
    {
        _bufferPtr++;
    }
}

double MobileMeanMeasure::delta() const
{
    double u1 = _means[(_meansPtr - 2) % _means.size()];
    double u2 = _means[(_meansPtr - 1) % _means.size()];
    if (u1 == 0 && u2 == 0)
    {
        return HUGE_VAL;
    }
    return u1 - u2;
}

MobileMeanMeasure::MobileMeanMeasure(int bufferSize, int maxMeans)
    : _means(bufferSize), _buffer(maxMeans), _bufferPtr(), _meansPtr(), BaseMeasure("mobilemean", "s")
{
}
