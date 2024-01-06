#include "Chains/DTMC.hpp"
#include "Collections/Matrix.hpp"
#include <Mva.hpp>
#include <fmt/core.h>
#include <fmt/format.h>
#include <fstream>
#include <ios>
#include <string>
#include <vector>

std::string MVAToString(Matrix<double> result, std::string resultName)
{
    auto res = fmt::format("########{}######\n", resultName);
    for (int i = 0; i < result.Rows(); i++)
    {
        res += fmt::format("S_{}_{}=[{}]\n", i, resultName, fmt::join(result.Row(i), ","));
    }
    return res;
}

std::string MVAToCsv(Matrix<double> result, std::string resultName)
{
    using namespace fmt;
    std::string header = "N;";
    for (int i = 0; i < result.Rows(); i++)
        header += format("S{};", i);
    header.erase(header.size() - 1);
    std::string res = format("{}\n", header);
    std::string line = "";
    for (int j = 0; j < result.Columns(); j++)
    {
        res += format("{};{}\n", j, join(result.Col(j), ";"));
    }
    return res;
}

void MVAToFile(Matrix<double> result, std::string resultName)
{
    using namespace fmt;
    auto csv = MVAToCsv(result, resultName);
    print("{}\n{}", resultName, csv);
    std::fstream file{format("{}.csv", resultName), std::ios_base::out};
    file.write(csv.c_str(), csv.size());
}

void DoMva(Matrix<double> transition)
{
    std::vector<double> visits = powerMatrixMethod(transition, 10000);
    for (int i = 1; i < visits.size(); i++)
    {
        visits[i] = visits[i] * (1 / visits[0]);
    }
    // std::vector<double> visits = {1, 250, 2.5, 16.25, 6.25};
    std::vector<double> stimes = {5000, 210, 2.7, 40, 180};
    std::vector<StationType> types = {D, I, I, I, I};
    auto result = MVALID(visits, stimes, types, 30);
    MVAToFile(result.meanClients, "meanClients");
    MVAToFile(result.meanWaits, "meanWaits");
    MVAToFile(result.throughputs, "throughputs");
    MVAToFile(result.utilizations, "utilizations");
}

int main()
{
    Matrix<double> q{
        {{0, 1, 0, 0, 0}, {0, 0, 1, 0, 0}, {0.004, 0.006, 0.9, 0.065, 0.025}, {0, 0, 1, 0, 0}, {0, 0, 1, 0, 0}}};
        DoMva(q);
}