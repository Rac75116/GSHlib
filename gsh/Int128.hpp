#pragma once
#include "TypeDef.hpp"
#include "Util.hpp"
#include "internal/Operation.hpp"
#include <compare>
namespace gsh {
namespace internal {
GSH_INTERNAL_INLINE constexpr std::pair<u64, u64> Mulu128(u64 muler, u64 mulnd) noexcept {
  __uint128_t tmp = static_cast<__uint128_t>(muler) * mulnd;
  return {tmp >> 64, tmp};
}
GSH_INTERNAL_INLINE constexpr u64 Mulu128High(u64 muler, u64 mulnd) noexcept { return static_cast<u64>((static_cast<__uint128_t>(muler) * mulnd) >> 64); }
GSH_INTERNAL_INLINE constexpr std::pair<u64, u64> Divu128(u64 high, u64 low, u64 div) noexcept {
  if constexpr(sizeof(void*) == 8) {
    if(!std::is_constant_evaluated()) {
      u64 res, rem;
      __asm__("divq %[v]" : "=a"(res), "=d"(rem) : [v] "r"(div), "a"(low), "d"(high));
      return {res, rem};
    }
  }
  __uint128_t n = (static_cast<__uint128_t>(high) << 64 | low);
  __uint128_t res = n / div;
  return {res, n - res * div};
}
GSH_INTERNAL_INLINE constexpr std::pair<u64, u64> Divu128(u64 high, u64 low, u64 dhigh, u64 dlow) noexcept {
  if(dhigh == 0) {
    if(high >= dlow) {
      u64 qh = high / dlow, r = high % dlow;
      u64 ql = internal::Divu128(r, low, dlow).first;
      high = qh, low = ql;
    } else {
      low = internal::Divu128(high, low, dlow).first;
      high = 0;
    }
  } else if(high >= dhigh) {
    Assume(dhigh != 0);
    i32 s = std::countl_zero(dhigh);
    if(s != 0) {
      u64 yh = dhigh << s | dlow >> (64 - s), yl = dlow << s;
      auto [q, r] = internal::Divu128(high >> (64 - s), high << s | low >> (64 - s), yh);
      auto [mh, ml] = internal::Mulu128(q, yl);
      low = q - (mh >= r && (q >= (low << s) || mh != r));
      high = 0;
    } else {
      low = (high > dhigh || low >= dlow);
      high = 0;
    }
  } else {
    low = 0;
    high = 0;
  }
  return {high, low};
}
GSH_INTERNAL_INLINE constexpr std::pair<u64, u64> Modu128(u64 high, u64 low, u64 dhigh, u64 dlow) noexcept {
  if(dhigh == 0) {
    low = internal::Divu128(high % dlow, low, dlow).second;
    high = 0;
  } else if(high >= dhigh) {
    Assume(dhigh != 0);
    i32 s = std::countl_zero(dhigh);
    if(s != 0) {
      u64 yh = dhigh << s | dlow >> (64 - s), yl = dlow << s;
      auto [q, r] = internal::Divu128(high >> (64 - s), high << s | low >> (64 - s), yh);
      auto [mh, ml] = internal::Mulu128(q, yl);
      u64 d = q - (mh >= r && (q >= (low << s) || mh != r));
      auto [dh, dl] = internal::Mulu128(d, dlow);
      high -= dh + d * dhigh;
      high -= low < dl;
      low -= dl;
    } else if(high > dhigh || low >= dlow) {
      high -= dhigh;
      high -= low < dlow;
      low -= dlow;
    }
  }
  return {high, low};
}
GSH_INTERNAL_INLINE constexpr u64 ShiftLeft128High(u64 high, u64 low, i32 shift) noexcept {
  Assume(0 <= shift && shift < 64);
  return high << shift | (low >> 1 >> (63 - shift));
}
GSH_INTERNAL_INLINE constexpr u64 ShiftRight128Low(u64 high, u64 low, i32 shift) noexcept {
  Assume(0 <= shift && shift < 64);
  return low >> shift | (high << 1 << (63 - shift));
}
} // namespace internal
using i128 = __int128_t;
using u128 = __uint128_t;
} // namespace gsh
