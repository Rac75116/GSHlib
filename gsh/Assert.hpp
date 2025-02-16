#pragma once
#include <source_location>  // std::source_location
#include "TypeDef.hpp"      // gsh::itype
#include "InOut.hpp"        // gsh::BasicWriter

namespace gsh {
namespace internal {
    [[noreturn]] void AssertPrint(const ctype::c8* message, std::source_location loc) {
        BasicWriter<2048> w(2);
#if !defined(_MSC_VER) && defined(GSH_DIAGNOSTICS_COLOR)
        w.writeln("\e[2m[Assert] ", loc.file_name(), ':', loc.line(), ':', loc.column(), "\n\e[0mDuring the execution of \e[1m\e[3m'", loc.function_name(), "'\e[0m: \e[31mAssertion Failed:\e[0m \e[1m\e[3m'", message, "'\e[0m").sep(NoOut);
        w.reload();
#else
        w.writeln("[Assert] ", loc.file_name(), ':', loc.line(), ':', loc.column(), "\nDuring the execution of '", loc.function_name(), "': Assertion Failed: '", message, '\'').sep(NoOut);
        w.reload();
#endif
        std::exit(1);
    }
    template<itype::u32> GSH_INTERNAL_INLINE constexpr void Assert(const bool cond, const ctype::c8* message, std::source_location loc = std::source_location::current()) {
        if (!cond) [[unlikely]] {
            if (std::is_constant_evaluated()) {
                throw 0;
            } else {
                AssertPrint(message, loc);
            }
        }
    }
}  // namespace internal
}  // namespace gsh

// clang-format off
#define GSH_INTERNAL_ASSERT1(cond)          gsh::internal::Assert<0>(cond, #cond)
#define GSH_INTERNAL_ASSERT2(cond, message) gsh::internal::Assert<0>(cond, message)
#ifdef NDEBUG
#define ASSERT(...) [](){}
#else
#define ASSERT(...) GSH_INTERNAL_SELECT3(__VA_ARGS__, GSH_INTERNAL_ASSERT2, GSH_INTERNAL_ASSERT1)(__VA_ARGS__)
#endif
//clang-format on
