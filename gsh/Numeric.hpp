#pragma once
#include <type_traits>
#include <cmath>
#include <gsh/Modint.hpp>
#include <gsh/TypeDef.hpp>

namespace gsh {

//@brief Find the largest x for which x * x <= n (https://rsk0315.hatenablog.com/entry/2023/11/07/221428)
constexpr itype::u32 IntSqrt(const itype::u32 x) {
    if (x == 0) return 0;
    itype::u32 tmp = static_cast<itype::u32>(std::sqrt(static_cast<ftype::f32>(x))) - 1;
    return tmp + (tmp * (tmp + 2) < x);
}
constexpr itype::u64 IntSqrt(const itype::u64 x) {
    if (x == 0) return 0;
    itype::u64 tmp = static_cast<itype::u64>(std::sqrt(static_cast<ftype::f64>(x))) - 1;
    return tmp + (tmp * (tmp + 2) < x);
}
namespace internal {
    bool isSquare_mod9360(const itype::u16 x) {
        // clang-format off
        const static itype::u64 table[147] = {0x2001002010213u,0x200001000020001u,0x20100010000u,0x10000200000010u,0x200000001u,0x20000000010u,0x200000000010000u,0x1200000000u,0x20000u,0x2000002000201u,0x1000000201u,0x20002100000u,0x10000000010000u,0x1000000000200u,0x2000000000010u,0x2010002u,0x100001u,0x20002u,0x210u,0x1000200000200u,0x110000u,0x2000000u,0x201001100000000u,0x2000100000000u,0x2000002000000u,0x201u,
        0x20002u,0x10001000000002u,0x200000000000000u,0x2100000u,0x10012u,0x200020100000000u,0x20100000000u,0x2000000000010u,0x1000200100200u,0u,0x10001000000003u,0x1200000000u,0x10000000000000u,0x2000002000010u,0x21000000001u,0x20100000000u,0x10000000010000u,0x200000200000000u,0u,0x2001000010200u,0x1000020000u,0x20000u,0x12000000000000u,0x1000200000201u,0x2020000100000u,0x10000002010000u,0x1001000000000u,0x20000u,
        0x2000000u,0x1u,0x10000000130000u,0x2u,0x201000300000200u,0x2000000100010u,0x2000010u,0x200001000000001u,0x100000002u,0x2000000000000u,0x1000000000201u,0x2010000u,0x10000000000002u,0x200020100000000u,0x100020010u,0x10u,0x200u,0x20100100000u,0x1000010000u,0x201000200020200u,0x2000000u,0x2000000000002u,0x21000000000u,0x20000000000u,0x13000000000010u,0x1u,0x20000000002u,0x10000002010001u,0x200000200020000u,
        0x100020000u,0x2000200000000u,0x1000000000u,0x120000u,0x211000000000000u,0x1000200000200u,0x100000u,0x2010201u,0x1000020001u,0x10020000020000u,0u,0x200000001u,0x100010u,0x200000000000002u,0x201001200000000u,0x100020000u,0x2000210u,0x1000000201u,0x10000100100000u,0x200000002u,0x1000000000200u,0x2000000000010u,0x2000000000012u,0x200000000000000u,0x20100020000u,0x10000000000010u,0x1000000000200u,0x20000110000u,
        0x10000u,0x201000200000000u,0x2000100000000u,0x3000000000000u,0x1000100000u,0x20000000000u,0x10001000010002u,0x200000000020000u,0x2000000u,0x2010010u,0x200000000000001u,0x20100020000u,0x203000000000000u,0x200100000u,0x100000u,0x10001002000001u,0x1001200000000u,0u,0x2000000u,0x1000000201u,0x20000020000u,0x200000000010002u,0x200000000u,0x100000u,0x212u,0x200001000000000u,0x100030000u,0x200000010u,0x1000000000201u,
        0x2000000100000u,0x2000002u,0x1000000000000u,0x20000u,0x2000000000011u,0u,0u};
        // clang-format on
        return (table[x / 64] >> (x % 64)) & 1;
    }
}  // namespace internal
constexpr bool isSquare(const itype::u32 x) {
    const itype::u32 tmp = IntSqrt(x);
    return tmp * tmp == x;
}
constexpr bool isSquare(const itype::u64 x) {
    if (!std::is_constant_evaluated() && !internal::isSquare_mod9360(x % 9360)) return false;
    const itype::u64 tmp = IntSqrt(x);
    return tmp * tmp == x;
}

template<class T> constexpr T IntPow(const T x, itype::u64 e) {
    T res = 1, pow = x;
    while (e != 0) {
        if (e & 1) res *= pow;
        pow *= pow;
        e >>= 1;
    }
    return res;
}
template<class T> constexpr T PowMod(const T x, itype::u64 e, const T mod) {
    T res = 1, pow = x % mod;
    while (e != 0) {
        if (e & 1) res = (res * pow) % mod;
        pow = (pow * pow) % mod;
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

constexpr itype::u64 LinearFloorSum(itype::u32 n, itype::u32 m, itype::u32 a, itype::u32 b) {
    itype::u64 res = 0;
    while (true) {
        const itype::u32 p = a / m, q = b / m;
        a %= m;
        b %= m;
        res += (itype::u64) n * (n - 1) / 2 * p + (itype::u64) n * q;
        const itype::u64 last = a * (itype::u64) n + b;
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
    constexpr QuotientsList(itype::u64 n) : x(n), sq(IntSqrt(n)) { m = (itype::u64(sq) * sq + sq <= n ? sq : sq - 1); }
    constexpr itype::u32 size() const noexcept { return sq + m; }
    constexpr itype::u32 iota_limit() const noexcept { return sq; }
    constexpr itype::u32 div_limit() const noexcept { return m; }
    constexpr itype::u64 val() const noexcept { return x; }
    constexpr itype::u64 operator[](itype::u32 n) { return n < m ? n + 1 : x / (sq - (n - m)); }
};

}  // namespace gsh
