#pragma once
#include "Exception.hpp"
#include "Int128.hpp"
#include "TypeDef.hpp"
#include "Util.hpp"
#include <bit>
#include <cerrno>
#include <cstdlib>
#include <ranges>
#include <utility>
namespace gsh {
namespace io {
struct i4dig;
struct u4dig;
struct i8dig;
struct u8dig;
struct i16dig;
struct u16dig;
} // namespace io
template<class T> class Parser;
namespace internal {
template<class Stream> constexpr u16 Parseu4dig(Stream&& stream) {
  u32 v;
  MemoryCopy(&v, stream.current(), 4);
  v ^= 0x30303030;
  i32 tmp = std::countr_zero(v & 0xf0f0f0f0) >> 3;
  v <<= (32 - (tmp << 3));
  v = (v * 10 + (v >> 8)) & 0x00ff00ff;
  v = (v * 100 + (v >> 16)) & 0x0000ffff;
  stream.skip(tmp + 1);
  return v;
}
template<class Stream> constexpr u32 Parseu8dig(Stream&& stream) {
  u64 v;
  MemoryCopy(&v, stream.current(), 8);
  v ^= 0x3030303030303030;
  i32 tmp = std::countr_zero(v & 0xf0f0f0f0f0f0f0f0) >> 3;
  v <<= (64 - (tmp << 3));
  v = (v * 10 + (v >> 8)) & 0x00ff00ff00ff00ff;
  v = (v * 100 + (v >> 16)) & 0x0000ffff0000ffff;
  v = (v * 10000 + (v >> 32)) & 0x00000000ffffffff;
  stream.skip(tmp + 1);
  return v;
}
template<class Stream> constexpr u8 Parseu8(Stream&& stream) { return Parseu4dig(stream); }
template<class Stream> constexpr u16 Parseu16(Stream&& stream) { return Parseu8dig(stream); }
template<class Stream> constexpr u32 Parseu32(Stream&& stream) {
  u32 res = 0;
  u64 buf[2];
  MemoryCopy(buf, stream.current(), 16);
  u64 rem;
  {
    buf[0] ^= 0x3030303030303030, buf[1] ^= 0x3030303030303030;
    u64 v = buf[0];
    rem = v;
    if(!(v & 0xf0f0f0f0f0f0f0f0)) [[likely]] {
      stream.skip(8);
      rem = buf[1];
      v = (v * 10 + (v >> 8)) & 0x00ff00ff00ff00ff;
      v = (v * 100 + (v >> 16)) & 0x0000ffff0000ffff;
      v = (v * 10000 + (v >> 32)) & 0x00000000ffffffff;
      res = v;
    }
  }
  {
    u32 v = rem;
    if(!(v & 0xf0f0f0f0)) {
      rem >>= 32;
      v = (v * 10 + (v >> 8)) & 0x00ff00ff;
      v = (v * 100 + (v >> 16)) & 0x0000ffff;
      res = 10000 * res + v;
      stream.skip(4);
    }
  }
  {
    u32 v = rem & 0xffff;
    if(!(v & 0xf0f0)) {
      rem >>= 16;
      v = (v * 10 + (v >> 8)) & 0x00ff;
      res = 100 * res + v;
      stream.skip(2);
    }
  }
  {
    const bool f = !(rem & 0xf0);
    res = f ? 10 * res + (rem & 0xff) : res;
    stream.skip(f + 1);
  }
  return res;
};
template<class Stream> constexpr u64 Parseu64(Stream&& stream) {
  c8 const* cur = stream.current();
  u64 v;
  MemoryCopy(&v, cur, 8);
  if(!((v ^= 0x3030303030303030) & 0xf0f0f0f0f0f0f0f0)) {
    u64 u;
    MemoryCopy(&u, cur + 8, 8);
    v = (v * 10 + (v >> 8)) & 0x00ff00ff00ff00ff;
    v = (v * 100 + (v >> 16)) & 0x0000ffff0000ffff;
    v = (v * 10000 + (v >> 32)) & 0x00000000ffffffff;
    if(!((u ^= 0x3030303030303030) & 0xf0f0f0f0f0f0f0f0)) {
      u = (u * 10 + (u >> 8)) & 0x00ff00ff00ff00ff;
      u = (u * 100 + (u >> 16)) & 0x0000ffff0000ffff;
      u = (u * 10000 + (u >> 32)) & 0x00000000ffffffff;
      v = v * 100000000 + u;
      u32 rem;
      MemoryCopy(&rem, cur + 16, 4);
      rem ^= 0x30303030;
      if((rem & 0xf0f0f0f0) == 0) [[unlikely]] {
        rem = (rem * 10 + (rem >> 8)) & 0x00ff00ff;
        rem = (rem * 100 + (rem >> 16)) & 0x0000ffff;
        v = v * 10000 + rem;
        stream.skip(21);
      } else if((rem & 0xf0f0f0) == 0) {
        v = v * 1000 + ((rem & 0xff) * 100) + (((rem * 2561) & 0xff0000) >> 16);
        stream.skip(20);
      } else if((rem & 0xf0f0) == 0) {
        v = v * 100 + (((rem >> 8) + (rem * 10)) & 0xff);
        stream.skip(19);
      } else if((rem & 0xf0) == 0) {
        v = v * 10 + (rem & 0x0000000f);
        stream.skip(18);
      } else {
        stream.skip(17);
      }
      return v;
    } else {
      u32 c = 9;
      while(!(u & 0xf0)) {
        v = v * 10 + (u & 0xff);
        u >>= 8;
        ++c;
      }
      stream.skip(c);
      return v;
    }
  } else {
    i32 tmp = std::countr_zero(v & 0xf0f0f0f0f0f0f0f0) >> 3;
    v <<= (64 - (tmp << 3));
    v = (v * 10 + (v >> 8)) & 0x00ff00ff00ff00ff;
    v = (v * 100 + (v >> 16)) & 0x0000ffff0000ffff;
    v = (v * 10000 + (v >> 32)) & 0x00000000ffffffff;
    stream.skip(tmp + 1);
    return v;
  }
}
template<class Stream> constexpr u128 Parseu128(Stream&& stream) {
  u128 res = 0;
  for(i64 i = 0; i != 4; ++i) {
    u64 v;
    MemoryCopy(&v, stream.current(), 8);
    if(((v ^= 0x3030303030303030) & 0xf0f0f0f0f0f0f0f0) != 0) break;
    v = (v * 10 + (v >> 8)) & 0x00ff00ff00ff00ff;
    v = (v * 100 + (v >> 16)) & 0x0000ffff0000ffff;
    v = (v * 10000 + (v >> 32)) & 0x00000000ffffffff;
    if(i == 0) res = v;
    else res = res * 100000000 + v;
    stream.skip(8);
  }
  u64 buf;
  MemoryCopy(&buf, stream.current(), 8);
  buf ^= 0x3030303030303030;
  u64 res2 = 0, pw = 1;
  {
    u32 v = buf;
    if(!(v & 0xf0f0f0f0)) {
      buf >>= 32;
      v = (v * 10 + (v >> 8)) & 0x00ff00ff;
      v = (v * 100 + (v >> 16)) & 0x0000ffff;
      res2 = v;
      pw = 10000;
      stream.skip(4);
    }
  }
  {
    u32 v = buf & 0xffff;
    if(!(v & 0xf0f0)) {
      buf >>= 16;
      v = (v * 10 + (v >> 8)) & 0x00ff;
      res2 = res2 * 100 + v;
      pw *= 100;
      stream.skip(2);
    }
  }
  {
    const c8 v = buf;
    const bool f = (v & 0xf0) == 0;
    const volatile auto tmp1 = pw * 10, tmp2 = res2 * 10 + v;
    const auto tmp3 = tmp1, tmp4 = tmp2;
    pw = f ? tmp3 : pw;
    res2 = f ? tmp4 : res2;
    stream.skip(f + 1);
  }
  return res * pw + res2;
}
} // namespace internal
template<> class Parser<u8> {
public:
  template<class Stream> constexpr u8 operator()(Stream&& stream) const {
    stream.reload(8);
    return internal::Parseu8(stream);
  }
};
template<> class Parser<i8> {
public:
  template<class Stream> constexpr i8 operator()(Stream&& stream) const {
    stream.reload(8);
    bool neg = *stream.current() == '-';
    stream.skip(neg);
    i8 tmp = internal::Parseu8(stream);
    if(neg) tmp = -tmp;
    return tmp;
  }
};
template<> class Parser<u16> {
public:
  template<class Stream> constexpr u16 operator()(Stream&& stream) const {
    stream.reload(8);
    return internal::Parseu16(stream);
  }
};
template<> class Parser<i16> {
public:
  template<class Stream> constexpr i16 operator()(Stream&& stream) const {
    stream.reload(8);
    bool neg = *stream.current() == '-';
    stream.skip(neg);
    i16 tmp = internal::Parseu16(stream);
    if(neg) tmp = -tmp;
    return tmp;
  }
};
template<> class Parser<u32> {
public:
  template<class Stream> constexpr u32 operator()(Stream&& stream) const {
    stream.reload(16);
    return internal::Parseu32(stream);
  }
};
template<> class Parser<i32> {
public:
  template<class Stream> constexpr i32 operator()(Stream&& stream) const {
    stream.reload(16);
    bool neg = *stream.current() == '-';
    stream.skip(neg);
    i32 tmp = internal::Parseu32(stream);
    if(neg) tmp = -tmp;
    return tmp;
  }
};
template<> class Parser<u64> {
public:
  template<class Stream> constexpr u64 operator()(Stream&& stream) const {
    stream.reload(32);
    return internal::Parseu64(stream);
  }
};
template<> class Parser<i64> {
public:
  template<class Stream> constexpr i64 operator()(Stream&& stream) const {
    stream.reload(32);
    bool neg = *stream.current() == '-';
    stream.skip(neg);
    i64 tmp = internal::Parseu64(stream);
    if(neg) tmp = -tmp;
    return tmp;
  }
};
template<> class Parser<u128> {
public:
  template<class Stream> constexpr u128 operator()(Stream&& stream) const {
    stream.reload(64);
    return internal::Parseu128(stream);
  }
};
template<> class Parser<i128> {
public:
  template<class Stream> constexpr i128 operator()(Stream&& stream) const {
    stream.reload(64);
    bool neg = *stream.current() == '-';
    stream.skip(neg);
    i128 tmp = internal::Parseu128(stream);
    if(neg) tmp = -tmp;
    return tmp;
  }
};
template<> class Parser<io::u4dig> {
public:
  using value_type = u16;
  template<class Stream> constexpr u16 operator()(Stream&& stream) const {
    stream.reload(8);
    return internal::Parseu4dig(stream);
  }
};
template<> class Parser<io::i4dig> {
public:
  using value_type = i16;
  template<class Stream> constexpr i16 operator()(Stream&& stream) const {
    stream.reload(8);
    bool neg = *stream.current() == '-';
    stream.skip(neg);
    i16 tmp = internal::Parseu4dig(stream);
    if(neg) tmp = -tmp;
    return tmp;
  }
};
template<> class Parser<io::u8dig> {
public:
  using value_type = u32;
  template<class Stream> constexpr u32 operator()(Stream&& stream) const {
    stream.reload(16);
    return internal::Parseu8dig(stream);
  }
};
template<> class Parser<io::i8dig> {
public:
  using value_type = i32;
  template<class Stream> constexpr i32 operator()(Stream&& stream) const {
    stream.reload(16);
    bool neg = *stream.current() == '-';
    stream.skip(neg);
    i32 tmp = internal::Parseu8dig(stream);
    if(neg) tmp = -tmp;
    return tmp;
  }
};
template<> class Parser<c8> {
public:
  template<class Stream> constexpr c8 operator()(Stream&& stream) const {
    stream.reload(2);
    c8 tmp = *stream.current();
    stream.skip(2);
    return tmp;
  }
};
template<> class Parser<c8*> {
public:
  template<class Stream> constexpr c8* operator()(Stream&& stream, c8* s) const {
    stream.reload(16);
    c8* c = s;
    while(true) {
      const c8* e = stream.current();
      while(*e >= '!') ++e;
      const i64 len = static_cast<i64>(e - stream.current());
      MemoryCopy(c, stream.current(), len);
      stream.skip(len);
      c += len;
      if(stream.avail() == 0) stream.reload();
      else break;
    }
    stream.skip(1);
    *c = '\0';
    return s;
  }
  template<class Stream> constexpr c8* operator()(Stream&& stream, c8* s, i64 n) const {
    i64 rem = n;
    c8* c = s;
    i64 avail = stream.avail();
    while(avail <= rem) {
      MemoryCopy(c, stream.current(), avail);
      c += avail;
      rem -= avail;
      stream.skip(avail);
      if(rem == 0) {
        *c = '\0';
        return s;
      }
      stream.reload();
      avail = stream.avail();
    }
    MemoryCopy(c, stream.current(), rem);
    c += rem;
    stream.skip(rem + 1);
    *c = '\0';
    return s;
  }
};
template<> class Parser<float> {
public:
  template<class Stream> constexpr float operator()(Stream&& stream) const {
    stream.reload(128);
    const c8* cur = stream.current();
    c8* end = nullptr;
    float res = std::strtof(cur, &end);
    if(errno) { throw Exception("gsh::Parser<float>::operator() / Failed to parse."); }
    stream.skip(end - cur + 1);
    return res;
  }
};
template<> class Parser<double> {
public:
  template<class Stream> constexpr double operator()(Stream&& stream) const {
    stream.reload(128);
    const c8* cur = stream.current();
    c8* end = nullptr;
    double res = std::strtod(cur, &end);
    if(errno) { throw Exception("gsh::Parser<double>::operator() / Failed to parse."); }
    stream.skip(end - cur + 1);
    return res;
  }
};
template<> class Parser<long double> {
public:
  template<class Stream> constexpr long double operator()(Stream&& stream) const {
    stream.reload(128);
    const c8* cur = stream.current();
    c8* end = nullptr;
    long double res = std::strtold(cur, &end);
    if(errno) { throw Exception("gsh::Parser<long double>::operator() / Failed to parse."); }
    stream.skip(end - cur + 1);
    return res;
  }
};
namespace internal {
template<class T, class P, class Stream, class... Args> struct ParsingIterator {
  using value_type = T;
  using difference_type = i64;
  using pointer = T*;
  using reference = T&;
  i64 n;
  P* ref;
  Stream* stream;
  std::tuple<Args...>* args;
  constexpr ParsingIterator() noexcept : ParsingIterator(0, nullptr, nullptr, nullptr) {}
  constexpr ParsingIterator(i64 m, P* r, Stream* s, std::tuple<Args...>* a) noexcept : n(m), ref(r), stream(s), args(a) {}
  GSH_INTERNAL_INLINE friend constexpr i64 operator-(const ParsingIterator& a, const ParsingIterator& b) noexcept { return a.n - b.n; }
  GSH_INTERNAL_INLINE friend constexpr bool operator==(const ParsingIterator& a, const ParsingIterator& b) noexcept { return a.n == b.n; }
  GSH_INTERNAL_INLINE constexpr ParsingIterator& operator++() noexcept { return ++n, *this; }
  GSH_INTERNAL_INLINE constexpr ParsingIterator operator++(int) noexcept { return {n++, ref, stream, args}; }
  GSH_INTERNAL_INLINE constexpr T operator*() const {
    return [this]<std::size_t... I>(std::index_sequence<I...>) -> T { return (*ref)(*stream, std::get<I>(*args)...); }(std::make_index_sequence<sizeof...(Args)>());
  }
};
} // namespace internal
template<class R> concept ParsableRange = std::ranges::forward_range<R> && requires { sizeof(Parser<std::decay_t<std::ranges::range_value_t<R>>>) != 0; };
template<ParsableRange R> class Parser<R> {
public:
  template<class Stream, class... Args> constexpr R operator()(Stream&& stream, i64 len, Args&&... args) const {
    Parser<std::ranges::range_value_t<R>> p;
    std::tuple<Args...> a(std::forward<Args>(args)...);
    using iter = internal::ParsingIterator<std::ranges::range_value_t<R>, decltype(p), std::remove_cvref_t<Stream>, Args...>;
    return R(iter(0, &p, &stream, &a), iter(len, nullptr, nullptr, nullptr));
  }
};
/*
namespace internal {
    template<class T, class U> constexpr bool ParsableTupleImpl = false;
    template<class T, std::size_t... I> constexpr bool ParsableTupleImpl<T, std::integer_sequence<std::size_t, I...>> = (... && requires { sizeof(Parser<std::decay_t<typename std::tuple_element<I, T>::type>>) != 0; });
}  // namespace internal
template<class T> concept ParsableTuple = requires { std::tuple_size<T>::value; } && internal::ParsableTupleImpl<T, std::make_index_sequence<std::tuple_size<T>::value>>;
template<ParsableTuple T>
    requires(!ParsableRange<T>)
class Parser<T> {
public:
    template<class Stream> constexpr T operator()(Stream&&& stream) const {}
};
*/
} // namespace gsh
