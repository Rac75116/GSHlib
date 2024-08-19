#pragma once
#include <type_traits>
#include <cmath>
#include <new>
#include <gsh/Modint.hpp>
#include <gsh/TypeDef.hpp>
#include <gsh/Arr.hpp>
#include <gsh/Option.hpp>

namespace gsh {

//@brief Find the largest x for which x * x <= n (https://rsk0315.hatenablog.com/entry/2023/11/07/221428)
GSH_INTERNAL_PUSH_ATTRIBUTE(function, optimize("no-fast-math"))
constexpr itype::u32 IntSqrt32(const itype::u32 x) {
    if (x == 0) return 0;
    if (std::is_constant_evaluated()) {
        itype::u32 low = 0, high = 0xffff;
        while (low != high) {
            itype::u32 mid = low + (high - low + 1) / 2;
            if (mid * mid > x) high = mid - 1;
            else low = mid;
        }
        return low;
    } else {
        itype::u32 tmp = static_cast<itype::u32>(std::sqrt(static_cast<ftype::f32>(x))) - 1;
        return tmp + (tmp * (tmp + 2) < x);
    }
}
GSH_INTERNAL_POP_ATTRIBUTE
GSH_INTERNAL_PUSH_ATTRIBUTE(function, optimize("no-fast-math"))
constexpr itype::u64 IntSqrt64(const itype::u64 x) {
    if (x == 0) return 0;
    if (std::is_constant_evaluated()) {
        itype::u64 low = 0, high = 0xffffffff;
        while (low != high) {
            itype::u64 mid = low + (high - low + 1) / 2;
            if (mid * mid > x) high = mid - 1;
            else low = mid;
        }
        return low;
    } else {
        itype::u64 tmp = static_cast<itype::u64>(std::sqrt(static_cast<ftype::f64>(x))) - 1;
        return tmp + (tmp * (tmp + 2) < x);
    }
}
GSH_INTERNAL_POP_ATTRIBUTE
namespace internal {
    template<itype::u32> struct isSquareMod9360 {
        // clang-format off
        constexpr static itype::u64 table[147] = {0x2001002010213u,0x200001000020001u,0x20100010000u,0x10000200000010u,0x200000001u,0x20000000010u,0x200000000010000u,0x1200000000u,0x20000u,0x2000002000201u,0x1000000201u,0x20002100000u,0x10000000010000u,0x1000000000200u,0x2000000000010u,0x2010002u,0x100001u,0x20002u,0x210u,0x1000200000200u,0x110000u,0x2000000u,0x201001100000000u,0x2000100000000u,0x2000002000000u,0x201u,
        0x20002u,0x10001000000002u,0x200000000000000u,0x2100000u,0x10012u,0x200020100000000u,0x20100000000u,0x2000000000010u,0x1000200100200u,0u,0x10001000000003u,0x1200000000u,0x10000000000000u,0x2000002000010u,0x21000000001u,0x20100000000u,0x10000000010000u,0x200000200000000u,0u,0x2001000010200u,0x1000020000u,0x20000u,0x12000000000000u,0x1000200000201u,0x2020000100000u,0x10000002010000u,0x1001000000000u,0x20000u,
        0x2000000u,0x1u,0x10000000130000u,0x2u,0x201000300000200u,0x2000000100010u,0x2000010u,0x200001000000001u,0x100000002u,0x2000000000000u,0x1000000000201u,0x2010000u,0x10000000000002u,0x200020100000000u,0x100020010u,0x10u,0x200u,0x20100100000u,0x1000010000u,0x201000200020200u,0x2000000u,0x2000000000002u,0x21000000000u,0x20000000000u,0x13000000000010u,0x1u,0x20000000002u,0x10000002010001u,0x200000200020000u,
        0x100020000u,0x2000200000000u,0x1000000000u,0x120000u,0x211000000000000u,0x1000200000200u,0x100000u,0x2010201u,0x1000020001u,0x10020000020000u,0u,0x200000001u,0x100010u,0x200000000000002u,0x201001200000000u,0x100020000u,0x2000210u,0x1000000201u,0x10000100100000u,0x200000002u,0x1000000000200u,0x2000000000010u,0x2000000000012u,0x200000000000000u,0x20100020000u,0x10000000000010u,0x1000000000200u,0x20000110000u,
        0x10000u,0x201000200000000u,0x2000100000000u,0x3000000000000u,0x1000100000u,0x20000000000u,0x10001000010002u,0x200000000020000u,0x2000000u,0x2010010u,0x200000000000001u,0x20100020000u,0x203000000000000u,0x200100000u,0x100000u,0x10001002000001u,0x1001200000000u,0u,0x2000000u,0x1000000201u,0x20000020000u,0x200000000010002u,0x200000000u,0x100000u,0x212u,0x200001000000000u,0x100030000u,0x200000010u,0x1000000000201u,
        0x2000000100000u,0x2000002u,0x1000000000000u,0x20000u,0x2000000000011u,0u,0u};
        // clang-format on
        constexpr static bool calc(const itype::u16 x) { return (table[x / 64] >> (x % 64)) & 1; }
    };
}  // namespace internal
constexpr bool isSquare32(const itype::u32 x) {
    const itype::u32 tmp = IntSqrt32(x);
    return tmp * tmp == x;
}
constexpr bool isSquare64(const itype::u64 x) {
    if (!internal::isSquareMod9360<0>::calc(x % 9360)) return false;
    const itype::u64 tmp = IntSqrt64(x);
    return tmp * tmp == x;
}

template<class T> constexpr T IntPow(const T x, itype::u64 e) {
    T res = 1, pow = x;
    while (e != 0) {
        const T tmp = pow * pow;
        if (e & 1) res *= pow;
        pow = tmp;
        e >>= 1;
    }
    return res;
}
template<class T> constexpr T ModPow(const T x, itype::u64 e, const T mod) {
    T res = 1, pow = x % mod;
    while (e != 0) {
        const T tmp = (pow * pow) % mod;
        if (e & 1) res = (res * pow) % mod;
        pow = tmp;
        e >>= 1;
    }
    return res;
}

// @brief Find the greatest common divisor as in std::gcd. (https://lpha-z.hatenablog.com/entry/2020/05/24/231500)
// @return std::common_type_t<T, U>
template<class T, class U> constexpr auto GCD(T x, U y) {
    static_assert(!std::is_same_v<T, bool> && !std::is_same_v<U, bool> && std::is_integral_v<T> && std::is_integral_v<U>, "gsh::GCD / The input must be an integral type.");
    if constexpr (std::is_same_v<T, U>) {
        if constexpr (std::is_unsigned_v<T>) {
            return internal::calc_gcd(x, y);
        } else {
            return static_cast<T>(GCD<std::make_unsigned_t<T>, std::make_unsigned<T>>((x < 0 ? -x : x), (y < 0 ? -y : y)));
        }
    } else {
        return GCD<std::common_type_t<T, U>, std::common_type_t<T, U>>(x, y);
    }
}
// @brief Find the greatest common divisor of multiple numbers.
template<class T, class... Args> constexpr auto GCD(T x, Args... y) {
    return GCD(x, GCD(y...));
}
// @brief Find the  least common multiple as in std::lcm.
template<class T, class U> constexpr auto LCM(T x, U y) {
    return static_cast<std::common_type_t<T, U>>(x < 0 ? -x : x) / GCD(x, y) * static_cast<std::common_type_t<T, U>>(y < 0 ? -y : y);
}
// @brief Find the least common multiple of multiple numbers.
template<class T, class... Args> constexpr auto LCM(T x, Args... y) {
    return LCM(x, LCM(y...));
}

namespace internal {
    template<itype::u32> struct KthRootImpl {
        // clang-format off
constexpr static itype::u64 pw3[] = {
1,3,9,27,81,243,729,2187,6561,19683,59049,177147,531441,1594323,4782969,14348907,43046721,129140163,387420489,1162261467,3486784401,10460353203,31381059609,94143178827,282429536481,847288609443,2541865828329,7625597484987,
22876792454961,68630377364883,205891132094649,617673396283947,1853020188851841,5559060566555523,16677181699666569,50031545098999707,150094635296999121,450283905890997363,1350851717672992089,4052555153018976267,12157665459056928801u
};
constexpr static itype::u64 pw5[] = {
1,5,25,125,625,3125,15625,78125,390625,1953125,9765625,48828125,244140625,1220703125,6103515625,30517578125,152587890625,762939453125,3814697265625,
19073486328125,95367431640625,476837158203125,2384185791015625,11920928955078125,59604644775390625,298023223876953125,1490116119384765625,7450580596923828125
};
constexpr static itype::u64 pw7[] = {
1,7,49,343,2401,16807,117649,823543,5764801,40353607,282475249,1977326743,13841287201,96889010407,678223072849,4747561509943,33232930569601,
232630513987207,1628413597910449,11398895185373143,79792266297612001,558545864083284007,3909821048582988049,8922003266371364727,7113790643470898241
};
constexpr static itype::u64 pw11[] = {
1,11,121,1331,14641,161051,1771561,19487171,214358881,2357947691,25937424601,285311670611,3138428376721,34522712143931,379749833583241,4177248169415651,45949729863572161,505447028499293771,5559917313492231481
};
constexpr static itype::u64 pw13[] = {
1,13,169,2197,28561,371293,4826809,62748517,815730721,10604499373,137858491849,1792160394037,23298085122481,302875106592253,3937376385699289,51185893014090757,665416609183179841,8650415919381337933
};
constexpr static itype::u64 pw17[] = {
1,17,289,4913,83521,1419857,24137569,410338673,6975757441,118587876497,2015993900449,34271896307633,582622237229761,9904578032905937,168377826559400929,2862423051509815793
};
constexpr static itype::u64 pw19[] = {
1,19,361,6859,130321,2476099,47045881,893871739,16983563041,322687697779,6131066257801,116490258898219,2213314919066161,42052983462257059,799006685782884121,15181127029874798299u
};
constexpr static itype::u64 pw23[] = {
1,23,529,12167,279841,6436343,148035889,3404825447,78310985281,1801152661463,41426511213649,952809757913927,21914624432020321,504036361936467383,11592836324538749809u
};
constexpr static itype::u64 pw29[] = {
1,29,841,24389,707281,20511149,594823321,17249876309,500246412961,14507145975869,420707233300201,12200509765705829,353814783205469041,10260628712958602189u
};
constexpr static itype::u64 pw31[] = {
1,31,961,29791,923521,28629151,887503681,27512614111,852891037441,26439622160671,819628286980801,25408476896404831,787662783788549761
};
constexpr static itype::u64 pw37[] = {
1,37,1369,50653,1874161,69343957,2565726409,94931877133,3512479453921,129961739795077,4808584372417849,177917621779460413,6582952005840035281
};
constexpr static ftype::f64 iv[] = {
0.0,0x1.fffffffffffffp-1,0x1.fffffffffffffp-2,0x1.5555555555554p-2,0x1.fffffffffffffp-3,0x1.9999999999999p-3,0x1.5555555555554p-3,0x1.2492492492491p-3,0x1.fffffffffffffp-4,0x1.c71c71c71c71bp-4,0x1.9999999999999p-4,0x1.745d1745d1745p-4
};
constexpr static itype::u64 lim[] = {
0,18446744073709551615u,4294967295,2642245,65535,7131,1625,565,255,138,84,56,40,30,23,19,15,13,11,10,9,8,7,6,6,5,5,5,4,4,4,4
};
constexpr static ftype::f64 rt3[] = {
0x1p+0,0x1.965fea53d6e3cp-1,0x1.428a2f98d728bp-1,0x1p-1,0x1.965fea53d6e3cp-2,0x1.428a2f98d728bp-2,0x1p-2,0x1.965fea53d6e3cp-3,0x1.428a2f98d728bp-3,0x1p-3,0x1.965fea53d6e3cp-4,0x1.428a2f98d728bp-4,0x1p-4,0x1.965fea53d6e3cp-5,0x1.428a2f98d728bp-5,0x1p-5,0x1.965fea53d6e3cp-6,
0x1.428a2f98d728bp-6,0x1p-6,0x1.965fea53d6e3cp-7,0x1.428a2f98d728bp-7,0x1p-7,0x1.965fea53d6e3cp-8,0x1.428a2f98d728bp-8,0x1p-8,0x1.965fea53d6e3cp-9,0x1.428a2f98d728bp-9,0x1p-9,0x1.965fea53d6e3cp-10,0x1.428a2f98d728bp-10,0x1p-10,0x1.965fea53d6e3cp-11,0x1.428a2f98d728bp-11,
0x1p-11,0x1.965fea53d6e3cp-12,0x1.428a2f98d728bp-12,0x1p-12,0x1.965fea53d6e3cp-13,0x1.428a2f98d728bp-13,0x1p-13,0x1.965fea53d6e3cp-14,0x1.428a2f98d728bp-14,0x1p-14,0x1.965fea53d6e3cp-15,0x1.428a2f98d728bp-15,0x1p-15,0x1.965fea53d6e3cp-16,0x1.428a2f98d728bp-16,0x1p-16,
0x1.965fea53d6e3cp-17,0x1.428a2f98d728bp-17,0x1p-17,0x1.965fea53d6e3cp-18,0x1.428a2f98d728bp-18,0x1p-18,0x1.965fea53d6e3cp-19,0x1.428a2f98d728bp-19,0x1p-19,0x1.965fea53d6e3cp-20,0x1.428a2f98d728bp-20,0x1p-20,0x1.965fea53d6e3cp-21,0x1.428a2f98d728bp-21,0x1p-21
};
        // clang-format on
        template<itype::u64 K> constexpr static itype::u64 calc(itype::u64 n) {
            if constexpr (K == 3) {
                const itype::u32 t = std::bit_width(n) - 1;
                ftype::f64 x = rt3[t];
                GSH_INTERNAL_UNROLL(3)
                for (itype::u32 i = 0; i != 3; ++i) {
                    ftype::f64 h = 1.0 - n * x * x * x;
                    x += x * h * ((1.0 / 3) + h * ((2.0 / 9) + h * ((14.0 / 81) + (h * ((35.0 / 243) + h * (91.0 / 729))))));
                }
                const itype::u64 r = 1.0 / x;
                return r + (r < lim[K] && (r + 1) * (r + 1) * (r + 1) <= n) - (r > lim[K] || r * r * r > n);
            } else if constexpr (K == 2) return IntSqrt64(n);
            else if constexpr (K == 1) return n;
            else if constexpr (K == 0) return 0xffffffffffffffff;
            else if constexpr (K >= 12) {
                itype::u64 res = 1 + (n >= (1ull << K));
                if constexpr (K < 41) res += (n >= pw3[K]);
                if constexpr (K < 32) res += (n >= (1ull << (2 * K)));
                if constexpr (K < 28) res += (n >= pw5[K]);
                if constexpr (K < 25) res += (n >= (pw3[K] << K));
                if constexpr (K < 23) res += (n >= pw7[K]);
                if constexpr (K < 22) res += (n >= (1ull << (3 * K)));
                if constexpr (K < 21) res += (n >= pw3[K] * pw3[K]);
                if constexpr (K < 20) res += (n >= (pw5[K] << K));
                if constexpr (K < 19) res += (n >= pw11[K]);
                if constexpr (K < 18) res += (n >= (pw3[K] << (2 * K))) + (n >= pw13[K]);
                if constexpr (K < 17) res += (n >= (pw7[K] << K)) + (n >= (pw3[K] * pw5[K]));
                if constexpr (K < 16) res += (n >= (1ull << (4 * K))) + (n >= pw17[K]) + (n >= ((pw3[K] * pw3[K]) << K)) + (n >= pw19[K]);
                if constexpr (K < 15) res += (n >= (pw5[K] << (2 * K))) + (n >= (pw3[K] * pw7[K])) + (n >= (pw11[K] << K)) + (n >= pw23[K]);
                if constexpr (K < 14) res += (n >= (pw3[K] << (3 * K))) + (n >= (pw5[K] * pw5[K])) + (n >= (pw13[K] << K)) + (n >= (pw3[K] * pw3[K] * pw3[K])) + (n >= (pw7[K] << (2 * K))) + (n >= pw29[K]) + (n >= ((pw3[K] * pw5[K]) << K));
                if constexpr (K < 13) res += (n >= pw31[K]) + (n >= (1ull << (5 * K))) + (n >= (pw3[K] * pw11[K])) + (n >= (pw17[K] << K)) + (n >= (pw5[K] * pw7[K])) + (n >= ((pw3[K] * pw3[K]) << (2 * K))) + (n >= pw37[K]) + (n >= (pw19[K] << K)) + (n >= (pw3[K] * pw13[K])) + (n >= (pw5[K] << (3 * K)));
                return res;
            } else {
                const itype::u64 r = static_cast<itype::u64>(std::pow(n, iv[K]));
                itype::u64 a = 1, p = r + 1;
                GSH_INTERNAL_UNROLL(8)
                for (itype::u64 e = K; e != 0; e >>= 1) {
                    if (e & 1) a *= p;
                    p *= p;
                }
                return r + (r < lim[K] && a <= n);
            }
        }
        constexpr static itype::u64 calc2(itype::u64 n, itype::u64 k) {
            if (n == 0) return 0;
                // clang-format off
#ifdef F
#define GSH_INTERNAL_DEFINED
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
#ifdef GSH_INTERNAL_DEFINED
#pragma pop_macro("F")
#endif
            // clang-format on
        }
    };
}  // namespace internal
constexpr itype::u64 KthRoot(itype::u64 n, itype::u64 k) {
    return internal::KthRootImpl<0>::calc2(n, k);
}

constexpr itype::u64 LinearFloorSum(itype::u32 n, itype::u32 m, itype::u32 a, itype::u32 b) {
    itype::u64 res = 0;
    while (true) {
        const itype::u32 p = a / m, q = b / m;
        a %= m;
        b %= m;
        res += static_cast<itype::u64>(n) * (n - 1) / 2 * p + static_cast<itype::u64>(n) * q;
        const itype::u64 last = a * static_cast<itype::u64>(n) + b;
        if (last < m) return res;
        n = last / m;
        b = last % m;
        itype::u32 tmp = a;
        a = m, m = tmp;
    }
}

constexpr itype::u32 LinearModMin(itype::u32 n, itype::u32 m, itype::u32 a, itype::u32 b) {
    itype::u32 res = 0;
    bool z = true;
    itype::u32 p = 1, q = 1;
    while (a != 0) {
        const itype::u32 e = (z ? a : m) - 1;
        const itype::u32 d = m / a, r = m % a;
        const itype::u32 g = d * p + q;
        if ((z ? b + 1 : m - b) > a) {
            const itype::u32 t = (m - b + (z ? a : 0) - 1) / a;
            const itype::u32 c = (t - z) * p + (z ? q : 0);
            if (n <= c) {
                const itype::u32 h = z ? 0 : a * ((n - 1) / p);
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

class QuotientsList {
    const itype::u64 x;
    const itype::u32 sq;
    itype::u32 m;
public:
    using value_type = itype::u32;
    constexpr QuotientsList(itype::u64 n) : x(n), sq(IntSqrt64(n)) { m = (itype::u64(sq) * sq + sq <= n ? sq : sq - 1); }
    constexpr itype::u32 size() const noexcept { return sq + m; }
    constexpr itype::u32 iota_limit() const noexcept { return sq; }
    constexpr itype::u32 div_limit() const noexcept { return m; }
    constexpr itype::u64 val() const noexcept { return x; }
    constexpr itype::u64 operator[](itype::u32 n) { return n < m ? n + 1 : x / (sq - (n - m)); }
};

namespace internal {
    template<class T> class BinCoeffTable {
        T mint;
        Arr<typename T::value_type> fac, finv;
    public:
        using value_type = typename T::value_type;
        constexpr BinCoeffTable(itype::u32 mx, value_type mod) : fac(mx), finv(mx) {
            if (mx >= mod) throw Exception("gsh::internal::BinCoeffTable:::BinCoeffTable / The table size cannot be larger than mod.");
            mint.set(mod);
            fac[0] = mint.raw(1), finv[0] = mint.raw(1);
            if (mx > 1) {
                fac[1] = mint.raw(1), finv[1] = mint.raw(1);
                if (mx > 2) {
                    for (itype::u32 i = 2; i != mx; ++i) fac[i] = mint.mul(fac[i - 1], mint.raw(i));
                    finv.back() = mint.inv(fac.back());
                    for (itype::u32 i = mx - 1; i != 2; --i) finv[i - 1] = mint.mul(finv[i], mint.raw(i));
                }
            }
        }
        constexpr value_type operator()(itype::u32 n, itype::u32 k) const {
            if (n < k) return 0;
            else return mint.val(mint.mul(mint.mul(fac[n], finv[k]), finv[n - k]));
        }
    };
    template<IsStaticModint T> class BinCoeffTable<T> {
        [[no_unique_address]] T mint;
        Arr<typename T::value_type> fac, finv;
    public:
        using value_type = typename T::value_type;
        constexpr BinCoeffTable(itype::u32 mx) : fac(mx), finv(mx) {
            if (mx >= mint.mod()) throw Exception("gsh::internal::BinCoeffTable:::BinCoeffTable / The table size cannot be larger than mod.");
            fac[0] = mint.raw(1), finv[0] = mint.raw(1);
            if (mx > 1) {
                fac[1] = mint.raw(1), finv[1] = mint.raw(1);
                if (mx > 2) {
                    for (itype::u32 i = 2; i != mx; ++i) fac[i] = mint.mul(fac[i - 1], mint.raw(i));
                    finv.back() = mint.inv(fac.back());
                    for (itype::u32 i = mx - 1; i != 2; --i) finv[i - 1] = mint.mul(finv[i], mint.raw(i));
                }
            }
        }
        constexpr value_type operator()(itype::u32 n, itype::u32 k) const {
            if (n < k) return 0;
            else return mint.val(mint.mul(mint.mul(fac[n], finv[k]), finv[n - k]));
        }
    };
}  // namespace internal
using BinCoeffTable32 = internal::BinCoeffTable<internal::DynamicModint32Impl>;
using BinCoeffTable64 = internal::BinCoeffTable<internal::DynamicModint64Impl>;
template<itype::u64 mod = 998244353> using BinCoeffTableStaticMod = internal::BinCoeffTable<internal::StaticModintImpl<mod>>;

}  // namespace gsh
