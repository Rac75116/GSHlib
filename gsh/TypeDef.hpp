#pragma once
#include <type_traits>
#include <cstdint>
#include <limits>
#if __has_include(<stdfloat>)
#include <stdfloat>
#endif

namespace gsh {

namespace itype {
#if !defined(INT8_MAX) || !defined(UINT8_MAX)
    static_assert(false, "This library needs std::int8_t and std::uint8_t.");
#endif
#if !defined(INT16_MAX) || !defined(UINT16_MAX)
    static_assert(false, "This library needs std::int16_t and std::uint16_t.");
#endif
#if !defined(INT32_MAX) || !defined(UINT32_MAX)
    static_assert(false, "This library needs std::int32_t and std::uint32_t.");
#endif
#if !defined(INT64_MAX) || !defined(UINT64_MAX)
    static_assert(false, "This library needs std::int64_t and std::uint64_t.");
#endif
    using i8 = std::int8_t;
    using u8 = std::uint8_t;
    using i16 = std::int16_t;
    using u16 = std::uint16_t;
    using i32 = std::int32_t;
    using u32 = std::uint32_t;
    using i64 = std::int64_t;
    using u64 = std::uint64_t;
#ifdef __SIZEOF_INT128__
    using i128 = __int128_t;
    using u128 = __uint128_t;
#else
    static_assert(false, "This library needs __int128_t and __uint128_t.");
#endif
}  // namespace itype

namespace ftype {
#ifdef __STDCPP_FLOAT16_T__
    using f16 = std::float16_t;
#else
    using f16 = void;
#endif
#ifdef __STDCPP_FLOAT32_T__
    using f32 = std::float32_t;
#else
    static_assert(std::numeric_limits<float>::is_iec559, "There are no types compliant with IEC 559 binary32.");
    using f32 = float;
#endif
#ifdef __STDCPP_FLOAT64_T__
    using f64 = std::float64_t;
#else
    static_assert(std::numeric_limits<double>::is_iec559, "There are no types compliant with IEC 559 binary64.");
    using f64 = double;
#endif
#ifdef __STDCPP_FLOAT128_T__
    using f128 = std::float128_t;
#elif defined(__SIZEOF_FLOAT128__)
    using f128 = std::conditional_t<std::numeric_limits<long double>::is_iec559 && sizeof(long double) == 16, long double, __float128>;
#else
    using f128 = std::conditional_t<std::numeric_limits<long double>::is_iec559 && sizeof(long double) == 16, long double, void>;
#endif
#ifdef __STDCPP_BFLOAT16_T__
    using bf16 = std::bfloat16_t;
#else
    using bf16 = void;
#endif
}  // namespace ftype

namespace ctype {
    using c8 = char;
    using wc = wchar_t;
    using utf8 = char8_t;
    using utf16 = char16_t;
    using utf32 = char32_t;
}  // namespace ctype

}  // namespace gsh
