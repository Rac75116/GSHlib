#pragma once
#include "Exception.hpp"
#include "Int128.hpp"
#include "TypeDef.hpp"
#include "Util.hpp"
#include <charconv>
#include <ranges>
#include <tuple>
#include <utility>
namespace gsh {
namespace io {
struct i4dig;
struct u4dig;
struct i8dig;
struct u8dig;
struct i16dig;
struct u16dig;
struct u8_pad;
struct u16_pad;
struct u32_pad;
struct u64_pad;
struct u128_pad;
struct u4dig_pad;
struct u8dig_pad;
struct u16dig_pad;
enum class FormatterOption : std::underlying_type_t<std::chars_format> {};
constexpr FormatterOption operator|(FormatterOption a, FormatterOption b) noexcept { return static_cast<FormatterOption>(static_cast<std::underlying_type_t<FormatterOption>>(a) | static_cast<std::underlying_type_t<FormatterOption>>(b)); }
[[maybe_unused]] constexpr auto Fixed = static_cast<FormatterOption>(std::chars_format::fixed);
[[maybe_unused]] constexpr auto General = static_cast<FormatterOption>(std::chars_format::general);
[[maybe_unused]] constexpr auto Hex = static_cast<FormatterOption>(std::chars_format::hex);
[[maybe_unused]] constexpr auto Scientific = static_cast<FormatterOption>(std::chars_format::scientific);
} // namespace io
template<class T> class Formatter;
namespace internal {
template<bool Flag> constexpr auto InttoStr = [] {
  struct {
    c8 table[40004] = {};
  } res;
  if constexpr(Flag) {
    for(u32 i = 0; i != 10000; ++i) {
      res.table[4 * i + 0] = i < 1000 ? ' ' : (i / 1000 + '0');
      res.table[4 * i + 1] = i < 100 ? ' ' : (i / 100 % 10 + '0');
      res.table[4 * i + 2] = i < 10 ? ' ' : (i / 10 % 10 + '0');
      res.table[4 * i + 3] = (i % 10 + '0');
    }
  } else {
    for(u32 i = 0; i != 10000; ++i) {
      res.table[4 * i + 0] = (i / 1000 + '0');
      res.table[4 * i + 1] = (i / 100 % 10 + '0');
      res.table[4 * i + 2] = (i / 10 % 10 + '0');
      res.table[4 * i + 3] = (i % 10 + '0');
    }
  }
  return res;
}();
template<bool Flag, class T> GSH_INTERNAL_INLINE constexpr void CopyHead(c8*& p, T x) {
  if constexpr(Flag) {
    MemoryCopy(p, InttoStr<Flag>.table + 4 * x, 4);
    p += 4;
  } else {
    u32 off = (x < 10) + (x < 100) + (x < 1000);
    MemoryCopy(p, InttoStr<false>.table + (4 * x + off), 4);
    p += 4 - off;
  }
}
template<class T> GSH_INTERNAL_INLINE constexpr void CopyAll(c8*& p, T x) {
  MemoryCopy(p, InttoStr<false>.table + 4 * x, 4);
  p += 4;
}
template<bool Flag = false, class Stream> constexpr void Formatu16(Stream&& stream, u16 n) {
  c8 *cur = stream.current(), *p = cur;
  if(n < 10000) CopyHead<Flag>(p, n);
  else CopyHead<Flag>(p, n / 10000), CopyAll(p, n % 10000);
  stream.skip(p - cur);
}
template<bool Flag = false, class Stream> constexpr void Formatu32(Stream&& stream, u32 n) {
  c8 *cur = stream.current(), *p = cur;
  if(n < 100000000) {
    if(n < 10000) CopyHead<Flag>(p, n);
    else CopyHead<Flag>(p, n / 10000), CopyAll(p, n % 10000);
  } else CopyHead<Flag>(p, n / 100000000), CopyAll(p, n / 10000 % 10000), CopyAll(p, n % 10000);
  stream.skip(p - cur);
}
template<bool Flag = false, class Stream> constexpr void Formatu64(Stream&& stream, u64 n) {
  c8 *cur = stream.current(), *p = cur;
  if(n >= 10000000000000000) {
    u64 a = n / 100000000, b = n % 100000000;
    u64 c = a / 10000, d = a % 10000, e = b / 10000, f = b % 10000;
    u64 g = c / 10000, h = c % 10000;
    CopyHead<Flag>(p, g), CopyAll(p, h), CopyAll(p, d), CopyAll(p, e), CopyAll(p, f);
  } else if(n >= 1000000000000) {
    u64 a = n / 100000000, b = n % 100000000;
    u64 c = a / 10000, d = a % 10000, e = b / 10000, f = b % 10000;
    CopyHead<Flag>(p, c), CopyAll(p, d), CopyAll(p, e), CopyAll(p, f);
  } else if(n >= 100000000) {
    u64 a = n / 100000000, b = n % 100000000;
    u64 c = b / 10000, d = b % 10000;
    CopyHead<Flag>(p, a), CopyAll(p, c), CopyAll(p, d);
  } else if(n >= 10000) {
    u64 a = n / 10000, b = n % 10000;
    CopyHead<Flag>(p, a), CopyAll(p, b);
  } else {
    CopyHead<Flag>(p, n);
  }
  stream.skip(p - cur);
}
template<bool Flag = false, class Stream> constexpr void Formatu128(Stream&& stream, u128 n) {
  if(u64(n >> 64) == 0) {
    Formatu64<Flag>(std::forward<Stream>(stream), n);
    return;
  }
  c8 *cur = stream.current(), *p = cur;
  constexpr u128 t = static_cast<u128>(10000000000000000) * 10000000000000000;
  if(n >= t) {
    const u32 dv = n / t;
    n -= dv * t;
    if(dv >= 10000) {
      CopyHead<Flag>(p, dv / 10000), CopyAll(p, dv % 10000);
    } else CopyHead<Flag>(p, dv);
    auto [a, b] = Divu128(n >> 64, n, 10000000000000000);
    const u32 c = a / 100000000, d = a % 100000000, e = b / 100000000, f = b % 100000000;
    CopyAll(p, c / 10000), CopyAll(p, c % 10000);
    CopyAll(p, d / 10000), CopyAll(p, d % 10000);
    CopyAll(p, e / 10000), CopyAll(p, e % 10000);
    CopyAll(p, f / 10000), CopyAll(p, f % 10000);
  } else {
    auto [a, b] = Divu128(n >> 64, n, 10000000000000000);
    const u32 c = a / 100000000, d = a % 100000000, e = b / 100000000, f = b % 100000000;
    const u32 g = c / 10000, h = c % 10000, i = d / 10000, j = d % 10000, k = e / 10000, l = e % 10000, m = f / 10000, n = f % 10000;
    if(c == 0) {
      if(i == 0) CopyHead<Flag>(p, j);
      else CopyHead<Flag>(p, i), CopyAll(p, j);
    } else {
      if(g == 0) CopyHead<Flag>(p, h), CopyAll(p, i), CopyAll(p, j);
      else CopyHead<Flag>(p, g), CopyAll(p, h), CopyAll(p, i), CopyAll(p, j);
    }
    CopyAll(p, k), CopyAll(p, l), CopyAll(p, m), CopyAll(p, n);
  }
  stream.skip(p - cur);
}
template<bool Flag = false, class Stream> constexpr void Formatu4dig(Stream&& stream, u16 x) {
  c8 *cur = stream.current(), *p = cur;
  CopyHead<Flag>(p, x);
  stream.skip(p - cur);
}
template<bool Flag = false, class Stream> constexpr void Formatu8dig(Stream&& stream, u32 n) {
  c8 *cur = stream.current(), *p = cur;
  if(n < 10000) CopyHead<Flag>(p, n);
  else CopyHead<Flag>(p, n / 10000), CopyAll(p, n % 10000);
  stream.skip(p - cur);
}
template<bool Flag = false, class Stream> constexpr void Formatu16dig(Stream&& stream, u64 n) {
  c8 *cur = stream.current(), *p = cur;
  if(n >= 1000000000000) {
    u64 a = n / 100000000, b = n % 100000000;
    u64 c = a / 10000, d = a % 10000, e = b / 10000, f = b % 10000;
    CopyHead<Flag>(p, c), CopyAll(p, d), CopyAll(p, e), CopyAll(p, f);
  } else if(n >= 100000000) {
    u64 a = n / 100000000, b = n % 100000000;
    u64 c = b / 10000, d = b % 10000;
    CopyHead<Flag>(p, a), CopyAll(p, c), CopyAll(p, d);
  } else if(n >= 10000) {
    u64 a = n / 10000, b = n % 10000;
    CopyHead<Flag>(p, a), CopyAll(p, b);
  } else {
    CopyHead<Flag>(p, n);
  }
  stream.skip(p - cur);
}
template<class T, u32 Reload, auto Func> struct UnsignedFormatter {
  using value_type = T;
  template<class Stream> constexpr void operator()(Stream&& stream, T n) const {
    stream.reload(Reload);
    Func(stream, n);
  }
};
template<class T, u32 Reload, auto Func> struct SignedFormatter {
  using value_type = T;
  template<class Stream> constexpr void operator()(Stream&& stream, T n) const {
    stream.reload(Reload);
    *stream.current() = '-';
    stream.skip(n < 0);
    Func(stream, n < 0 ? -n : n);
  }
};
template<class T> struct FloatFormatter {
  using value_type = T;
  template<class Stream> constexpr void operator()(Stream&& stream, T f, io::FormatterOption fmt = io::Fixed, i32 precision = 12) {
    stream.reload(32);
    auto [ptr, err] = std::to_chars(stream.current(), stream.current() + stream.avail(), f, static_cast<std::chars_format>(fmt), precision);
    if(err != std::errc{}) [[unlikely]] {
      stream.reload();
      auto [ptr, err] = std::to_chars(stream.current(), stream.current() + stream.avail(), f, static_cast<std::chars_format>(fmt), precision);
      if(err != std::errc{}) throw Exception("gsh::internal::FloatFormatter::operator() / The value is too large.");
      stream.skip(ptr - stream.current());
    } else {
      stream.skip(ptr - stream.current());
    }
  }
};
} // namespace internal
template<> class Formatter<u8> : public internal::UnsignedFormatter<u8, 4, [](auto& s, u8 n) { internal::Formatu4dig(s, n); }> {};
template<> class Formatter<i8> : public internal::SignedFormatter<i8, 4, [](auto& s, i8 n) { internal::Formatu4dig(s, n); }> {};
template<> class Formatter<u16> : public internal::UnsignedFormatter<u16, 8, [](auto& s, u16 n) { internal::Formatu16(s, n); }> {};
template<> class Formatter<i16> : public internal::SignedFormatter<i16, 8, [](auto& s, i16 n) { internal::Formatu16(s, n); }> {};
template<> class Formatter<u32> : public internal::UnsignedFormatter<u32, 16, [](auto& s, u32 n) { internal::Formatu32(s, n); }> {};
template<> class Formatter<i32> : public internal::SignedFormatter<i32, 16, [](auto& s, i32 n) { internal::Formatu32(s, n); }> {};
template<> class Formatter<u64> : public internal::UnsignedFormatter<u64, 32, [](auto& s, u64 n) { internal::Formatu64(s, n); }> {};
template<> class Formatter<i64> : public internal::SignedFormatter<i64, 32, [](auto& s, i64 n) { internal::Formatu64(s, n); }> {};
template<> class Formatter<u128> : public internal::UnsignedFormatter<u128, 64, [](auto& s, u128 n) { internal::Formatu128(s, n); }> {};
template<> class Formatter<i128> : public internal::SignedFormatter<i128, 64, [](auto& s, i128 n) { internal::Formatu128(s, n); }> {};
template<> class Formatter<io::u4dig> : public internal::UnsignedFormatter<u16, 4, [](auto& s, u16 n) { internal::Formatu4dig(s, n); }> {};
template<> class Formatter<io::i4dig> : public internal::SignedFormatter<i16, 5, [](auto& s, i16 n) { internal::Formatu4dig(s, n); }> {};
template<> class Formatter<io::u8dig> : public internal::UnsignedFormatter<u32, 8, [](auto& s, u32 n) { internal::Formatu8dig(s, n); }> {};
template<> class Formatter<io::i8dig> : public internal::SignedFormatter<i32, 9, [](auto& s, i32 n) { internal::Formatu8dig(s, n); }> {};
template<> class Formatter<io::u16dig> : public internal::UnsignedFormatter<u64, 16, [](auto& s, u64 n) { internal::Formatu16dig(s, n); }> {};
template<> class Formatter<io::i16dig> : public internal::SignedFormatter<i64, 17, [](auto& s, i64 n) { internal::Formatu16dig(s, n); }> {};
template<> class Formatter<io::u8_pad> : public internal::UnsignedFormatter<u8, 4, [](auto& s, u8 n) { internal::Formatu4dig<true>(s, n); }> {};
template<> class Formatter<io::u16_pad> : public internal::UnsignedFormatter<u16, 8, [](auto& s, u16 n) { internal::Formatu16<true>(s, n); }> {};
template<> class Formatter<io::u32_pad> : public internal::UnsignedFormatter<u32, 16, [](auto& s, u32 n) { internal::Formatu32<true>(s, n); }> {};
template<> class Formatter<io::u64_pad> : public internal::UnsignedFormatter<u64, 32, [](auto& s, u64 n) { internal::Formatu64<true>(s, n); }> {};
template<> class Formatter<io::u128_pad> : public internal::UnsignedFormatter<u128, 64, [](auto& s, u128 n) { internal::Formatu128<true>(s, n); }> {};
template<> class Formatter<io::u4dig_pad> : public internal::UnsignedFormatter<u16, 4, [](auto& s, u16 n) { internal::Formatu4dig<true>(s, n); }> {};
template<> class Formatter<io::u8dig_pad> : public internal::UnsignedFormatter<u32, 8, [](auto& s, u32 n) { internal::Formatu8dig<true>(s, n); }> {};
template<> class Formatter<io::u16dig_pad> : public internal::UnsignedFormatter<u64, 16, [](auto& s, u64 n) { internal::Formatu16dig<true>(s, n); }> {};
template<> class Formatter<float> : public internal::FloatFormatter<float> {};
template<> class Formatter<double> : public internal::FloatFormatter<double> {};
template<> class Formatter<long double> : public internal::FloatFormatter<long double> {};
#ifdef __STDCPP_FLOAT16_T__
template<> class Formatter<std::float16_t> : public internal::FloatFormatter<std::float16_t> {};
#endif
#ifdef __STDCPP_FLOAT32_T__
template<> class Formatter<std::float32_t> : public internal::FloatFormatter<std::float32_t> {};
#endif
#ifdef __STDCPP_FLOAT64_T__
template<> class Formatter<std::float64_t> : public internal::FloatFormatter<std::float64_t> {};
#endif
#ifdef __STDCPP_FLOAT128_T__
template<> class Formatter<std::float128_t> : public internal::FloatFormatter<std::float128_t> {};
#endif
#ifdef __STDCPP_BFLOAT16_T__
template<> class Formatter<std::bfloat16_t> : public internal::FloatFormatter<std::bfloat16_t> {};
#endif
#ifdef __SIZEOF_FLOAT128__
template<> class Formatter<__float128> : public internal::FloatFormatter<__float128> {};
#endif
template<> class Formatter<InvalidFloat16Tag> {};
template<> class Formatter<InvalidFloat128Tag> {};
template<> class Formatter<InvalidBfloat16Tag> {};
template<> class Formatter<bool> {
public:
  template<class Stream> constexpr void operator()(Stream&& stream, bool b) const {
    stream.reload(1);
    *stream.current() = '0' + b;
    stream.skip(1);
  }
};
template<> class Formatter<c8> {
public:
  template<class Stream> constexpr void operator()(Stream&& stream, c8 c) const {
    stream.reload(1);
    *stream.current() = c;
    stream.skip(1);
  }
};
template<> class Formatter<const c8*> {
public:
  template<class Stream> constexpr void operator()(Stream&& stream, const c8* s) const { operator()(stream, s, StrLen(s)); }
  template<class Stream> constexpr void operator()(Stream&& stream, const c8* s, u32 len) const {
    u32 avail = stream.avail();
    if(avail >= len) [[likely]] {
      MemoryCopy(stream.current(), s, len);
      stream.skip(len);
    } else {
      MemoryCopy(stream.current(), s, avail);
      len -= avail;
      s += avail;
      stream.skip(avail);
      while(len != 0) {
        stream.reload();
        avail = stream.avail();
        const u32 tmp = len < avail ? len : avail;
        MemoryCopy(stream.current(), s, tmp);
        len -= tmp;
        s += tmp;
        stream.skip(tmp);
      }
    }
  }
};
template<> class Formatter<c8*> : public Formatter<const c8*> {};
class NoOutTag {};
[[maybe_unused]] constexpr NoOutTag NoOut;
template<> class Formatter<NoOutTag> {
public:
  template<class Stream> constexpr void operator()(Stream&&, NoOutTag) const {}
};
template<class R> concept FormatableRange = std::ranges::forward_range<R> && requires { sizeof(Formatter<std::decay_t<std::ranges::range_value_t<R>>>) != 0; };
template<FormatableRange R> class Formatter<R> {
  template<class Stream, class T, class U> constexpr void print(Stream&& stream, T&& r, U&& sep) const {
    auto first = std::ranges::begin(r);
    auto last = std::ranges::end(r);
    if(!(first != last)) return;
    Formatter<std::decay_t<std::ranges::range_value_t<R>>> formatter;
    while(true) {
      formatter(stream, *first);
      ++first;
      if(first != last) {
        Formatter<std::decay_t<U>>()(stream, sep);
      } else break;
    }
  }
public:
  template<class Stream, class T> requires std::same_as<std::decay_t<T>, R> constexpr void operator()(Stream&& stream, T&& r) const { print(std::forward<Stream>(stream), std::forward<T>(r), ' '); }
  template<class Stream, class T, class U> requires std::same_as<std::decay_t<T>, R> constexpr void operator()(Stream&& stream, T&& r, U&& sep) const { print(std::forward<Stream>(stream), std::forward<T>(r), std::forward<U>(sep)); }
};
namespace internal {
template<class T, class U> constexpr bool FormatableTupleImpl = false;
template<class T, std::size_t... I> constexpr bool FormatableTupleImpl<T, std::integer_sequence<std::size_t, I...>> = (... && requires { sizeof(Formatter<std::decay_t<typename std::tuple_element<I, T>::type>>) != 0; });
} // namespace internal
template<class T> concept FormatableTuple = requires { std::tuple_size<T>::value; } && internal::FormatableTupleImpl<T, std::make_index_sequence<std::tuple_size<T>::value>>;
template<FormatableTuple T> requires (!FormatableRange<T>) class Formatter<T> {
  template<u32 I, class Stream, class U, class Sep> constexpr void print_element(Stream&& stream, U&& x, Sep&& sep) const {
    using std::get;
    using element_type = std::decay_t<std::tuple_element_t<I, T>>;
    if constexpr(requires { x.template get<I>(); }) Formatter<element_type>()(stream, x.template get<I>());
    else Formatter<element_type>()(stream, get<I>(x));
    if constexpr(I < std::tuple_size_v<T> - 1) Formatter<std::decay_t<Sep>>()(stream, sep);
  }
  template<class Stream, class U, class Sep> constexpr void print(Stream&& stream, U&& x, Sep&& sep) const {
    [&]<u32... I>(std::integer_sequence<u32, I...>) { (..., print_element<I>(stream, x, sep)); }(std::make_integer_sequence<u32, std::tuple_size_v<T>>());
  }
public:
  template<class Stream, class U> constexpr void operator()(Stream&& stream, U&& x) const { print(std::forward<Stream>(stream), std::forward<U>(x), ' '); }
  template<class Stream, class U, class Sep> constexpr void operator()(Stream&& stream, U&& x, Sep&& sep) const { print(std::forward<Stream>(stream), std::forward<U>(x), std::forward<Sep>(sep)); }
};
} // namespace gsh
