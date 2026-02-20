#pragma once

#include <fmt/core.h>
#include <fmt/format.h>

struct FormatParser
{
    std::string fmt = "";
    std::string innerfmt = "{}";
    constexpr auto parse(fmt::format_parse_context &ctx) -> fmt::format_parse_context::iterator
    {
        auto itr = ctx.begin();
        if (itr == nullptr)
            return ctx.end();
        while (itr != ctx.end() && *itr != '}')
        {
            fmt += *itr;
        }
        return ctx.end();
    }
};