#pragma once
#include <ctime>   // std::clock_t, std::clock, CLOCKS_PER_SEC
#include <limits>  // std::numeric_limits
#include "TypeDef.hpp"
#include "InOut.hpp"

namespace gsh {

class Timer {
    std::clock_t start_time;
public:
    Timer() { start_time = std::clock(); }
    void restart() { start_time = std::clock(); }
    itype::u64 elapsed() const { return static_cast<itype::u64>(std::clock() - start_time) * 1000 / CLOCKS_PER_SEC; }
};

template<> class Formatter<Timer> {
public:
    template<class Stream> void operator()(Stream&& stream, const Timer& t) {
        Formatter<itype::u64>{}(stream, t.elapsed());
        Formatter<ctype::c8>{}(stream, 'm');
        Formatter<ctype::c8>{}(stream, 's');
    }
};

}  // namespace gsh
