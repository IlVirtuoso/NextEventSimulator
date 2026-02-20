#pragma once

#include "Measure.hpp"
#include "Usings.hpp"
#include <map>
#include <memory>
#include <optional>
#include <string>
#include <vector>

class Station;
class DataCollector
{
    friend class Station;

  private:
    int _samples = 0;

  public:
    int getSamples() const;

  private:
    std::string _stationName{};
    std::vector<sptr<Measure<double>>> _measures{};
    double lastTimeStamp{};

  public:
    std::vector<sptr<Measure<double>>> GetAccumulators() const;
    sptr<Measure<double>> operator[](int index)
    {
        return _measures[index];
    }

    std::optional<sptr<Measure<double>>> operator[](std::string name)
    {
        for (auto &m : _measures)
        {
            if (m->Name() == name)
                return m;
        }
        return {};
    };
    inline double TimeStamp() const
    {
        return lastTimeStamp;
    }
    void SetTimeStamp(double timestamp)
    {
        lastTimeStamp = timestamp;
    }

    template <typename M> void AddMeasure(M measure)
    {
        _measures.push_back(std::make_shared<M>(measure));
    }
    std::string Name() const
    {
        return _stationName;
    }
    void Reset();
    std::string Header() const;
    std::string Csv();
    DataCollector(std::string stationName);
};
