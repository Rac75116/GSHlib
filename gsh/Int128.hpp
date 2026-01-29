#pragma once
#include "TypeDef.hpp"
#include "Util.hpp"
#include "internal/Operation.hpp"
#include <compare>
namespace gsh {
namespace internal {
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
}
using i128 = __int128_t;
using u128 = __uint128_t;
}
