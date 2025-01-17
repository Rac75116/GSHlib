#pragma once
#include <source_location>  // std::source_location
#include "TypeDef.hpp"      // gsh::itype
#include "InOut.hpp"        // gsh::BasicWriter

namespace gsh {
namespace internal {
#ifdef __cpp_lib_source_location
    void AssertPrint(const ctype::c8* message, std::source_location loc) {
        BasicWriter<2048> w(2);
#if !defined(_MSC_VER) && defined(GSH_DIAGNOSTICS_COLOR)
        w.write("\e[2m[Assert] \e[0mDuring the execution of \e[1m\e[3m'");
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
#else
        w.write("[Assert] During the execution of '");
        w.writeln(loc.function_name());
        w.write(loc.file_name());
        w.write(':');
        w.write(loc.line());
        w.write(':');
        w.write(loc.column());
        w.write(':');
        w.write(" Assertion Failed: '");
        w.write(message);
        w.writeln("'");
        w.reload();
#endif
    }
    template<itype::u32> GSH_INTERNAL_INLINE constexpr void Assert(const bool cond, const ctype::c8* message, std::source_location loc = std::source_location::current()) {
        if (!cond) [[unlikely]] {
            AssertPrint(message, loc);
            if (std::is_constant_evaluated()) {
                throw 0;
            } else {
                std::exit(1);
            }
        }
    }
#else
    template<itype::u32> GSH_INTERNAL_INLINE constexpr void Assert(const bool cond, const ctype::c8* message) {
        if (!cond) [[unlikely]] {
            if (std::is_constant_evaluated()) {
                throw 0;
            } else {
                BasicWriter<2048> w(2);
#if !defined(_MSC_VER) && defined(GSH_DIAGNOSTICS_COLOR)
                w.write("\e[2m[Assert]\e[0m ");
                w.write("\e[31mAssertion Failed:\e[0m \e[1m\e[3m'");
                w.write(message);
                w.write("'\e[0m\n");
                w.reload();
#else
                w.write("[Assert] ");
                w.write(" Assertion Failed: '");
                w.write(message);
                w.writeln("'");
                w.reload();
#endif
                std::exit(1);
            }
        }
    }
#endif
}  // namespace internal
}  // namespace gsh
#define GSH_INTERNAL_ASSERT1(cond)          gsh::internal::Assert<0>(cond, #cond)
#define GSH_INTERNAL_ASSERT2(cond, message) gsh::internal::Assert<0>(cond, message)
#define ASSERT(...)                         GSH_INTERNAL_SELECT3(__VA_ARGS__, GSH_INTERNAL_ASSERT2, GSH_INTERNAL_ASSERT1)(__VA_ARGS__)
