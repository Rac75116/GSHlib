#pragma once
#include <type_traits>
#include <cmath>
#include <new>
#include <gsh/Modint.hpp>
#include <gsh/TypeDef.hpp>
#include <gsh/Arr.hpp>

namespace gsh {

//@brief Find the largest x for which x * x <= n (https://rsk0315.hatenablog.com/entry/2023/11/07/221428)
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

template<class T> constexpr itype::i32 Legendre(const T& x) noexcept {
    auto res = x.pow((T::mod() - 1) >> 1).val();
    return (res <= 1 ? static_cast<itype::i32>(res) : -1);
}
template<bool skip_calc_gcd = false, class T> constexpr itype::i32 Jacobi(const T& x) noexcept {
    auto a = x.val(), n = T::mod();
    if (a == 1) return 1;
    if constexpr (!skip_calc_gcd)
        if (internal::calc_gcd(a, n) != 1) return 0;
    itype::i32 res = 1;
    while (a != 0) {
        while (!(a & 1) && a != 0) {
            a >>= 1;
            res = ((n & 0b111) == 3 || (n & 0b111) == 5) ? -res : res;
        }
        res = ((a & 0b11) == 3 || (n & 0b11) == 3) ? -res : res;
        auto tmp = n;
        n = a;
        a = tmp;
        a %= n;
    }
    return n == 1 ? res : 0;
}
template<class T> constexpr Option<T> ModSqrt(const T& n) noexcept {
    const auto vl = n.val(), md = T::mod();
    if (vl <= 1) return n;
    auto get_min = [](T x) {
        return x.val() > (T::mod() >> 1) ? -x : x;
    };
    if ((md & 0b11) == 3) {
        T res = n.pow((md + 1) >> 2);
        if (res * res != n) return Null;
        else return get_min(res);
    } else if ((md & 0b111) == 5) {
        T res = n.pow((md + 3) >> 3);
        if constexpr (T::is_static_mod) {
            constexpr T p = T::raw(2).pow((md - 1) >> 2);
            res *= p;
        } else if (res * res != n) res *= T::raw(2).pow((md - 1) >> 2);
        if (res * res != n) return Null;
        else return get_min(res);
    } else {
        const itype::u32 S = std::countr_zero(md - 1);
        const auto Q = (md - 1) >> S;
        const itype::u32 W = std::bit_width(md);
        if (S * S <= 12 * W) {
            const T tmp = n.pow(Q / 2);
            T R = tmp * n, t = R * tmp;
            if (t.val() == 1) return R;
            T u = t;
            for (itype::u32 i = 0; i != S - 1; ++i) u *= u;
            if (u.val() != 1) return Null;
            const T z = [&]() {
                if (md % 3 == 2) return T::raw(3);
                if (auto x = md % 5; x == 2 || x == 3) return T::raw(5);
                if (auto x = md % 7; x == 3 || x == 5 || x == 6) return T::raw(7);
                if (auto x = md % 11; x == 2 || x == 6 || x == 7 || x == 8 || x == 10) return T(11);
                if (auto x = md % 13; x == 2 || x == 5 || x == 6 || x == 7 || x == 8 || x == 11) return T(13);
                for (const itype::u32 x : { 17, 19, 23, 29, 31, 37, 41, 43, 47, 53, 59, 61, 67, 71, 73, 79, 83, 89, 97 }) {
                    if (Legendre(T(x)) == -1) return T(x);
                }
                T z = 101;
                while (Legendre(z) != -1) z += 2;
                return z;
            }();
            itype::u32 M = S;
            T c = z.pow(Q);
            do {
                T U = t * t;
                itype::u32 i = 1;
                while (U.val() != 1) U = U * U, ++i;
                T b = c;
                for (itype::u32 j = 0, k = M - i - 1; j < k; ++j) b *= b;
                M = i, c = b * b, t *= c, R *= b;
            } while (t.val() != 1);
            return get_min(R);
        } else {
            if (Legendre(n) != 1) return Null;
            T a = 2;
            while (Legendre(a * a - n) != -1) ++a;
            T res1 = T::raw(1), res2, pow1 = a, pow2 = T::raw(1), w = a * a - n;
            auto e = (md + 1) / 2;
            while (true) {
                const T tmp2 = pow2 * w;
                if (e & 1) {
                    const T tmp = res1;
                    res1 = res1 * pow1 + res2 * tmp2;
                    res2 = tmp * pow2 + res2 * pow1;
                }
                e >>= 1;
                if (e == 0) return get_min(res1);
                const T tmp = pow1;
                pow1 = pow1 * pow1 + pow2 * tmp2;
                pow2 *= tmp + tmp;
            }
        }
    }
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
        constexpr BinCoeffTable(itype::u32 mx, value_type mod) : fac(mod < mx ? mod : mx), finv(mod < mx ? mod : mx) {
            mx = mod < mx ? mod : mx;
            mint.set_mod(mod);
            fac[0] = mint.raw(1), finv[0] = mint.raw(1);
            if (mx > 1) {
                fac[1] = mint.raw(1), finv[1] = mint.raw(1);
                if (mx > 2) {
                    for (itype::u32 i = 2; i != mx; ++i) fac[i] = mint.mul(fac[i - 1], mint.raw(i));
                    value_type e = mod - 2;
                    auto res = mint.raw(1), pow = fac.back();
                    while (e) {
                        const auto tmp = mint.mul(pow, pow);
                        if (e & 1) res = mint.mul(res, pow);
                        pow = tmp;
                        e >>= 1;
                    }
                    finv.back() = res;
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

template<itype::u64 mod = 998244353> class BinCoeffTableStaticMod {
    using mint = StaticModint<mod>;
    Arr<mint> fac, finv;
public:
    using value_type = typename mint::value_type;
    constexpr BinCoeffTableStaticMod(itype::u32 mx) : fac(mx), finv(mx) {
        fac[0] = mint::raw(1), finv[0] = mint::raw(1);
        if (mx > 1) {
            fac[1] = mint::raw(1), finv[1] = mint::raw(1);
            if (mx > 2) {
                for (itype::u32 i = 2; i != mx; ++i) fac[i] = fac[i - 1] * mint::raw(i);
                finv.back() = fac.back().pow(mod - 2);
                for (itype::u32 i = mx - 1; i != 2; --i) finv[i - 1] = finv[i] * mint::raw(i);
            }
        }
    }
    constexpr value_type operator()(itype::u32 n, itype::u32 k) const {
        if (n < k) return 0;
        else return (fac[n] * finv[k] * finv[n - k]).val();
    }
};

}  // namespace gsh
