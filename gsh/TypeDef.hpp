#pragma once
#include <cstddef>
#include <cstdint>
#include <limits>
#include <type_traits>
#if __has_include(<stdfloat>)
#include <stdfloat>
#endif
namespace gsh {
using i8 = std::int8_t;
using u8 = std::uint8_t;
using i16 = std::int16_t;
using u16 = std::uint16_t;
using i32 = std::int32_t;
using u32 = std::uint32_t;
using i64 = long long;
using u64 = unsigned long long;
static_assert(sizeof(i64) == 8);
static_assert(sizeof(u64) == 8);
class InvalidFloat16Tag;
class InvalidBfloat16Tag;
class InvalidFloat128Tag;
#ifdef __STDCPP_FLOAT16_T__
using f16 = std::float16_t;
#else
using f16 = InvalidFloat16Tag;
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
using f128 = std::conditional_t<std::numeric_limits<long double>::is_iec559 && sizeof(long double) == 16, long double, InvalidFloat128Tag>;
#endif
#ifdef __STDCPP_BFLOAT16_T__
using bf16 = std::bfloat16_t;
#else
using bf16 = InvalidBfloat16Tag;
#endif
using c8 = char;
using wc = wchar_t;
using utf8 = char8_t;
using utf16 = char16_t;
using utf32 = char32_t;
} // namespace gsh
