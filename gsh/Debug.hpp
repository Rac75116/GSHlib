#pragma once
#include <source_location>  // std::source_location
#include <type_traits>
#include "InOut.hpp"

#ifndef NDEBUG
namespace gsh {
namespace internal {
    BasicWriter<2048> DebugPrinter;
    template<class... Args> void DebugPrintImpl(std::source_location loc, Args&&... args) {
        DebugPrinter.write("[Debug] ");
        DebugPrinter.write_sep(" / ", std::forward<Args>(args)...);
        DebugPrinter.writeln(" | ", loc.function_name(), ' ', loc.file_name(), ':', loc.line(), ':', loc.column());
    }
    template<class... Args> constexpr void DebugPrint(std::source_location loc, Args&&... args) {
        if (std::is_constant_evaluated()) {
            return;
        }
        DebugPrintImpl(loc, std::forward<Args>(args)...);
    }
}  // namespace internal
}  // namespace gsh
#endif

// clang-format off
#ifdef NDEBUG
#define DEBUG(...) [](){}
#else
#define DEBUG(...) gsh::internal::DebugPrint(std::source_location::current(), __VA_ARGS__)
#endif
// clang-format on
