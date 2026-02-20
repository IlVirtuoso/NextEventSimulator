#include "DataWriter.hpp"
#include "LogEngine.hpp"
#include <filesystem>
#include <fmt/core.h>
#include <fstream>
#include <ios>
#include <sstream>
#include <string>

void DataWriter::Flush()
{
}

void DataWriter::SnapShot()
{
    std::fstream f{"data.json", std::ios_base::app | std::ios_base::out};
    for (auto collector : _collectors)
    {
        std::string json = "{\n";
        json += fmt::format("\"Station\":\"{}\",\n", collector->Name());
        json += fmt::format("\"TimeStamp\":\"{}\",\n", collector->TimeStamp());
        for (auto &measure : collector->GetAccumulators())
        {
            json += fmt::format("\"{}\":{},\n", measure->Name(),
                                std::string("{\n") + std::string(measure->Json()) + std::string("}\n"));
        }
        json += "},\n";
        f.write(json.c_str(), json.size());
        json.clear();
    }
}
