#pragma once
#include <concepts>
template <typename F, typename R, typename... Args>
concept has_return_value = requires(F a, Args... args) {
    {
        a(args...)
    } -> std::same_as<R>;
};

struct Slot
{
};

struct EventHandler
{
};