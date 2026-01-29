#pragma once
#include "Algorithm.hpp"
#include "Int128.hpp"
#include "Modint.hpp"
#include "Numeric.hpp"
#include "Random.hpp"
#include "TypeDef.hpp"
#include "Util.hpp"
#include "Vec.hpp"
namespace gsh {
namespace internal {
struct IsPrime8 {
  constexpr static u64 flag_table[4] = {2891462833508853932u, 9223979663092122248u, 9234666804958202376u, 577166812715155618u};
  GSH_INTERNAL_INLINE constexpr static bool calc(const u8 n) noexcept { return (flag_table[n / 64] >> (n % 64)) & 1; }
};
/**
 * 
 * The algorithm in this library is based on Bradley Berg's method.
 * See this page for more information: https://www.techneon.com/download/is.prime.64.base.data
 * 
 * Copyright 2018 Bradley Berg   < (My last name) @ t e c h n e o n . c o m >
 *
 * Permission to use, copy, modify, and distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 *
 * This algorithm is deliberately unpatented. The license above also
 * lets you even freely use it in commercial code.
 * 
 * Primality testing using a hash table of bases originated with Steven Worley.
 * 
**/
struct IsPrime32 {
  // clang-format off
        constexpr static u16 bases[] = {
1216,1836,8885,4564,10978,5228,15613,13941,1553,173,3615,3144,10065,9259,233,2362,6244,6431,10863,5920,6408,6841,22124,2290,45597,6935,4835,7652,1051,445,5807,842,1534,22140,1282,1733,347,6311,14081,11157,186,703,9862,15490,1720,17816,10433,49185,2535,9158,2143,2840,664,29074,24924,1035,41482,1065,10189,8417,130,4551,5159,48886,
786,1938,1013,2139,7171,2143,16873,188,5555,42007,1045,3891,2853,23642,148,3585,3027,280,3101,9918,6452,2716,855,990,1925,13557,1063,6916,4965,4380,587,3214,1808,1036,6356,8191,6783,14424,6929,1002,840,422,44215,7753,5799,3415,231,2013,8895,2081,883,3855,5577,876,3574,1925,1192,865,7376,12254,5952,2516,20463,186,
5411,35353,50898,1084,2127,4305,115,7821,1265,16169,1705,1857,24938,220,3650,1057,482,1690,2718,4309,7496,1515,7972,3763,10954,2817,3430,1423,714,6734,328,2581,2580,10047,2797,155,5951,3817,54850,2173,1318,246,1807,2958,2697,337,4871,2439,736,37112,1226,527,7531,5418,7242,2421,16135,7015,8432,2605,5638,5161,11515,14949,
748,5003,9048,4679,1915,7652,9657,660,3054,15469,2910,775,14106,1749,136,2673,61814,5633,1244,2567,4989,1637,1273,11423,7974,7509,6061,531,6608,1088,1627,160,6416,11350,921,306,18117,1238,463,1722,996,3866,6576,6055,130,24080,7331,3922,8632,2706,24108,32374,4237,15302,287,2296,1220,20922,3350,2089,562,11745,163,11951};
  // clang-format on
  GSH_INTERNAL_INLINE constexpr static bool calc(const u32 x) noexcept {
    internal::MontgomeryModint64Impl mint;
    mint.set(x);
    const u32 h = x * 0xad625b89;
    u32 d = x - 1;
    auto pow = mint.raw(bases[h >> 24]);
    u32 s = std::countr_zero(d);
    d >>= s;
    const auto one = mint.one(), mone = mint.neg(one);
    auto cur = one;
    while(d) {
      auto tmp = mint.mul(pow, pow);
      if(d & 1) cur = mint.mul(cur, pow);
      pow = tmp;
      d >>= 1;
    }
    if(mint.same(cur, one)) return true;
    while(--s && !mint.same(cur, mone)) cur = mint.mul(cur, cur);
    return mint.same(cur, mone);
  }
};
struct IsPrime64 {
  GSH_INTERNAL_INLINE constexpr static bool calc(const u64 x) noexcept {
    internal::MontgomeryModint64Impl mint;
    mint.set(x);
    const u32 S = std::countr_zero(x - 1);
    const u64 D = (x - 1) >> S;
    const auto one = mint.one(), mone = mint.neg(one);
    auto test2 = [&](u64 base1, u64 base2) {
      auto a = one, b = one;
      auto c = mint.build(base1), d = mint.build(base2);
      u64 ex = D;
      while(ex) {
        auto e = mint.mul(c, c), f = mint.mul(d, d);
        if(ex & 1) a = mint.mul(a, e), b = mint.mul(b, f);
        c = e, d = f;
        ex >>= 1;
      }
      bool res1 = mint.same(a, one) || mint.same(a, mone);
      bool res2 = mint.same(b, one) || mint.same(b, mone);
      if(!(res1 && res2)) {
        for(u32 i = 0; i != S - 1; ++i) {
          a = mint.mul(a, a), b = mint.mul(b, b), c = mint.mul(c, c);
          res1 |= mint.same(a, mone), res2 |= mint.same(b, mone);
        }
        if(!res1 || !res2) return false;
      }
      return true;
    };
    auto test3 = [&](u64 base1, u64 base2, u64 base3) {
      auto a = one, b = one, c = one;
      auto d = mint.build(base1), e = mint.build(base2), f = mint.build(base3);
      u64 ex = D;
      while(ex) {
        const auto g = mint.mul(d, d), h = mint.mul(e, e), i = mint.mul(f, f);
        if(ex & 1) a = mint.mul(a, d), b = mint.mul(b, e), c = mint.mul(c, f);
        d = g, e = h, f = i;
        ex >>= 1;
      }
      bool res1 = mint.same(a, one) || mint.same(a, mone);
      bool res2 = mint.same(b, one) || mint.same(b, mone);
      bool res3 = mint.same(c, one) || mint.same(c, mone);
      if(!(res1 && res2 && res3)) {
        for(u32 i = 0; i != S - 1; ++i) {
          a = mint.mul(a, a), b = mint.mul(b, b), c = mint.mul(c, c);
          res1 |= mint.same(a, mone), res2 |= mint.same(b, mone), res3 |= mint.same(c, mone);
        }
        if(!res1 || !res2 || !res3) return false;
      }
      return true;
    };
    auto test4 = [&](u64 base1, u64 base2, u64 base3, u64 base4) {
      auto a = one, b = one, c = one, d = one;
      auto e = mint.build(base1), f = mint.build(base2), g = mint.build(base3), h = mint.build(base4);
      u64 ex = D;
      while(ex) {
        auto i = mint.mul(e, e), j = mint.mul(f, f), k = mint.mul(g, g), l = mint.mul(h, h);
        if(ex & 1) a = mint.mul(a, e), b = mint.mul(b, f), c = mint.mul(c, g), d = mint.mul(d, h);
        e = i, f = j, g = k, h = l;
        ex >>= 1;
      }
      bool res1 = mint.same(a, one) || mint.same(a, mone);
      bool res2 = mint.same(b, one) || mint.same(b, mone);
      bool res3 = mint.same(c, one) || mint.same(c, mone);
      bool res4 = mint.same(d, one) || mint.same(d, mone);
      if(!(res1 && res2 && res3 && res4)) {
        for(u32 i = 0; i != S - 1; ++i) {
          a = mint.mul(a, a), b = mint.mul(b, b), c = mint.mul(c, c), d = mint.mul(d, d);
          res1 |= mint.same(a, mone), res2 |= mint.same(b, mone), res3 |= mint.same(c, mone), res4 |= mint.same(d, mone);
        }
        if(!res1 || !res2 || !res3 || !res4) return false;
      }
      return true;
    };
    if(x < 585226005592931977ull) {
      if(x < 7999252175582851ull) {
        if(x < 350269456337ull) return test3(4230279247111683200ull, 14694767155120705706ull, 16641139526367750375ull);
        else if(x < 55245642489451ull) return test4(2ull, 141889084524735ull, 1199124725622454117ull, 11096072698276303650ull);
        else return test2(2ull, 4130806001517ull) && test3(149795463772692060ull, 186635894390467037ull, 3967304179347715805ull);
      } else return test3(2ull, 123635709730000ull, 9233062284813009ull) && test3(43835965440333360ull, 761179012939631437ull, 1263739024124850375ull);
    } else return test3(2ull, 325ull, 9375ull) && test4(28178ull, 450775ull, 9780504ull, 1795265022ull);
  }
};
}
// @brief Prime number determination
constexpr bool IsPrime(const u64 x) noexcept {
  if(x < 256u) {
    return internal::IsPrime8::calc(x);
  } else {
    if(x % 2 == 0 || x % 3 == 0 || x % 5 == 0 || x % 7 == 0 || x % 11 == 0 || x % 13 == 0 || x % 17 == 0 || x % 19 == 0) return false;
    if(x <= 0xffffffff) return internal::IsPrime32::calc(x);
    else return internal::IsPrime64::calc(x);
  }
}
constexpr u32 CountPrimes(u64 N) {
  if(N <= 1) return 0;
  const u32 v = IntSqrt64(N);
  u32 s = (v + 1) / 2;
  u64* const invs = new u64[s];
  u32* const smalls = new u32[s];
  u32* const larges = new u32[s];
  u32* const roughs = new u32[s];
  bool* const smooth = new bool[v + 1];
  for(u32 i = 0; i != v; ++i) smooth[i] = false;
  for(u32 i = 0; i != s; ++i) smalls[i] = i;
  for(u32 i = 0; i != s; ++i) roughs[i] = 2 * i + 1;
  for(u32 i = 0; i != s; ++i) invs[i] = (f64)N / roughs[i];
  for(u32 i = 0; i != s; ++i) larges[i] = (invs[i] - 1) / 2;
  u32 pc = 0;
  for(u64 p = 3; p * p <= v; p += 2) {
    if(smooth[p]) continue;
    for(u64 i = p * p; i <= v; i += 2 * p) smooth[i] = true;
    smooth[p] = true;
    const auto divide_p = [invp = 0xffffffffffffffffu / p + 1](u64 inv_j) -> u64 { return (u128(inv_j) * invp) >> 64; };
    u32 ns = 0;
    u32 k = 0;
    GSH_INTERNAL_UNROLL(16)
    for(; true; ++k) {
      const u32 j = roughs[k];
      if(j * p > v) break;
      if(smooth[j]) continue;
      larges[ns] = larges[k] - larges[smalls[j * p / 2] - pc] + pc;
      invs[ns] = invs[k];
      roughs[ns] = roughs[k];
      ++ns;
    }
    GSH_INTERNAL_UNROLL(16)
    for(; k < s; ++k) {
      const u32 j = roughs[k];
      if(smooth[j]) continue;
      larges[ns] = larges[k] - smalls[(divide_p(invs[k]) - 1) / 2] + pc;
      invs[ns] = invs[k];
      roughs[ns] = roughs[k];
      ++ns;
    }
    s = ns;
    u64 i = (v - 1) / 2;
    for(u64 j = (divide_p(v) - 1) | 1; j >= p; j -= 2) {
      const u32 d = smalls[j / 2] - pc;
      for(; i >= j * p / 2; --i) smalls[i] -= d;
    }
    ++pc;
  }
  u32 ret = 1;
  ret += larges[0] + s * (s - 1) / 2 + (pc - 1) * (s - 1);
  for(u32 k = 1; k < s; ++k) ret -= larges[k];
  for(u32 k1 = 1; k1 < s; ++k1) {
    const u64 p = roughs[k1];
    const auto divide_p = [invp = 0xffffffffffffffffu / p + 1](u64 inv_j) -> u64 { return (u128(inv_j) * invp) >> 64; };
    const u32 k2_max = smalls[(divide_p(invs[k1]) - 1) / 2] - pc;
    if(k2_max <= k1) break;
    for(u32 k2 = k1 + 1; k2 <= k2_max; ++k2) ret += smalls[(divide_p(invs[k2]) - 1) / 2];
    ret -= (k2_max - k1) * (pc + k1 - 1);
  }
  delete[] invs;
  delete[] smalls;
  delete[] larges;
  delete[] roughs;
  delete[] smooth;
  return ret;
}
constexpr Vec<u32> EnumeratePrimes(u32 size) {
  if(size <= 1000) {
    // clang-format off
constexpr u32 primes[]={2,3,5,7,11,13,17,19,23,29,31,37,41,43,47,53,59,61,67,71,73,79,83,89,97,101,103,107,109,113,127,131,137,139,149,151,157,163,167,173,179,181,191,193,197,199,211,223,227,229,233,239,241,251,257,263,269,271,277,281,283,293,307,311,313,317,331,337,347,349,353,359,367,373,379,383,389,397,401,409,419,421,431,433,439,443,449,457,461,463,467,479,487,491,499,503,509,521,523,541,547,557,563,569,571,577,587,593,599,601,607,613,617,619,631,641,643,647,653,659,661,673,677,683,691,701,709,719,727,733,739,743,751,757,761,769,773,787,797,809,811,821,823,827,829,839,853,857,859,863,877,881,883,887,907,911,919,929,937,941,947,953,967,971,977,983,991,997};
    // clang-format on
    return Vec(primes, Subrange(primes, primes + 168).upper_bound(size));
  }
  const u32 flag_size = size / 30 + (size % 30 != 0);
  // clang-format off
constexpr u32 table1[]={0,1,7,1,11,1,7,1,13,1,7,1,11,1,7,1,17,1,7,1,11,1,7,1,13,1,7,1,11,1,7,1,19,1,7,1,11,1,7,1,13,1,7,1,11,1,7,1,17,1,7,1,11,1,7,1,13,1,7,1,11,1,7,1,23,1,7,1,11,1,7,1,13,1,7,1,11,1,7,1,17,1,7,1,11,1,7,1,13,1,7,1,11,1,7,1,19,1,7,1,11,1,7,1,13,1,7,1,11,1,7,1,17,1,7,1,11,1,7,1,13,1,7,1,11,1,7,1,29,1,7,1,11,1,7,1,13,1,7,1,11,1,7,1,17,1,7,1,11,1,7,1,13,1,7,1,11,1,7,1,19,1,7,1,11,1,7,1,13,1,7,1,11,1,7,1,17,1,7,1,11,1,7,1,13,1,7,1,11,1,7,1,23,1,7,1,11,1,7,1,13,1,7,1,11,1,7,1,17,1,7,1,11,1,7,1,13,1,7,1,11,1,7,1,19,1,7,1,11,1,7,1,13,1,7,1,11,1,7,1,17,1,7,1,11,1,7,1,13,1,7,1,11,1,7,1};
constexpr u8 table2[]={0,1,0,0,0,0,0,2,0,0,0,4,0,8,0,0,0,16,0,32,0,0,0,64,0,0,0,0,0,128};
  // clang-format on
  Vec<u8> flag(flag_size, 0xffu);
  flag[0] = 0b11111110u;
  Vec<u32> primes{2, 3, 5};
  f64 primes_size = std::is_constant_evaluated() ? static_cast<f64>(size) / 8 : size / std::log(size);
  primes.reserve(static_cast<u32>(1.1 * primes_size));
  Vec<u32> sieved(static_cast<u32>(primes_size));
  u32 *first = sieved.data(), *last;
  u32 k, l, x, y;
  u8 temp;
  for(k = 0; k * k < flag_size; ++k) {
    while(flag[k] != 0) {
      x = 30ull * k + table1[flag[k]];
      u32 limit = size / x;
      primes.push_back(x);
      last = first;
      bool smaller = true;
      for(l = k; smaller; ++l) {
        for(temp = flag[l]; temp != 0; temp &= (temp - 1)) {
          y = 30u * l + table1[temp];
          if(y > limit) {
            smaller = false;
            break;
          }
          *(last++) = x * y;
        }
      }
      flag[k] &= (flag[k] - 1);
      for(u32* i = first; i < last; ++i) flag[*i / 30] ^= table2[*i % 30];
    }
  }
  for(; k < flag_size; k++) {
    while(flag[k] != 0) {
      x = 30 * k + table1[flag[k]];
      if(x > size) { return primes; }
      primes.push_back(x);
      flag[k] &= (flag[k] - 1);
    }
  }
  return primes;
}
namespace internal {
inline u16 TinyPrimes[6542] = {};
inline u64 InvPrimes[6542] = {};
inline u64 FindFactor(u64 x) {
  MontgomeryModint64Impl mint;
  mint.set(x);
  static Rand64 engine;
  constexpr u32 repeat = 8192;
retry:
  u64 r = mint.raw(engine() % (x - 1) + 1), a, b = mint.raw(engine() % (x - 1) + 1);
  u32 k = repeat;
  while(true) {
    for(u32 i = k + 1; --i;) b = mint.fma(b, b, r);
    a = b;
    for(u32 i = 0; i < k; i += repeat) {
      u64 mul = mint.one(), prev = b;
      for(u32 j = repeat + 1; --j;) {
        b = mint.fma(b, b, r);
        mul = mint.mul(mul, mint.sub(a, b));
      }
      u64 g = GCD(mint.val(mul), x);
      if(g == x) {
        mul = mint.one();
        do {
          prev = mint.fma(prev, prev, r);
          mul = mint.mul(mul, mint.sub(a, prev));
          g = GCD(mint.val(mul), x);
        } while(g == 1);
        if(g == x) goto retry;
      }
      if(g != 1) return g;
    }
    k *= 2;
  }
}
inline u64* FactorizeSub64(u64 n, u64* res) noexcept {
  Assume(n % 2 != 0 && n % 3 != 0 && n % 5 != 0 && n % 7 != 0 && n % 11 != 0 && n % 13 != 0 && n % 17 != 0 && n % 19 != 0);
  if(IsPrime(n)) {
    *(res++) = n;
    return res;
  }
  if(n <= 0xffffffff) {
    if(TinyPrimes[0] == 0) {
      u32 cnt = 0;
      for(u32 i = 0; i != (1 << 16); ++i) {
        if(IsPrime(i)) TinyPrimes[cnt++] = i;
      }
      for(u32 i = 0; i != 6542; ++i) { InvPrimes[i] = 0xffffffffffffffff / TinyPrimes[i] + 1; }
    }
    auto check = [&](u64 idx, u64 m) {
      if(m * n < m) {
        u64 p = TinyPrimes[idx];
        do {
          *(res++) = p;
          n = (static_cast<u128>(m) * n) >> 64;
        } while(m * n < m);
      }
    };
    for(u32 i = 8; i != 14; ++i) {
      check(i, InvPrimes[i]);
      u64 p = TinyPrimes[i + 1];
      if(p * p > n) {
        if(n != 1) *(res++) = n;
        return res;
      }
    }
    for(u32 i = 14; i != 6542; i += 8) {
      u64 m1 = InvPrimes[i];
      u64 m2 = InvPrimes[i + 1];
      u64 m3 = InvPrimes[i + 2];
      u64 m4 = InvPrimes[i + 3];
      u64 m5 = InvPrimes[i + 4];
      u64 m6 = InvPrimes[i + 5];
      u64 m7 = InvPrimes[i + 6];
      u64 m8 = InvPrimes[i + 7];
      if(m1 * n < m1 || m2 * n < m2 || m3 * n < m3 || m4 * n < m4 || m5 * n < m5 || m6 * n < m6 || m7 * n < m7 || m8 * n < m8) {
        check(i, m1);
        check(i + 1, m2);
        check(i + 2, m3);
        check(i + 3, m4);
        check(i + 4, m5);
        check(i + 5, m6);
        check(i + 6, m7);
        check(i + 7, m8);
      }
      u64 p = TinyPrimes[i + 7];
      if(p * p >= n) break;
    }
    if(n != 1) *(res++) = n;
    return res;
  }
  u64 m = FindFactor(n);
  if(n / m < 529) *(res++) = n / m;
  else res = FactorizeSub64(n / m, res);
  if(m < 529) {
    *(res++) = m;
    return res;
  } else return FactorizeSub64(m, res);
}
}
inline auto Factorize(u64 n) {
  thread_local u64 res[64];
  if(n <= 1) [[unlikely]]
    return Subrange(res, res);
  u64* p = res;
  {
    Assume(n != 0);
    u32 rz = std::countr_zero(n);
    n >>= rz;
    for(u32 i = 0; i != rz; ++i) *(p++) = 2;
  }
  {
    const bool a = n % 3 == 0, b = n % 5 == 0, c = n % 7 == 0, d = n % 11 == 0, e = n % 13 == 0, f = n % 17 == 0, g = n % 19 == 0;
    if(a) [[unlikely]] {
      do {
        n /= 3;
        *(p++) = 3;
      } while(n % 3 == 0);
    }
    if(b) [[unlikely]] {
      do {
        n /= 5;
        *(p++) = 5;
      } while(n % 5 == 0);
    }
    if(c) [[unlikely]] {
      do {
        n /= 7;
        *(p++) = 7;
      } while(n % 7 == 0);
    }
    if(d) [[unlikely]] {
      do {
        n /= 11;
        *(p++) = 11;
      } while(n % 11 == 0);
    }
    if(e) [[unlikely]] {
      do {
        n /= 13;
        *(p++) = 13;
      } while(n % 13 == 0);
    }
    if(f) [[unlikely]] {
      do {
        n /= 17;
        *(p++) = 17;
      } while(n % 17 == 0);
    }
    if(g) [[unlikely]] {
      do {
        n /= 19;
        *(p++) = 19;
      } while(n % 19 == 0);
    }
  }
  if(n >= 529) [[likely]] {
    p = internal::FactorizeSub64(n, p);
  } else {
    *p = n;
    p += n != 1;
  }
  return Subrange(res, p);
}
}
