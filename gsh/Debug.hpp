#pragma once
#include <source_location>  // std::source_location
#include <type_traits>
#include "InOut.hpp"
#include "Util.hpp"

#ifndef NDEBUG
namespace gsh {
namespace internal {
    BasicWriter<2048> DebugPrinter(2);
    template<class... Args> void DebugPrintImpl(std::source_location loc, Args&&... args) {
#if !defined(_MSC_VER) && defined(GSH_DIAGNOSTICS_COLOR)
        DebugPrinter.write("\e[2m[Debug] ", loc.file_name(), ':', loc.line(), ':', loc.column(), "\n\e[0m\e[1m\e[3m").sep(NoOut);
        DebugPrinter.write(std::forward<Args>(args)...).sep("\e[0m\e[2m / \e[0m\e[1m\e[3m").end("\e[0m\n");
        DebugPrinter.reload();
#else
        DebugPrinter.writeln("[Debug] ", loc.file_name(), ':', loc.line(), ':', loc.column()).sep(NoOut);
        DebugPrinter.writeln(std::forward<Args>(args)...).sep(" / ");
        DebugPrinter.reload();
#endif
    }
    template<class... Args> constexpr void DebugPrint(std::source_location loc, Args&&... args) {
        if (std::is_constant_evaluated()) {
            return;
        } else {
            DebugPrintImpl(loc, std::forward<Args>(args)...);
        }
    }
}  // namespace internal
}  // namespace gsh
#endif

// clang-format off
#ifdef NDEBUG
#define DEBUG(...) [](){}()
#else
#define DEBUG(...) gsh::internal::DebugPrint(std::source_location::current(), __VA_ARGS__)
#endif
// clang-format on
