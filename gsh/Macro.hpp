#pragma once
#include <iterator>         // std::next
#include <cstdlib>          // std::exit
#include <tuple>            // std::forward_as_tuple
#include <source_location>  // std::source_location
#include "TypeDef.hpp"      // gsh::itype
#include "InOut.hpp"        // gsh::BasicWriter, gsh::Formatter
#include "Range.hpp"        // gsh::RangeTraits
#include "Option.hpp"       // gsh::Option
#include "Util.hpp"         // gsh::Step

#define ALL(V)       std::begin(V), std::end(V)
#define RALL(V)      std::rbegin(V), std::rend(V)
#define ALLMID(V, n) std::begin(V), std::next(std::begin(V), n), std::end(V)
#define VALUE(...)   (([&]() __VA_ARGS__)())
// clang-format off
#define RET_WITH(...) { __VA_ARGS__; return; } []{}
#define RETV_WITH(val, ...) { __VA_ARGS__; return val; } []{}
#define BRK_WITH(...) { __VA_ARGS__; break; } []{}
#define CTN_WITH(...) { __VA_ARGS__; continue; } []{}
#define EXT_WITH(...) { __VA_ARGS__; std::exit(0); } []{}
#define ARGS(...) __VA_ARGS__
#define EXPR(args, ...) [&](auto&&... GSH_EXPR_ARGS){ auto [args] = std::forward_as_tuple(GSH_EXPR_ARGS...); return (__VA_ARGS__); }
// clang-format on

/*
#define REP(var_name, ...)  for ([[maybe_unused]] auto var_name : gsh::Step(__VA_ARGS__))
#define RREP(var_name, ...) for (auto var_name : gsh::Step(__VA_ARGS__) | std::ranges::reverse)
*/

namespace gsh {
namespace internal {
    void AssertPrint(const ctype::c8* message, std::source_location loc) {
        BasicWriter<2048> w(2);
        Formatter<const ctype::c8*>{}(w, "\e[2m[from gsh::internal::AssertPrint]\e[0m\nDuring the execution of \e[1m\e[3m'");
        Formatter<const ctype::c8*>{}(w, loc.function_name());
        Formatter<const ctype::c8*>{}(w, "'\e[0m\n");
        Formatter<const ctype::c8*>{}(w, loc.file_name());
        Formatter<ctype::c8>{}(w, ':');
        Formatter<itype::u32>{}(w, loc.line());
        Formatter<ctype::c8>{}(w, ':');
        Formatter<itype::u32>{}(w, loc.column());
        Formatter<ctype::c8>{}(w, ':');
        Formatter<const ctype::c8*>{}(w, " \e[31mAssertion Faild:\e[0m \e[1m\e[3m'");
        Formatter<const ctype::c8*>{}(w, message);
        Formatter<const ctype::c8*>{}(w, "'\e[0m\n");
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
