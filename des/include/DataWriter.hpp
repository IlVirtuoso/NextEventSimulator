#include "DataCollector.hpp"
#include "Measure.hpp"
#include "Station.hpp"
#include <map>
#include <vector>

class DataWriter
{
  private:
    std::vector<DataCollector *> _collectors;

  public:
    static DataWriter &Instance()
    {
        static DataWriter instance{};
        return instance;
    }

    void Register(DataCollector *collector)
    {
        _collectors.push_back(collector);
    }

    void Unregister(DataCollector *collector)
    {
        auto itr = _collectors.begin();
        while (itr != _collectors.end())
        {
            if (*itr == collector)
            {
                _collectors.erase(itr);
                return;
            }
            itr++;
        }
    }
    void Flush();
    void SnapShot();
};
