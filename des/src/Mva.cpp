#include "Mva.hpp"
#include <vector>
using Vector = std::vector<double>;

Vector RouteToVisit(Matrix<double> &routings)
{
    Vector visits(routings.Rows());
    auto m = routings.Copy();
    auto old = m.Copy();
    for (int i = 0; i < 10; i++)
    {
        m = old * old;
        old = m;
    }
    for (int i = 0; i < routings.Rows(); i++)
        visits[i] = m(i, i);
    for (int i = 1; i < visits.size(); i++)
        visits[i] = visits[i] * (1 / visits[0]);
    visits[0] = 1;
    return visits;
}

MVAResult MVALID(const Vector &visits, const Vector &serviceTimes, const std::vector<StationType> &types, int N)
{
    int M = visits.size();
    MVAResult result(M, N);

    for (int i = 0; i < M; i++)
    {
        result.meanClients(i, 0) = 0;
        result.meanWaits(i, 0) = 0;
    }

    for (int k = 1; k < N; k++)
    {
        for (int i = 0; i < M; i++)
        {
            result.meanWaits(i, k) =
                types[i] == StationType::D ? serviceTimes[i] : serviceTimes[i] * (1 + result.meanClients(i, k - 1));
        }
        double sum = 0.0;
        for (int i = 0; i < M; i++)
            sum += visits[i] * result.meanWaits(i, k);
        double Xref = k / sum;
        for (int i = 0; i < M; i++)
        {
            result.throughputs(i, k) = visits[i] * Xref;
            if (types[i] == StationType::D)
            {
                result.meanClients(i, k) = serviceTimes[i] * result.throughputs(i, k);
                result.utilizations(i, k) = result.meanClients(i, k) / k;
            }
            else
            {
                result.utilizations(i, k) = serviceTimes[i] * result.throughputs(i, k);
                result.meanClients(i, k) = result.utilizations(i, k) * (1 + result.meanClients(i, k - 1));
            }
        }
    }
    return result;
}

MVAResult::MVAResult(int mStations, int nClients)
    : meanClients(mStations, nClients), meanWaits(mStations, nClients), throughputs(mStations, nClients),
      utilizations(mStations, nClients)
{
}
