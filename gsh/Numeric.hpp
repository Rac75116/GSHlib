#pragma once
#include "Memory.hpp"
#include "Modint.hpp"
#include "TypeDef.hpp"
#include <cmath>
#include <new>
#include <ranges>
#include <type_traits>
namespace gsh {
constexpr u32 CharLength32(u32 x) {
  // clang-format off
    constexpr static u32 lower_bound_table[] = { 9,9,9,8,8,8,7,7,7,6,6,6,6,5,5,5,4,4,4,3,3,3,3,2,2,2,1,1,1,0,0,0 };
    constexpr static u32 pow10[] = { 1000000000,1000000000,1000000000,100000000,100000000,100000000,10000000,10000000,10000000,1000000,1000000,1000000,1000000,100000,100000,100000,10000,10000,10000,1000,1000,1000,1000,100,100,100,10,10,10,1,1,1 };
  // clang-format on
  x |= 1;
  i32 clz = std::countl_zero(x);
  return lower_bound_table[clz] + static_cast<u32>(pow10[clz] <= x);
}
constexpr u32 CharLength64(u64 x) {
  // clang-format off
    constexpr static u32 lower_bound_table[] = { 19,18,18,18,18,17,17,17,16,16,16,15,15,15,15,14,14,14,13,13,13,12,12,12,12,11,11,11,10,10,10,9,9,9,9,8,8,8,7,7,7,6,6,6,6,5,5,5,4,4,4,3,3,3,3,2,2,2,1,1,1,0,0,0 };
    constexpr static u64 pow10[] = { 10000000000000000000ull,1000000000000000000ull,1000000000000000000ull,1000000000000000000ull,1000000000000000000ull,100000000000000000ull,100000000000000000ull,100000000000000000ull,10000000000000000ull,10000000000000000ull,10000000000000000ull,1000000000000000ull,1000000000000000ull,1000000000000000ull,1000000000000000ull,100000000000000ull,100000000000000ull,100000000000000ull,10000000000000ull,10000000000000ull,10000000000000ull,1000000000000ull,1000000000000ull,1000000000000ull,1000000000000ull,100000000000ull,100000000000ull,100000000000ull,10000000000ull,10000000000ull,10000000000ull,1000000000ull,1000000000ull,1000000000ull,1000000000ull,100000000ull,100000000ull,100000000ull,10000000ull,10000000ull,10000000ull,1000000ull,1000000ull,1000000ull,1000000ull,100000ull,100000ull,100000ull,10000ull,10000ull,10000ull,1000ull,1000ull,1000ull,1000ull,100ull,100ull,100ull,10ull,10ull,10ull,1ull,1ull,1ull };
  // clang-format on
  x |= 1ULL;
  i32 clz = std::countl_zero(x);
  return lower_bound_table[clz] + static_cast<u32>(pow10[clz] <= x);
}
template<class... Bounds> requires (sizeof...(Bounds) != 0 && sizeof...(Bounds) % 2 == 0) class InRange {
  std::tuple<Bounds...> bounds;
public:
  constexpr InRange(const Bounds&... bounds) : bounds(bounds...) {}
  template<class... Args> requires (sizeof...(Args) == sizeof...(Bounds) / 2) constexpr bool operator()(Args&&... args) {
    return [&]<u32... I>(std::integer_sequence<u32, I...>) {
      auto det = [&]<u32 Idx>(std::integral_constant<u32, Idx>, auto&& val) { return std::get<2 * Idx>(bounds) <= val && val < std::get<2 * Idx + 1>(bounds); };
      return (det(std::integral_constant<u32, I>(), std::forward<Args>(args)) && ...);
    }(std::make_integer_sequence<u32, sizeof...(Args)>());
  }
};
//@brief Find the largest x for which x * x <= n (https://rsk0315.hatenablog.com/entry/2023/11/07/221428)
GSH_INTERNAL_INLINE constexpr u32 IntSqrt32(const u32 x) {
  if(x == 0) return 0;
  if(std::is_constant_evaluated()) {
    u32 low = 0, high = 0xffff;
    while(low != high) {
      u32 mid = low + (high - low + 1) / 2;
      if(mid * mid > x) high = mid - 1;
      else low = mid;
    }
    return low;
  } else {
    u32 tmp = static_cast<u32>(std::sqrt(static_cast<f32>(x))) - 1;
    return tmp + (tmp * (tmp + 2) < x);
  }
}
GSH_INTERNAL_INLINE constexpr u64 IntSqrt64(const u64 x) {
  if(x == 0) return 0;
  if(std::is_constant_evaluated()) {
    u64 low = 0, high = 0xffffffff;
    while(low != high) {
      u64 mid = low + (high - low + 1) / 2;
      if(mid * mid > x) high = mid - 1;
      else low = mid;
    }
    return low;
  } else {
    u64 tmp = static_cast<u64>(std::sqrt(static_cast<f64>(x))) - 1;
    return tmp + (tmp * (tmp + 2) < x);
  }
}
constexpr bool isSquare32(const u32 x) {
  const u32 tmp = IntSqrt32(x);
  return tmp * tmp == x;
}
constexpr bool isSquare64(const u64 x) {
  if(!(144680414395695635u >> (x % 64) & 1)) return false;
  const u64 tmp = IntSqrt64(x);
  return tmp * tmp == x;
}
template<class T, class U> constexpr auto Umod(T x, U m) {
  if((m >= 0) ^ (x >= 0)) {
    m = m >= 0 ? m : -m;
    auto res = x % m + m;
    res = res >= m ? res - m : res;
    return static_cast<std::make_unsigned_t<decltype(res)>>(res);
  } else {
    auto res = x % m;
    return static_cast<std::make_unsigned_t<decltype(res)>>(res);
  }
}
template<class T> constexpr T IntPow(const T& x, u64 e) {
  T res = 1, pow = x;
  while(e != 0) {
    const T tmp = pow * pow;
    if(e & 1) res *= pow;
    pow = tmp;
    e >>= 1;
  }
  return res;
}
template<class T, class U> constexpr T ModPow(const T& x, u64 e, const U& mod) {
  T res = 1, pow = x % mod;
  while(e != 0) {
    const T tmp = (pow * pow) % mod;
    if(e & 1) res = (res * pow) % mod;
    pow = tmp;
    e >>= 1;
  }
  return res;
}
template<class T, class U> constexpr auto DivCeil(const T& a, const U& b) { return (a + b - 1) / b; }
template<class T> constexpr T Factorial(const T& n) {
  T res = n;
  T cnt = {};
  if(!(cnt != n)) return res;
  for(++cnt; cnt != n; ++cnt) res *= cnt;
  return res;
}
constexpr struct GCDFunc {
  template<class T, class U> constexpr std::common_type_t<T, U> operator()(T x, U y) const {
    static_assert(!std::is_same_v<T, bool> && !std::is_same_v<U, bool> && std::is_integral_v<T> && std::is_integral_v<U>, "gsh::GCD / The input must be an integral type.");
    if constexpr(std::is_same_v<T, U>) {
      if constexpr(std::is_unsigned_v<T>) {
        if(x == 0 || y == 0) return x | y;
        const i32 n = std::countr_zero(x);
        const i32 m = std::countr_zero(y);
        const i32 l = n < m ? n : m;
        x >>= n;
        y >>= m;
        while(x != y) {
          const T a = y - x, b = x - y;
          const i32 m = std::countr_zero(a), n = std::countr_zero(b);
          Assume(m == n);
          const T s = y < x ? b : a;
          const T t = x < y ? x : y;
          x = s >> m;
          y = t;
        }
        return x << l;
      } else {
        return static_cast<T>(operator()<std::make_unsigned_t<T>, std::make_unsigned_t<T>>((x < 0 ? -x : x), (y < 0 ? -y : y)));
      }
    } else {
      return operator()<std::common_type_t<T, U>, std::common_type_t<T, U>>(x, y);
    }
  }
  template<class T, class... Args> constexpr auto operator()(T x, Args... y) const { return (*this)(x, (*this)(y...)); }
  using is_transparent = void;
} GCD;
constexpr struct LCMFunc {
  template<class T, class U> constexpr auto operator()(T x, U y) const { return static_cast<std::common_type_t<T, U>>(x < 0 ? -x : x) / GCD(x, y) * static_cast<std::common_type_t<T, U>>(y < 0 ? -y : y); }
  template<class T, class... Args> constexpr auto operator()(T x, Args... y) const { return (*this)(x, (*this)(y...)); }
  using is_transparent = void;
} LCM;
namespace internal {
template<class R, class Proj> constexpr auto GCDImpl(R&& r, Proj&& proj) {
  auto itr = std::ranges::begin(r);
  auto sent = std::ranges::end(r);
  if(!(itr != sent)) throw Exception("The container is empty.");
  auto res = Invoke(proj, *itr);
  for(++itr; itr != sent; ++itr) res = GCD(res, Invoke(proj, *itr));
  return res;
}
template<class R, class Proj> constexpr auto LCMImpl(R&& r, Proj&& proj) {
  auto itr = std::ranges::begin(r);
  auto sent = std::ranges::end(r);
  if(!(itr != sent)) throw Exception("The container is empty.");
  auto res = Invoke(proj, *itr);
  for(++itr; itr != sent; ++itr) res = LCM(res, Invoke(proj, *itr));
  return res;
}
} // namespace internal
namespace internal {
template<u32> struct KthRootImpl {
  // clang-format off
constexpr static u64 pw3[] = {
1,3,9,27,81,243,729,2187,6561,19683,59049,177147,531441,1594323,4782969,14348907,43046721,129140163,387420489,1162261467,3486784401,10460353203,31381059609,94143178827,282429536481,847288609443,2541865828329,7625597484987,
22876792454961,68630377364883,205891132094649,617673396283947,1853020188851841,5559060566555523,16677181699666569,50031545098999707,150094635296999121,450283905890997363,1350851717672992089,4052555153018976267,12157665459056928801u
};
constexpr static u64 pw5[] = {
1,5,25,125,625,3125,15625,78125,390625,1953125,9765625,48828125,244140625,1220703125,6103515625,30517578125,152587890625,762939453125,3814697265625,
19073486328125,95367431640625,476837158203125,2384185791015625,11920928955078125,59604644775390625,298023223876953125,1490116119384765625,7450580596923828125
};
constexpr static u64 pw7[] = {
1,7,49,343,2401,16807,117649,823543,5764801,40353607,282475249,1977326743,13841287201,96889010407,678223072849,4747561509943,33232930569601,
232630513987207,1628413597910449,11398895185373143,79792266297612001,558545864083284007,3909821048582988049,8922003266371364727,7113790643470898241
};
constexpr static u64 pw11[] = {
1,11,121,1331,14641,161051,1771561,19487171,214358881,2357947691,25937424601,285311670611,3138428376721,34522712143931,379749833583241,4177248169415651,45949729863572161,505447028499293771,5559917313492231481
};
constexpr static u64 pw13[] = {
1,13,169,2197,28561,371293,4826809,62748517,815730721,10604499373,137858491849,1792160394037,23298085122481,302875106592253,3937376385699289,51185893014090757,665416609183179841,8650415919381337933
};
constexpr static u64 pw17[] = {
1,17,289,4913,83521,1419857,24137569,410338673,6975757441,118587876497,2015993900449,34271896307633,582622237229761,9904578032905937,168377826559400929,2862423051509815793
};
constexpr static u64 pw19[] = {
1,19,361,6859,130321,2476099,47045881,893871739,16983563041,322687697779,6131066257801,116490258898219,2213314919066161,42052983462257059,799006685782884121,15181127029874798299u
};
constexpr static u64 pw23[] = {
1,23,529,12167,279841,6436343,148035889,3404825447,78310985281,1801152661463,41426511213649,952809757913927,21914624432020321,504036361936467383,11592836324538749809u
};
constexpr static u64 pw29[] = {
1,29,841,24389,707281,20511149,594823321,17249876309,500246412961,14507145975869,420707233300201,12200509765705829,353814783205469041,10260628712958602189u
};
constexpr static u64 pw31[] = {
1,31,961,29791,923521,28629151,887503681,27512614111,852891037441,26439622160671,819628286980801,25408476896404831,787662783788549761
};
constexpr static u64 pw37[] = {
1,37,1369,50653,1874161,69343957,2565726409,94931877133,3512479453921,129961739795077,4808584372417849,177917621779460413,6582952005840035281
};
constexpr static f64 iv[] = {
0.0,0x1.fffffffffffffp-1,0x1.fffffffffffffp-2,0x1.5555555555554p-2,0x1.fffffffffffffp-3,0x1.9999999999999p-3,0x1.5555555555554p-3,0x1.2492492492491p-3,0x1.fffffffffffffp-4,0x1.c71c71c71c71bp-4,0x1.9999999999999p-4,0x1.745d1745d1745p-4
};
constexpr static u64 lim[] = {
0,18446744073709551615u,4294967295,2642245,65535,7131,1625,565,255,138,84,56,40,30,23,19,15,13,11,10,9,8,7,6,6,5,5,5,4,4,4,4
};
constexpr static f64 rt3[] = {
0x1p+0,0x1.965fea53d6e3cp-1,0x1.428a2f98d728bp-1,0x1p-1,0x1.965fea53d6e3cp-2,0x1.428a2f98d728bp-2,0x1p-2,0x1.965fea53d6e3cp-3,0x1.428a2f98d728bp-3,0x1p-3,0x1.965fea53d6e3cp-4,0x1.428a2f98d728bp-4,0x1p-4,0x1.965fea53d6e3cp-5,0x1.428a2f98d728bp-5,0x1p-5,0x1.965fea53d6e3cp-6,
0x1.428a2f98d728bp-6,0x1p-6,0x1.965fea53d6e3cp-7,0x1.428a2f98d728bp-7,0x1p-7,0x1.965fea53d6e3cp-8,0x1.428a2f98d728bp-8,0x1p-8,0x1.965fea53d6e3cp-9,0x1.428a2f98d728bp-9,0x1p-9,0x1.965fea53d6e3cp-10,0x1.428a2f98d728bp-10,0x1p-10,0x1.965fea53d6e3cp-11,0x1.428a2f98d728bp-11,
0x1p-11,0x1.965fea53d6e3cp-12,0x1.428a2f98d728bp-12,0x1p-12,0x1.965fea53d6e3cp-13,0x1.428a2f98d728bp-13,0x1p-13,0x1.965fea53d6e3cp-14,0x1.428a2f98d728bp-14,0x1p-14,0x1.965fea53d6e3cp-15,0x1.428a2f98d728bp-15,0x1p-15,0x1.965fea53d6e3cp-16,0x1.428a2f98d728bp-16,0x1p-16,
0x1.965fea53d6e3cp-17,0x1.428a2f98d728bp-17,0x1p-17,0x1.965fea53d6e3cp-18,0x1.428a2f98d728bp-18,0x1p-18,0x1.965fea53d6e3cp-19,0x1.428a2f98d728bp-19,0x1p-19,0x1.965fea53d6e3cp-20,0x1.428a2f98d728bp-20,0x1p-20,0x1.965fea53d6e3cp-21,0x1.428a2f98d728bp-21,0x1p-21
};
  // clang-format on
  template<u64 K> constexpr static u64 calc(u64 n) {
    if constexpr(K == 3) {
      const u32 t = std::bit_width(n) - 1;
      f64 x = rt3[t];
      GSH_INTERNAL_UNROLL(3)
      for(u32 i = 0; i != 3; ++i) {
        f64 h = 1.0 - n * x * x * x;
        x += x * h * ((1.0 / 3) + h * ((2.0 / 9) + h * ((14.0 / 81) + (h * ((35.0 / 243) + h * (91.0 / 729))))));
      }
      const u64 r = 1.0 / x;
      return r + (r < lim[K] && (r + 1) * (r + 1) * (r + 1) <= n) - (r > lim[K] || r * r * r > n);
    } else if constexpr(K == 2) return IntSqrt64(n);
    else if constexpr(K == 1) return n;
    else if constexpr(K == 0) return 0xffffffffffffffff;
    else if constexpr(K >= 12) {
      u64 res = 1 + (n >= (1ull << K));
      if constexpr(K < 41) res += (n >= pw3[K]);
      if constexpr(K < 32) res += (n >= (1ull << (2 * K)));
      if constexpr(K < 28) res += (n >= pw5[K]);
      if constexpr(K < 25) res += (n >= (pw3[K] << K));
      if constexpr(K < 23) res += (n >= pw7[K]);
      if constexpr(K < 22) res += (n >= (1ull << (3 * K)));
      if constexpr(K < 21) res += (n >= pw3[K] * pw3[K]);
      if constexpr(K < 20) res += (n >= (pw5[K] << K));
      if constexpr(K < 19) res += (n >= pw11[K]);
      if constexpr(K < 18) res += (n >= (pw3[K] << (2 * K))) + (n >= pw13[K]);
      if constexpr(K < 17) res += (n >= (pw7[K] << K)) + (n >= (pw3[K] * pw5[K]));
      if constexpr(K < 16) res += (n >= (1ull << (4 * K))) + (n >= pw17[K]) + (n >= ((pw3[K] * pw3[K]) << K)) + (n >= pw19[K]);
      if constexpr(K < 15) res += (n >= (pw5[K] << (2 * K))) + (n >= (pw3[K] * pw7[K])) + (n >= (pw11[K] << K)) + (n >= pw23[K]);
      if constexpr(K < 14) res += (n >= (pw3[K] << (3 * K))) + (n >= (pw5[K] * pw5[K])) + (n >= (pw13[K] << K)) + (n >= (pw3[K] * pw3[K] * pw3[K])) + (n >= (pw7[K] << (2 * K))) + (n >= pw29[K]) + (n >= ((pw3[K] * pw5[K]) << K));
      if constexpr(K < 13) res += (n >= pw31[K]) + (n >= (1ull << (5 * K))) + (n >= (pw3[K] * pw11[K])) + (n >= (pw17[K] << K)) + (n >= (pw5[K] * pw7[K])) + (n >= ((pw3[K] * pw3[K]) << (2 * K))) + (n >= pw37[K]) + (n >= (pw19[K] << K)) + (n >= (pw3[K] * pw13[K])) + (n >= (pw5[K] << (3 * K)));
      return res;
    } else {
      const u64 r = static_cast<u64>(std::pow(n, iv[K]));
      u64 a = 1, p = r + 1;
      GSH_INTERNAL_UNROLL(8)
      for(u64 e = K; e != 0; e >>= 1) {
        if(e & 1) a *= p;
        p *= p;
      }
      return r + (r < lim[K] && a <= n);
    }
  }
  constexpr static u64 calc2(u64 n, u64 k) {
    if(n == 0) return 0;
    // clang-format off
#ifdef F
#define GSH_INTERNAL_DEFINED_F
#pragma push_macro("F")
#undef F
#endif
#define F(x) case x : return calc<x>(n)
            switch (k) {
F(0);F(1);F(2);F(3);F(4);F(5);F(6);F(7);F(8);F(9);F(10);F(11);F(12);F(13);F(14);F(15);F(16);F(17);F(18);F(19);F(20);F(21);F(22);F(23);F(24);F(25);F(26);F(27);F(28);F(29);F(30);F(31);F(32);
F(33);F(34);F(35);F(36);F(37);F(38);F(39);F(40);F(41);F(42);F(43);F(44);F(45);F(46);F(47);F(48);F(49);F(50);F(51);F(52);F(53);F(54);F(55);F(56);F(57);F(58);F(59);F(60);F(61);F(62);F(63);
default : return 1;
            }
#undef F
#ifdef GSH_INTERNAL_DEFINED_F
#pragma pop_macro("F")
#endif
    // clang-format on
  }
};
} // namespace internal
constexpr u64 KthRoot(u64 n, u64 k) { return internal::KthRootImpl<0>::calc2(n, k); }
// calc ∑ floor((a × i + b) / m) (0 <= i < n)
constexpr u64 LinearFloorSum(u32 n, u32 m, u32 a, u32 b) {
  u64 res = 0;
  while(true) {
    const u32 p = a / m, q = b / m;
    a %= m;
    b %= m;
    res += static_cast<u64>(n) * (n - 1) / 2 * p + static_cast<u64>(n) * q;
    const u64 last = a * static_cast<u64>(n) + b;
    if(last < m) return res;
    n = last / m;
    b = last % m;
    u32 tmp = a;
    a = m, m = tmp;
  }
}
// calc min{ (a × i + b) mod m | 0 <= i < n }
constexpr u32 LinearModMin(u32 n, u32 m, u32 a, u32 b) {
  u32 res = 0;
  bool z = true;
  u32 p = 1, q = 1;
  while(a != 0) {
    const u32 e = (z ? a : m) - 1;
    const u32 d = m / a, r = m % a;
    const u32 g = d * p + q;
    if((z ? b + 1 : m - b) > a) {
      const u32 t = (m - b + (z ? a : 0) - 1) / a;
      const u32 c = (t - z) * p + (z ? q : 0);
      if(n <= c) {
        const u32 h = z ? 0 : a * ((n - 1) / p);
        res += (z ? h : -h);
        break;
      }
      n -= c, b += a * t - (z ? m : 0);
    }
    q = g, p = g - p;
    res += z ? e : -e;
    m = a, a = r, b = e - b, z = !z;
  }
  res += (z ? b : -b);
  return res;
}
template<class F> i64 FibonacciSearch(i64 left, i64 right, F&& func) {
  using Ret = std::invoke_result_t<std::remove_reference_t<F>, i64>;
  i64 length = right - left;
  if(length <= 1) return left;
  i64 fib[93];
  fib[0] = 1;
  fib[1] = 1;
  i32 k = 1;
  while(fib[k] < length && k + 1 < 93) {
    ++k;
    fib[k] = fib[k - 1] + fib[k - 2];
  }
  i64 L = 0;
  i64 R = fib[k];
  i64 m1 = fib[k - 2];
  i64 m2 = fib[k - 1];
  auto eval = [&](i64 offset) GSH_INTERNAL_INLINE_LAMBDA -> Ret {
    i64 x = left + offset;
    if(x >= right) return std::numeric_limits<Ret>::max();
    return std::invoke(func, x);
  };
  Ret v1 = eval(m1);
  Ret v2 = eval(m2);
  while(k > 2) {
    if(v1 < v2) {
      R = m2;
      m2 = m1;
      v2 = v1;
      --k;
      m1 = L + fib[k - 2];
      v1 = eval(m1);
    } else {
      L = m1;
      m1 = m2;
      v1 = v2;
      --k;
      m2 = L + fib[k - 1];
      v2 = eval(m2);
    }
  }
  i64 ans = L;
  Ret best = eval(L);
  if(m1 < R) {
    if(v1 < best) {
      best = v1;
      ans = m1;
    }
  }
  if(m2 < R && m2 != m1) {
    if(v2 < best) {
      best = v2;
      ans = m2;
    }
  }
  i64 xR = (R < length) ? R : (length - 1);
  if(xR > L) {
    Ret vR = eval(xR);
    if(vR < best) {
      best = vR;
      ans = xR;
    }
  }
  return left + ans;
}
template<class F> f64 GoldenSectionSearch(f64 left, f64 right, u32 precision, F&& func) {
  constexpr f64 resphi = 2.0 - 1.61803398874989484820;
  f64 x1 = left + (right - left) * resphi;
  f64 x2 = right - (right - left) * resphi;
  auto f1 = std::invoke(func, x1);
  auto f2 = std::invoke(func, x2);
  for(u32 i = 0; i < precision; ++i) {
    if(f1 < f2) {
      right = x2;
      x2 = x1;
      f2 = f1;
      x1 = left + (right - left) * resphi;
      f1 = std::invoke(func, x1);
    } else {
      left = x1;
      x1 = x2;
      f1 = f2;
      x2 = right - (right - left) * resphi;
      f2 = std::invoke(func, x2);
    }
  }
  return (left + right) * 0.5;
}
class QuotientsList {
  const u64 x;
  const u32 sq;
  u32 m;
public:
  using value_type = u32;
  constexpr QuotientsList(u64 n) : x(n), sq(IntSqrt64(n)) { m = (u64(sq) * sq + sq <= n ? sq : sq - 1); }
  constexpr u32 size() const noexcept { return sq + m; }
  constexpr u32 iota_limit() const noexcept { return sq; }
  constexpr u32 div_limit() const noexcept { return m; }
  constexpr u64 val() const noexcept { return x; }
  constexpr u64 operator[](u32 n) { return n < m ? n + 1 : x / (sq - (n - m)); }
  // n <= 1e18
  constexpr u64 sum() const noexcept {
    u64 res = 0;
    for(u32 i = 1; i <= sq; ++i) res += x / i;
    res <<= 1;
    res -= u64(sq) * sq;
    return res;
  }
};
namespace internal {
template<class T> class BinCoeffTable {
  T mint;
  Mem<typename T::value_type> fac, finv;
public:
  using value_type = typename T::value_type;
  constexpr BinCoeffTable(u32 mx, value_type mod) : fac(mx, value_type{}), finv(mx, value_type{}) {
    if(mx > mod) throw Exception("gsh::internal::BinCoeffTable:::BinCoeffTable / The table size cannot be larger than mod.");
    mint.set(mod);
    fac[0] = mint.raw(1), finv[0] = mint.raw(1);
    if(mx > 1) {
      fac[1] = mint.raw(1), finv[1] = mint.raw(1);
      if(mx > 2) {
        auto cnt = mint.raw(1);
        for(u32 i = 2; i != mx; ++i) {
          cnt = mint.inc(cnt);
          fac[i] = mint.mul(fac[i - 1], cnt);
        }
        finv.back() = mint.inv(fac.back());
        for(u32 i = mx - 1; i != 2; --i) {
          finv[i - 1] = mint.mul(finv[i], cnt);
          cnt = mint.dec(cnt);
        }
      }
    }
  }
  constexpr value_type operator()(u32 n, u32 k) const {
    if(n < k) return 0;
    else return mint.val(mint.mul(mint.mul(fac[n], finv[k]), finv[n - k]));
  }
};
template<IsStaticModint T> class BinCoeffTable<T> {
  [[no_unique_address]] T mint;
  Mem<typename T::value_type> fac, finv;
public:
  using value_type = typename T::value_type;
  constexpr BinCoeffTable(u32 mx) : fac(mx, value_type{}), finv(mx, value_type{}) {
    if(mx > mint.mod()) throw Exception("gsh::internal::BinCoeffTable:::BinCoeffTable / The table size cannot be larger than mod.");
    fac[0] = mint.raw(1), finv[0] = mint.raw(1);
    if(mx > 1) {
      fac[1] = mint.raw(1), finv[1] = mint.raw(1);
      if(mx > 2) {
        auto cnt = mint.raw(1);
        for(u32 i = 2; i != mx; ++i) {
          cnt = mint.inc(cnt);
          fac[i] = mint.mul(fac[i - 1], cnt);
        }
        finv.back() = mint.inv(fac.back());
        for(u32 i = mx - 1; i != 2; --i) {
          finv[i - 1] = mint.mul(finv[i], cnt);
          cnt = mint.dec(cnt);
        }
      }
    }
  }
  constexpr value_type operator()(u32 n, u32 k) const {
    if(n < k) return 0;
    else return mint.val(mint.mul(mint.mul(fac[n], finv[k]), finv[n - k]));
  }
};
} // namespace internal
using BinCoeffTable32 = internal::BinCoeffTable<internal::DynamicModint32Impl>;
using BinCoeffTable64 = internal::BinCoeffTable<internal::DynamicModint64Impl>;
template<u64 mod = 998244353> using BinCoeffTableStaticMod = internal::BinCoeffTable<internal::StaticModintImpl<mod>>;
template<class T> constexpr T ToRad(const T& deg) { return deg * static_cast<T>(3.14159265358979323846) / static_cast<T>(180); }
template<class T> constexpr T ToDeg(const T& rad) { return rad * static_cast<T>(180) / static_cast<T>(3.14159265358979323846); }
} // namespace gsh
