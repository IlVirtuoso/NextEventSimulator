#include "LogEngine.hpp"
#include <cstdio>
#include <filesystem>
#include <fmt/args.h>
#include <fmt/color.h>
#include <fmt/core.h>
#include <fmt/format.h>
#include <fstream>
#include <ios>
#include <istream>
#include <ostream>
#include <streambuf>
#include <string>

LogEngine *LogEngine::_instance = nullptr;

constexpr fmt::color LogTypeToColor(LogType type)
{
    using fmt::color;
    switch (type)
    {
    case LogType::EXCEPTION:
        return fmt::color::red;
    case LogType::RESULT:
        return fmt::color::green;
    case LogType::INFORMATION:
        return fmt::color::blue;
    case LogType::TRANSFER:
        return fmt::color::aqua;
    case LogType::DEBUG:
        return fmt::color::yellow;
    }
    return fmt::color::white;
}

void writeBuffer(std::istream &buffer, std::ostream &save)
{
    using namespace std;
    std::string line;
    while (std::getline(buffer, line))
    {
        auto formatted = fmt::format("{}\n", line);
        save.write(formatted.c_str(), formatted.size());
    }
}

/**
 * @brief  Save the log file
 * @note   used RTTI on fstream  to save the log file so at the end of the function the stream will be closed
 * @retval None
 */
void LogEngine::Flush()
{
    using namespace std;
    fstream logFile(_logFile, ios_base::app);
    writeBuffer(_buffer, logFile);
    _buffer.clear();
    _buffer.seekp(0);
}

void LogEngine::Trace(LogType type, std::string message)
{
    using namespace std;
    std::string log = fmt::format("[{}]{}\n", type, message);
    char buffer[log.size() + 1+ 1024];
    memset(buffer, 0, sizeof(buffer));
    sprintf(buffer, "%s\n", log.c_str());
    if ((_printStdout && !_pauseStdout))
        fmt::print(fmt::fg(LogTypeToColor(type)), "[{}]{}\n",type,message);
    _buffer.write(buffer, log.size() + 1);
    if (_buffer.tellp() > 1024 * 1024)
    {
        Flush();
    }
}
