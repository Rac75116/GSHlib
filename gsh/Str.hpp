#pragma once
#include "Range.hpp"
#include "TypeDef.hpp"
#include <charconv>
#include <string>
#include <string_view>
namespace gsh {
using Str = std::basic_string<c8>;
using Str8 = std::basic_string<utf8>;
using Str16 = std::basic_string<utf16>;
using Str32 = std::basic_string<utf32>;
using Strw = std::basic_string<wc>;
using StrView = std::basic_string_view<c8>;
using StrView8 = std::basic_string_view<utf8>;
using StrView16 = std::basic_string_view<utf16>;
using StrView32 = std::basic_string_view<utf32>;
using StrVieww = std::basic_string_view<wc>;
template<class T> class Parser;
template<> class Parser<Str> {
public:
  template<class Stream> constexpr Str operator()(Stream&& stream) const {
    stream.reload(16);
    Str res;
    while(true) {
      const c8* e = stream.current();
      while(*e >= '!') ++e;
      const u32 len = e - stream.current();
      const u32 curlen = res.size();
      res.resize(curlen + len);
      MemoryCopy(res.data() + curlen, stream.current(), len);
      stream.skip(len);
      if(stream.avail() == 0) stream.reload();
      else break;
    }
    stream.skip(1);
    return res;
  }
  template<class Stream> constexpr Str operator()(Stream&& stream, u32 n) const {
    u32 rem = n;
    Str res;
    u32 avail = stream.avail();
    while(avail <= rem) {
      const u32 curlen = res.size();
      res.resize(curlen + avail);
      MemoryCopy(res.data() + curlen, stream.current(), avail);
      rem -= avail;
      stream.skip(avail);
      if(rem == 0) return res;
      stream.reload();
      avail = stream.avail();
    }
    const u32 curlen = res.size();
    res.resize(curlen + rem);
    MemoryCopy(res.data() + curlen, stream.current(), rem);
    stream.skip(rem + 1);
    return res;
  }
};
template<class T> class Formatter;
template<> class Formatter<StrView> {
public:
  template<class Stream> constexpr void operator()(Stream&& stream, const StrView& str) const {
    const c8* s = str.data();
    u32 len = str.size();
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
template<> class Formatter<Str> {
public:
  template<class Stream> constexpr void operator()(Stream&& stream, const Str& str) const { Formatter<StrView>{}(stream, StrView(str.data(), str.size())); }
};
template<class... Args> requires ((std::same_as<Args, c8> && ...) && sizeof...(Args) >= 1) constexpr Str UniteChars(Args... c) { return Str{c...}; }
template<std::integral T> constexpr Str NumToStr(const T& val, const i32 base = 10) {
  char buf[sizeof(T) * 8];
  char* last = std::to_chars(buf, buf + sizeof(T) * 8, val, base).ptr;
  return Str(buf, last);
}
template<std::floating_point T> constexpr Str NumToStr(const T& val, const i32 precision = 10) {
  char buf[64];
  auto result = std::to_chars(buf, buf + 64, val, std::chars_format::fixed, precision);
  if(result.ec != std::errc()) { throw Exception("gsh::ToStr / Conversion error in floating point to string."); }
  return Str(buf, result.ptr);
}
template<class T> constexpr T StrToNum(const StrView& str, const i32 base = 10) {
  T res{};
  auto result = std::from_chars(str.data(), str.data() + str.size(), res, base);
  if(result.ec != std::errc() || result.ptr != str.data() + str.size()) { throw Exception("gsh::FromStr / Conversion error in string to number."); }
  return res;
}
template<class T> constexpr T StrToNum(const Str& str, const i32 base = 10) { return StrToNum<T>(StrView(str), base); }
}
