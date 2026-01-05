#pragma once
#include "Functional.hpp"
#include "TypeDef.hpp"
#include <bit>
#include <ctime>
#include <immintrin.h>
#include <ranges>
namespace gsh {
namespace internal {
constexpr u64 Splitmix(u64 x) {
  u64 z = (x + 0x9e3779b97f4a7c15);
  z = (z ^ (z >> 30)) * 0xbf58476d1ce4e5b9;
  z = (z ^ (z >> 27)) * 0x94d049bb133111eb;
  return z ^ (z >> 31);
}
} // namespace internal
// @brief 64bit pseudo random number generator using xoroshiro128+
class Rand64 {
  u64 s0, s1;
public:
  using result_type = u64;
  static constexpr u32 word_size = sizeof(result_type) * 8;
  static constexpr result_type default_seed = 0xcafef00dd15ea5e5;
  constexpr Rand64() : Rand64(default_seed) {}
  constexpr explicit Rand64(result_type value) : s0(internal::Splitmix(value)), s1(internal::Splitmix(value << 32 | (value & 0xffffffffu))) {}
  constexpr result_type operator()() {
    u64 t0 = s0, t1 = s1;
    const u64 res = t0 + t1;
    t1 ^= t0;
    s0 = std::rotr(t0, 9) ^ t1 ^ (t1 << 14);
    s1 = std::rotr(t1, 28);
    return res;
  };
  constexpr void discard(u64 z) {
    for(u64 i = 0; i < z; ++i) operator()();
  }
  static constexpr result_type max() { return 18446744073709551615u; }
  static constexpr result_type min() { return 0; }
  constexpr void seed(result_type value = default_seed) {
    s0 = internal::Splitmix(value);
    s1 = internal::Splitmix(value << 32 | (value & 0xffffffffu));
  }
  friend constexpr bool operator==(Rand64 x, Rand64 y) { return x.s0 == y.s0 && x.s1 == y.s1; }
};
// @brief 32bit pseudo random number generator using Permuted congruential generator
class Rand32 {
  u64 val;
public:
  using result_type = u32;
  static constexpr u32 word_size = sizeof(result_type) * 8;
  static constexpr result_type default_seed = 0xcafef00d;
  constexpr Rand32() : Rand32(default_seed) {}
  constexpr explicit Rand32(result_type value) : val(internal::Splitmix(value)) {}
  constexpr result_type operator()() {
    u64 x = val;
    const i32 count = x >> 61;
    val = x * 0xcafef00dd15ea5e5;
    x ^= x >> 22;
    return x >> (22 + count);
  };
  constexpr void discard(u64 z) {
    u64 pow = 0xcafef00dd15ea5e5;
    while(z != 0) {
      if(z & 1) val *= pow;
      z >>= 1;
      pow *= pow;
    }
  }
  static constexpr result_type max() { return 4294967295u; }
  static constexpr result_type min() { return 0; }
  constexpr void seed(result_type value = default_seed) { val = internal::Splitmix(value); }
  friend constexpr bool operator==(Rand32 x, Rand32 y) { return x.val == y.val; }
};
class FastRand32 {
  u64 val;
public:
  using result_type = u32;
  static constexpr u32 word_size = sizeof(result_type) * 8;
  static constexpr result_type default_seed = 0xcafef00d;
  constexpr FastRand32() : FastRand32(default_seed) {}
  constexpr explicit FastRand32(result_type value) : val(internal::Splitmix(value)) {}
  constexpr result_type operator()() {
    val = val * 0xcafef00dd15ea5e5;
    return (u32)(val >> 32);
  }
  constexpr void discard(u64 z) {
    u64 pow = 0xcafef00dd15ea5e5;
    while(z != 0) {
      if(z & 1) val *= pow;
      z >>= 1;
      pow *= pow;
    }
  }
  static constexpr result_type max() { return 4294967295u; }
  static constexpr result_type min() { return 0; }
  constexpr void seed(result_type value = default_seed) { val = internal::Splitmix(value); }
  friend constexpr bool operator==(FastRand32 x, FastRand32 y) { return x.val == y.val; }
};
// @brief Generate 32bit uniform random numbers in [0, max) (https://www.pcg-random.org/posts/bounded-rands.html)
template<class URBG> constexpr u32 Uniform32(URBG&& g, u32 max) { return (static_cast<u64>(std::invoke(g) & 4294967295u) * max) >> 32; }
// @brief Generate 32bit uniform random numbers in [min, max) (https://www.pcg-random.org/posts/bounded-rands.html)
template<class URBG> constexpr u32 Uniform32(URBG&& g, u32 min, u32 max) { return static_cast<u32>((static_cast<u64>(std::invoke(g) & 4294967295u) * (max - min)) >> 32) + min; }
// @brief Generate 64bit uniform random numbers in [0, max) (https://www.pcg-random.org/posts/bounded-rands.html)
template<class URBG> constexpr u64 Uniform64(URBG&& g, u64 max) { return (static_cast<u128>(std::invoke(g)) * max) >> 64; }
// @brief Generate 64bit uniform random numbers in [min, max) (https://www.pcg-random.org/posts/bounded-rands.html)
template<class URBG> constexpr u64 Uniform64(URBG&& g, u64 min, u64 max) { return static_cast<u64>((static_cast<u128>(std::invoke(g)) * (max - min)) >> 64) + min; }
template<std::ranges::random_access_range R, class URBG> constexpr void Shuffle(R&& r, URBG&& g) {
  u32 sz = std::ranges::size(r);
  auto itr = std::ranges::begin(r);
  for(u32 i = 0; i != sz; ++i, ++itr) { std::ranges::swap(*itr, *std::ranges::next(itr, Uniform32(g, sz - i))); }
}
template<class URBG> constexpr u32 UnbiasedUniform32(URBG&& g, u32 max) {
  u32 mask = ~0u;
  --max;
  mask >>= std::countl_zero(max | 1);
  u32 x;
  do { x = std::invoke(g) & mask; } while(x > max);
  return x;
}
template<class URBG> constexpr u32 UnbiasedUniform32(URBG&& g, u32 min, u32 max) { return min + UnbiasedUniform32(g, max - min); }
template<class URBG> constexpr u64 UnbiasedUniform64(URBG&& g, u64 max) {
  u64 mask = ~0ull;
  --max;
  mask >>= std::countl_zero(max | 1);
  u64 x;
  do { x = std::invoke(g) & mask; } while(x > max);
  return x;
}
template<class URBG> constexpr u32 UnbiasedUniform64(URBG&& g, u64 min, u64 max) { return min + UnbiasedUniform64(g, max - min); }
//https://speakerdeck.com/hole/rand01?slide=31
template<class URBG> constexpr f32 Canocicaled32(URBG&& g) { return std::bit_cast<f32>((127u << 23) | (static_cast<u32>(std::invoke(g)) & 0x7fffff)) - 1.0f; }
template<class URBG> constexpr f32 Uniformf32(URBG&& g, f32 max) { return Canocicaled32(g) * max; }
template<class URBG> constexpr f32 Uniformf32(URBG&& g, f32 min, f32 max) { return Canocicaled32(g) * (max - min) + min; }
template<class URBG> constexpr f64 Canocicaled64(URBG&& g) { return std::bit_cast<f64>((1023ull << 52) | (static_cast<u64>(std::invoke(g)) & 0xfffffffffffffull)) - 1.0; }
template<class URBG> constexpr f64 Uniformf64(URBG&& g, f64 max) { return Canocicaled64(g) * max; }
template<class URBG> constexpr f64 Uniformf64(URBG&& g, f64 min, f64 max) { return Canocicaled64(g) * (max - min) + min; }
} // namespace gsh
