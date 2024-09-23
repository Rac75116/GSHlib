#pragma once
#include <ctime>   // std::clock_t, std::clock, CLOCKS_PER_SEC
#include <limits>  // std::numeric_limits
#include <cstdio>  // std::printf
#include "TypeDef.hpp"

namespace gsh {

class ClockTimer {
    std::clock_t start_time;
public:
    ClockTimer() { start_time = std::clock(); }
    void restart() { start_time = std::clock(); }
    std::clock_t elapsed() const { return (std::clock() - start_time) * 1000 / CLOCKS_PER_SEC; }
    void print() const { std::printf("gsh::ClockTimer::print / %llu ms\n", static_cast<unsigned long long>(elapsed())); }
};

}  // namespace gsh
