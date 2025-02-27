#pragma once
#include <chrono>   // std::chrono::system_clock
#include <limits>  // std::numeric_limits
#include "TypeDef.hpp"
#include "InOut.hpp"

namespace gsh {

template<class Clock = std::chrono::system_clock> class Timer {
    Clock::time_point start_time;
public:
    Timer() { start_time = Clock::now(); }
    void restart() { start_time = Clock::now(); }
    itype::u64 elapsed() const { return std::chrono::duration_cast<std::chrono::milliseconds>(Clock::now() - start_time).count(); }
};

template<class T> class Formatter<Timer<T>> {
public:
    template<class Stream> void operator()(Stream&& stream, const Timer<T>& t) {
        Formatter<itype::u64>{}(stream, t.elapsed());
        Formatter<ctype::c8>{}(stream, 'm');
        Formatter<ctype::c8>{}(stream, 's');
    }
};

}  // namespace gsh
