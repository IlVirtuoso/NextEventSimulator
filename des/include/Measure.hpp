//
// Created by drfaust on 03/02/23.
//

#pragma once

#include "Core.hpp"
#include "LogEngine.hpp"
#include "Measure.hpp"
#include "rvms.h"
#include <array>
#include <cmath>
#include <cstddef>
#include <cstdio>
#include <cstring>
#include <fmt/core.h>
#include <fmt/format.h>
#include <functional>
#include <iterator>
#include <stdexcept>
#include <string>
#include <utility>
#include <valarray>
#include <vector>

struct Interval
{
    double _mean;
    double _delta;

  public:
    Interval(double mean, double delta) : _mean(mean), _delta(delta)
    {
    }

    double higher() const
    {
        return _mean + _delta;
    }
    double lower() const
    {
        return _mean - _delta;
    }

    double width() const
    {
        return higher() - lower();
    }

    double precision() const
    {
        return _delta / _mean;
    }

    bool isInTval(double tval)
    {
        return lower() <= tval && tval <= higher();
    }

    double tvalDiff(double tval)
    {
        if (tval < lower())
            return lower() - tval;
        if (tval > higher())
            return higher() - tval;
        return 0.0;
    }
};

class BaseMeasure
{
  protected:
    std::string _name;
    std::string _unit;
    size_t _count{};

  public:
    BaseMeasure(std::string name, std::string unit) : _name(name), _unit(unit), _count(0)
    {
    }
    std::string Name() const
    {
        return _name;
    }
    size_t Count() const
    {
        return _count;
    }
    std::string Unit() const
    {
        return _unit;
    }
    virtual std::string Heading()
    {
        return fmt::format("{}({})", Name(), Unit());
    }

    virtual void Reset()
    {
        _count = 0;
    }

    virtual bool operator==(BaseMeasure &measure)
    {
        return this->_name == measure.Name() && this->Unit() == measure.Unit();
    }

    virtual bool operator!=(BaseMeasure &measure)
    {
        return !(*this == measure);
    }
    virtual ~BaseMeasure() = default;
};

template <typename T> class Measure : public BaseMeasure
{
  private:
    T _lastAccumulatedValue{};

  public:
    Measure(std::string name, std::string unit) : BaseMeasure(name, unit)
    {
    }

    virtual void Accumulate(T value)
    {
        _lastAccumulatedValue = value;
        _count++;
    }

    T Current() const
    {
        return _lastAccumulatedValue;
    }

    virtual std::string Csv()
    {
        return fmt::format("{}", _lastAccumulatedValue);
    }

    virtual std::string Json()
    {
        return fmt::format("\"lastAccumulatedValue\":{}", _lastAccumulatedValue);
    }

    void operator()(T value)
    {
        Accumulate(value);
        core_assert(_lastAccumulatedValue != NAN, "Value {} is drifting", Name());
    }

    void Reset() override
    {
        BaseMeasure::Reset();
        _lastAccumulatedValue = T{};
    }
};

template <int Moments = 2> class Accumulator : public Measure<double>
{

  private:
    double _confidence = 0.95;
    double _precision = 0.05;
    double _sum[Moments]{};

    double get(int moment) const
    {
        if (moment >= Moments)
            panic("moment selected is over Moments stored");
        return _sum[moment];
    }

    void constexpr ForMoment(std::function<void(double &, int)> &&operation)
    {
        for (int i = 0; i < Moments; i++)
            operation(_sum[i], i + 1);
    }

  public:
    Accumulator(std::string name, std::string unit) : Measure<double>(name, unit)
    {
    }

    Accumulator() : Measure<double>("", "")
    {
    }

    Accumulator<> &WithConfidence(double confidence)
    {
        _confidence = confidence;
        return *this;
    }

    Accumulator<> &WithPrecision(double precision)
    {
        _precision = precision;
        return *this;
    }

    void Accumulate(double value) override
    {
        Measure<double>::Accumulate(value);
        ForMoment([value](double &val, int moment) {
            val += pow(value, moment);
            core_assert(val != NAN, "Value {} is drifting in moment {}", val, moment);
        });
    }

    std::string Heading() override
    {
        std::string head = fmt::format("{};", Measure<double>::Heading());
        auto name = Measure<double>::Name();
        head += fmt::format("meanOf{};", name);
        head += fmt::format("varianceOf{};", name);
        head += fmt::format("lowerBoundOf{};", name);
        head += fmt::format("upperBoundOf{}", name);
        return head;
    }

    std::string Csv() override
    {
        std::string csv = fmt::format("{};", Measure<double>::Csv());
        csv += fmt::format("{};", mean());
        csv += fmt::format("{};", variance());
        csv += fmt::format("{};", confidence().higher());
        csv += fmt::format("{}", confidence().lower());
        return csv;
    }

    std::string Json() override
    {
        return fmt::format("{},\n \"samples\":{}\n \"mean\":{},\n \"variance\":{},\n \"lower_bound\":{},\n "
                           "\"higher_bound\":{} \n \"width\":{},\n \"precision\":{}\n",
                           Measure<double>::Json(), this->Count(), mean(), variance(), confidence().lower(),
                           confidence().higher(), confidence().width(), confidence().precision());
    }

    virtual void Reset() override
    {
        Measure<double>::Reset();
        ForMoment([](double &val, int moment) { val = 0; });
    }

    inline double mean(int moment = 0) const
    {
        return get(moment) / Count();
    }

    inline double variance() const
    {
        return mean(1) - pow(mean(0), 2); // this can lead to catastrophic cancellation
    }

    inline double sum() const
    {
        return _sum[0];
    }

    Interval confidence()
    {

        double u = mean(0);
        double sigma = variance();
        double alpha = 1 - _confidence;
        auto count = Count();
        double delta = 0.0;
        if (count < 40)
        {
            delta = idfStudent(count - 1, 1 - (alpha / 2)) * (sigma / sqrt(count - 1));
        }
        else
        {
            delta = idfNormal(0.0, 1.0, 1 - alpha / 2);
        }
        delta = (delta * variance()) / sqrtf(count);
        return {u, delta};
    }
};

template <> struct fmt::formatter<Accumulator<>>
{
    char mode[16]{};
    constexpr auto parse(format_parse_context &ctx) -> format_parse_context::iterator
    {
        auto itr = ctx.begin();
        int i = 0;
        while (itr != ctx.end() && *itr != '}')
        {
            mode[i] = *itr;
            itr++;
            i++;
        }
        return itr;
    }

    auto format(Accumulator<> m, format_context &ctx) const -> format_context::iterator
    {

        if (strcmp(mode, "csv") == 0)
        {
            return fmt::format_to(ctx.out(), "{};{};{};{};{};{}", m.Name(), m.mean(), m.confidence().lower(),
                                  m.confidence().higher(), m.Count(), m.confidence().precision());
        }
        else
        {
            return fmt::format_to(ctx.out(), "Measure: {}, R: {},LB:{}, HB:{}, Samples:{}, Variance:{} ,Precision:{}",
                                  m.Name(), m.mean(), m.confidence().lower(), m.confidence().higher(), m.Count(),
                                  m.variance(), m.confidence().precision());
        }
    }
};

template <int Moments = 2> struct BufferedMeasure : public Accumulator<Moments>
{
  private:
    std::vector<double> data{};

  public:
    virtual void Accumulate(double value) override
    {
        Accumulator<Moments>::Accumulate(value);
        data.push_back(value);
    }

    BufferedMeasure(std::string name, std::string unit) : Accumulator<Moments>(name, unit)
    {
    }

    virtual void Reset() override
    {
        Accumulator<>::Reset();
        data.clear();
    }
    const std::vector<double> &Data()
    {
        return data;
    }
};

template <> struct fmt::formatter<BufferedMeasure<>> : fmt::formatter<string_view>
{
    auto format(BufferedMeasure<> &m, format_context &ctx) -> format_context::iterator
    {
        return fmt::format_to(
            ctx.out(),
            "Measure: {}, Mean: {}, Variance:{}, Precision:{}, Samples:{}, LB:{}, LH:{},LastValue:{},BufferSize:{}",
            m.Name(), m.mean(), m.variance(), m.confidence().precision(), m.Count(), m.confidence().lower(),
            m.confidence().higher(), m.Current(), m.Data().size());
    }
};

template <int Esembles = 2> struct EsembledMeasure : public Measure<double>
{
  public:
    Accumulator<> accs[Esembles];
    EsembledMeasure() : Measure("", "")
    {
    }
    EsembledMeasure(std::string name, std::string unit) : Measure(name, unit)
    {
        for (int i = 0; i < Esembles; i++)
            accs[i] = Accumulator<>{name, unit};
    }

    void Accumulate(double value) override
    {
        Measure<double>::Accumulate(value);
        accs[0](value);
    }

    virtual void Reset() override
    {
        for (int i = 0; i < Esembles; i++)
            accs[i].Reset();
    }

    virtual void MoveEsemble(int esemble)
    {
        core_assert(esemble < Esembles, "Targeting unexsisting esemble");
        if (esemble > 0)
        {
            MoveEsemble(esemble - 1);
            if (accs[esemble - 1].Count() > 0)
                accs[esemble](accs[esemble - 1].mean());
        }
    }

    EsembledMeasure &WithConfidence(double confidence)
    {
        for (int i = 0; i < Esembles; i++)
            accs[i].WithConfidence(confidence);
        return *this;
    }

    EsembledMeasure &WithPrecision(double precision)
    {
        for (int i = 0; i < Esembles; i++)
            accs[i].WithPrecision(precision);
        return *this;
    }

    Accumulator<> &operator[](int esemble)
    {
        return accs[esemble];
    }

    Interval confidence()
    {
        return accs[Esembles - 1].confidence();
    }
};

template <int Esemble> struct fmt::formatter<EsembledMeasure<Esemble>>
{
    constexpr auto parse(format_parse_context &ctx) -> format_parse_context::iterator
    {
        return ctx.begin();
    }

    auto format(EsembledMeasure<Esemble> m, format_context &ctx) const -> format_context::iterator
    {
        format_context::iterator it = fmt::format_to(ctx.out(), "Measure:{},unit:{} ########\n", m.Name(), m.Unit());
        for (int i = 0; i < Esemble; i++)
        {
            fmt::format_to(it, "Ensemble:{}#########\n{}\n", i, m[i]);
        }
        return it;
    }
};

class MobileMeanMeasure : BaseMeasure
{
    std::vector<double> _means;
    std::vector<double> _buffer;
    int _meansPtr;
    int _bufferPtr;

  public:
    void push(double value);
    double delta() const;
    MobileMeanMeasure(int bufferSize, int maxMeans);
};

struct CovariatedMeasure : BaseMeasure
{
    double _confidence = 0.95;
    double _precision = 0.05;
    double _current[2]{};
    double _sum[2]{};
    double _times[2]{};
    double _weightedsum{};
    virtual void Accumulate(double value, double time);

    double R() const;
    double variance() const;
    Interval confidence() const;

    CovariatedMeasure(std::string name, std::string unit) : BaseMeasure(name, unit)
    {
    }

    CovariatedMeasure() : BaseMeasure("", "")
    {
    }

    CovariatedMeasure &WithConfidence(double confidence)
    {
        _confidence = confidence;
        return *this;
    }

    CovariatedMeasure &WithPrecision(double precision)
    {
        _precision = precision;
        return *this;
    }

    std::pair<double, double> Current()
    {
        return {_current[0], _current[1]};
    }

    void operator()(double value, double time)
    {
        Accumulate(value, time);
    }

    void Reset() override
    {
        memset(_sum, 0, sizeof(double) * 2);
        memset(_times, 0, sizeof(double) * 2);
        _weightedsum = 0;
        BaseMeasure::Reset();
    }
    int SampleNeedsForPrecision();

    double times(int moment = 0)
    {
        return _times[moment];
    }

    double sum(int moment = 0)
    {
        return _sum[moment];
    }
};

template <> struct fmt::formatter<CovariatedMeasure>
{
    char mode[16]{};
    constexpr auto parse(format_parse_context &ctx) -> format_parse_context::iterator
    {
        auto itr = ctx.begin();
        int i = 0;
        while (itr != ctx.end() && *itr != '}')
        {
            mode[i] = *itr;
            itr++;
            i++;
        }
        return itr;
    }

    auto format(const CovariatedMeasure &m, format_context &ctx) const -> format_context::iterator
    {

        if (strcmp(mode, "csv") == 0)
        {
            return fmt::format_to(ctx.out(), "{};{};{};{};{};{}", m.Name(), m.R(), m.confidence().lower(),
                                  m.confidence().higher(), m.Count(), m.confidence().precision());
        }
        else
        {
            return fmt::format_to(ctx.out(), "Measure: {}, R: {:5f},LB:{:5f}, HB:{:5f}, Samples:{} ,Precision:{:5f}", m.Name(),
                                  m.R(), m.confidence().lower(), m.confidence().higher(), m.Count(),
                                  m.confidence().precision());
        }
    }
};

namespace helper
{
template <typename T, typename K, typename F> std::vector<T> take(K item, F &&fnc)
{
    std::vector<T> res{};
    for (auto e : item)
    {
        res.push_back(fnc(e));
    }
    return res;
};

} // namespace helper
