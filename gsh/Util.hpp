#pragma once
#include "TypeDef.hpp"
#include "internal/Operation.hpp"
#include "internal/UtilMacro.hpp"
#include <bit>
#include <cstring>
#include <type_traits>
#include <utility>
namespace gsh {
[[noreturn]] inline void Unreachable() {
#if defined __GNUC__ || defined __clang__
  __builtin_unreachable();
#elif _MSC_VER
  __assume(false);
#else
  [[maybe_unused]] u32 n = 1 / 0;
#endif
};
GSH_INTERNAL_INLINE constexpr void Assume(const bool f) {
  if(std::is_constant_evaluated()) return;
#if defined __clang__
  __builtin_assume(f);
#elif defined __GNUC__
  if(!f) __builtin_unreachable();
#elif _MSC_VER
  __assume(f);
#else
  if(!f) Unreachable();
#endif
}
template<bool Likely = true> GSH_INTERNAL_INLINE constexpr bool Expect(const bool f) {
  if(std::is_constant_evaluated()) return f;
#if defined __GNUC__ || defined __clang__
  return __builtin_expect(f, Likely);
#else
  if constexpr(Likely) {
    if(f) [[likely]]
      return true;
    else return false;
  } else {
    if(f) [[unlikely]]
      return false;
    else return true;
  }
#endif
}
GSH_INTERNAL_INLINE constexpr bool Unpredictable(const bool f) {
  if(std::is_constant_evaluated()) return f;
#if defined __clang__
  return __builtin_unpredictable(f);
#elif defined __GNUC__
  return __builtin_expect_with_probability(f, 1, 0.5);
#else
  return f;
#endif
}
GSH_INTERNAL_INLINE inline void PreventConstexpr() noexcept {
  [[maybe_unused]] thread_local u8 dummy = 0;
  ++dummy;
}
class InPlaceTag {};
[[maybe_unused]] constexpr InPlaceTag InPlace;
template<class T> requires std::is_trivial_v<T> GSH_INTERNAL_INLINE constexpr void MemorySet(T* p, c8 byte, u32 len) {
  if(std::is_constant_evaluated()) {
    struct mem {
      c8 buf[sizeof(T)] = {};
    };
    mem init;
    for(u32 i = 0; i != sizeof(T); ++i) init.buf[i] = byte;
    for(u32 i = 0; i != len / sizeof(T); ++i) p[i] = std::bit_cast<T>(init);
    if(len % sizeof(T) != 0) {
      auto& ref = p[len / sizeof(T)];
      mem tmp = std::bit_cast<mem>(ref);
      for(u32 i = 0; i != len % sizeof(T); ++i) tmp.buf[i] = byte;
      ref = std::bit_cast<T>(tmp);
    }
  } else std::memset(p, byte, len);
}
template<class T> requires std::is_trivial_v<T> GSH_INTERNAL_INLINE constexpr u32 MemoryChar(T* p, c8 byte, u32 len) {
  if(std::is_constant_evaluated()) {
    struct mem {
      c8 buf[sizeof(T)] = {};
    };
    for(u32 i = 0; i != len / sizeof(T); ++i) {
      mem tmp = std::bit_cast<mem>(p[i]);
      for(u32 j = 0; j != sizeof(T); ++j) {
        if(tmp.buf[j] == byte) return i * sizeof(T) + j;
      }
    }
    if(len % sizeof(T) != 0) {
      mem tmp = std::bit_cast<mem>(p[len / sizeof(T)]);
      for(u32 i = 0; i != len % sizeof(T); ++i) {
        if(tmp.buf[i] == byte) return len / sizeof(T) * sizeof(T) + i;
      }
    }
    return 0xffffffff;
  } else {
    const void* tmp = std::memchr(p, byte, len);
    return (tmp == nullptr ? 0xffffffff : static_cast<const c8*>(tmp) - reinterpret_cast<const c8*>(p));
  }
}
template<class T, class U> requires std::is_trivial_v<T> GSH_INTERNAL_INLINE constexpr void MemoryCopy(T* GSH_INTERNAL_RESTRICT dst, U* GSH_INTERNAL_RESTRICT src, u32 len) {
  if(std::is_constant_evaluated()) {
    struct mem1 {
      c8 buf[sizeof(T)] = {};
    };
    struct mem2 {
      c8 buf[sizeof(U)] = {};
    };
    mem1 tmp1;
    mem2 tmp2;
    for(u32 i = 0; i != len; ++i) {
      if(i % sizeof(U) == 0) tmp2 = std::bit_cast<mem2>(src[i / sizeof(U)]);
      tmp1.buf[i % sizeof(T)] = tmp2.buf[i % sizeof(U)];
      if((i + 1) % sizeof(T) == 0) {
        dst[i / sizeof(T)] = std::bit_cast<T>(tmp1);
        tmp1 = mem1{};
      }
    }
    if(len % sizeof(T) != 0) {
      mem1 tmp3 = std::bit_cast<mem1>(dst[len / sizeof(T)]);
      for(u32 i = 0; i != len % sizeof(T); ++i) tmp3.buf[i] = tmp1.buf[i];
      dst[len / sizeof(T)] = std::bit_cast<T>(tmp3);
    }
  } else std::memcpy(dst, src, len);
}
/*
template<class T, class U>
    requires std::is_trivially_copyable_v<T> && std::is_trivially_copyable_v<U>
GSH_INTERNAL_INLINE constexpr void MemoryMove(T* dst, U* src, u32 len) {
    if (std::is_constant_evaluated()) {
    } else std::memmove(dst, src, len);
}
*/
GSH_INTERNAL_INLINE constexpr u32 StrLen(const c8* p) {
  if(std::is_constant_evaluated()) {
    auto q = p;
    while(*q != '\0') ++q;
    return q - p;
  } else return std::strlen(p);
}
namespace internal {
template<u32 N, class First, class... Tail> class TypeAtImpl : public TypeAtImpl<N - 1, Tail...> {};
template<class T, class... Types> class TypeAtImpl<0, T, Types...> {
public:
  using type = T;
};
} // namespace internal
template<u32 N, class... Types> using TypeAt = typename internal::TypeAtImpl<N, Types...>::type;
template<class... Types> class TypeArr {
public:
  constexpr static u32 size() noexcept { return sizeof...(Types); }
  template<u32 N> using type = std::conditional_t<(N < sizeof...(Types)), TypeAt<N, Types...>, void>;
};
template<> class TypeArr<> {
public:
  constexpr static u32 size() noexcept { return 0; }
  template<u32 N> using type = void;
};
} // namespace gsh
