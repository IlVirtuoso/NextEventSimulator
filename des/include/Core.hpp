#pragma once
#include <LogEngine.hpp>
#include <fmt/core.h>

[[noreturn]] void _panic(const char *message);

void _panic(const char *message, const char *file, int line);

void _panic(std::string message, const char *file, int line);

#define panic(message) _panic(message, __FILE_NAME__, __LINE__)
#ifdef CORE_DEBUG
#define core_assert(condition,fmtline ...) \
if(!(condition))\
panic(fmt::format(fmtline));
#else
#define core_assert(condition, failmessage, ...)
#endif