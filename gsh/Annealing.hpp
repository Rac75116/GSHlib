#pragma once
#include <type_traits>      // std::invoke_result_t, std::decay_t, std::is_same_v
#include <ctime>            // std::clock_t, std::clock
#include <gsh/TypeDef.hpp>  // gsh::itype

namespace gsh {

namespace sa {

    class Time {
        std::clock_t start, end;
        gsh::itype::i64 cnt = 0, sum = 0, gap = 4;
    public:
        Time(gsh::itype::u32 time) { start = std::clock(), end = start + static_cast<std::clock_t>(time) * 1000 / CLOCKS_PER_SEC; }
        bool get() {
            if (++cnt == gap) {
                sum += cnt;
                cnt = 0;
                const std::clock_t now = std::clock();
                if (now >= end) [[unlikely]]
                    return false;
                const gsh::itype::i64 passed = now - start;
                const gsh::itype::i64 tmp = (static_cast<gsh::itype::i64>(sum) * (end - now) / (passed < 8 ? 8 : passed)) / 4;
                gap = (tmp < 4 ? 4 : tmp);
                return true;
            }
            return true;
        }
    };

}  // namespace sa

template<class State> void SimulatedAnealing(const State& s, gsh::sa::Time time) {
    using score_type = std::decay_t<std::invoke_result_t<&State::eval>>;
    using diff_type = std::decay_t<std::invoke_result_t<&State::next>>;
    score_type curscore = s.eval();
    score_type bestscore = curscore;
    State best = s;
    State cur = s;
    while (time.get()) {}
    return best;
}

}  // namespace gsh
