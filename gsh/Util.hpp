#pragma once
#include <type_traits>  // std::is_constant_evaluated
#include <gsh/TypeDef.hpp>
#include <gsh/internal/UtilMacro.hpp>

namespace gsh {

[[noreturn]] void Unreachable() {
#if defined __GNUC__ || defined __clang__
    __builtin_unreachable();
#elif _MSC_VER
    __assume(false);
#else
    [[maybe_unused]] itype::u32 n = 1 / 0;
#endif
};
GSH_INTERNAL_INLINE constexpr void Assume(const bool f) {
    if (std::is_constant_evaluated()) return;
#if defined __clang__
    __builtin_assume(f);
#elif defined __GNUC__
    if (!f) __builtin_unreachable();
#elif _MSC_VER
    __assume(f);
#else
    if (!f) Unreachable();
#endif
}
template<bool Likely = true> GSH_INTERNAL_INLINE constexpr bool Expect(const bool f) {
    if (std::is_constant_evaluated()) return f;
#if defined __GNUC__ || defined __clang__
    return __builtin_expect(f, Likely);
#else
    if constexpr (Likely) {
        if (f) [[likely]]
            return true;
        else return false;
    } else {
        if (f) [[unlikely]]
            return false;
        else return true;
    }
#endif
}
GSH_INTERNAL_INLINE constexpr bool Unpredictable(const bool f) {
    if (std::is_constant_evaluated()) return f;
#if defined __clang__
    return __builtin_unpredictable(f);
#elif defined __GNUC__
    return __builtin_expect_with_probability(f, 1, 0.5);
#else
    return f;
#endif
}
template<class T>
    requires std::is_scalar_v<T>
GSH_INTERNAL_INLINE constexpr void ForceCalc([[maybe_unused]] const T& v) {
#if defined __GNUC__ || defined __INTEL_COMPILER
    if (!std::is_constant_evaluated()) __asm__ volatile("" ::"r"(v));
#endif
}
template<class T, class... Args> GSH_INTERNAL_INLINE constexpr void ForceCalc(const T& v, const Args&... args) {
    ForceCalc(v);
    ForceCalc(args...);
}

class InPlaceTag {};
constexpr InPlaceTag InPlace;


}  // namespace gsh
