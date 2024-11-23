#pragma once
#include <cstdlib>          // std::exit
#include <tuple>            // std::forward_as_tuple
#include <source_location>  // std::source_location
#include <ranges>           // std::ranges
#include "TypeDef.hpp"      // gsh::itype
#include "InOut.hpp"        // gsh::BasicWriter, gsh::Formatter

#define ALL(V)       std::ranges::begin(V), std::ranges::end(V)
#define RALL(V)      std::ranges::rbegin(V), std::ranges::rend(V)
#define ALLMID(V, n) std::ranges::begin(V), std::ranges::next(std::ranges::begin(V), n), std::ranges::end(V)
#define VALUE(...)   (([&]() __VA_ARGS__)())
// clang-format off
#define RET_WITH(...) { __VA_ARGS__; return; } []{}
#define RETV_WITH(val, ...) { __VA_ARGS__; return val; } []{}
#define BRK_WITH(...) { __VA_ARGS__; break; } []{}
#define CTN_WITH(...) { __VA_ARGS__; continue; } []{}
#define EXT_WITH(...) { __VA_ARGS__; std::exit(0); } []{}

#define GSH_INTERNAL_ARGS0() ()
#define GSH_INTERNAL_ARGS1(a) (auto&& a)
#define GSH_INTERNAL_ARGS2(a, b) (auto&& a, auto&& b)
#define GSH_INTERNAL_ARGS3(a, b, c) (auto&& a, auto&& b, auto&& c)
#define GSH_INTERNAL_ARGS4(a, b, c, d) (auto&& a, auto&& b, auto&& c, auto&& d)
#define GSH_INTERNAL_ARGS5(a, b, c, d, e) (auto&& a, auto&& b, auto&& c, auto&& d, auto&& e)
#define GSH_INTERNAL_ARGS6(a, b, c, d, e, f) (auto&& a, auto&& b, auto&& c, auto&& d, auto&& e, auto&& f)
#define GSH_INTERNAL_ARGS7(a, b, c, d, e, f, g) (auto&& a, auto&& b, auto&& c, auto&& d, auto&& e, auto&& f, auto&& g)
#define GSH_INTERNAL_ARGS(...) GSH_INTERNAL_SELECT8(__VA_ARGS__, GSH_INTERNAL_ARGS7, GSH_INTERNAL_ARGS6, GSH_INTERNAL_ARGS5, GSH_INTERNAL_ARGS4, GSH_INTERNAL_ARGS3, GSH_INTERNAL_ARGS2, GSH_INTERNAL_ARGS1, GSH_INTERNAL_ARGS0)(__VA_ARGS__)
#define GSH_INTERNAL_LAMBDA1(...) [&]() { return (__VA_ARGS__); }
#define GSH_INTERNAL_LAMBDA2(args, ...) [&] GSH_INTERNAL_ARGS##args { return (__VA_ARGS__); }
#define LAMBDA(...) GSH_INTERNAL_SELECT3(__VA_ARGS__, GSH_INTERNAL_LAMBDA2, GSH_INTERNAL_LAMBDA1)(__VA_ARGS__)
// clang-format on

/*
#define REP(var_name, ...)  for ([[maybe_unused]] auto var_name : gsh::Step(__VA_ARGS__))
#define RREP(var_name, ...) for (auto var_name : gsh::Step(__VA_ARGS__) | std::ranges::reverse)
*/

namespace gsh {
namespace internal {
    void AssertPrint(const ctype::c8* message, std::source_location loc) {
        BasicWriter<2048> w(2);
        w.write("\e[2m[from gsh::internal::Assert] \e[0mDuring the execution of \e[1m\e[3m'");
        w.write(loc.function_name());
        w.write("'\e[0m\n");
        w.write(loc.file_name());
        w.write(':');
        w.write(loc.line());
        w.write(':');
        w.write(loc.column());
        w.write(':');
        w.write(" \e[31mAssertion Failed:\e[0m \e[1m\e[3m'");
        w.write(message);
        w.write("'\e[0m\n");
        w.reload();
    }
    GSH_INTERNAL_INLINE constexpr void Assert(const bool cond, const ctype::c8* message, std::source_location loc = std::source_location::current()) {
        if (!cond) [[unlikely]] {
            if (std::is_constant_evaluated()) {
                throw 0;
            } else {
                AssertPrint(message, loc);
                std::exit(1);
            }
        }
    }
}  // namespace internal
}  // namespace gsh
#define GSH_INTERNAL_ASSERT1(cond)          gsh::internal::Assert(cond, #cond)
#define GSH_INTERNAL_ASSERT2(cond, message) gsh::internal::Assert(cond, message)
#define ASSERT(...)                         GSH_INTERNAL_SELECT3(__VA_ARGS__, GSH_INTERNAL_ASSERT2, GSH_INTERNAL_ASSERT1)(__VA_ARGS__)
