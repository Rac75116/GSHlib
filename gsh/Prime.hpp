#pragma once
#include <algorithm>
#include <bit>
#include "TypeDef.hpp"
#include "Modint.hpp"
#include "Vec.hpp"
#include "Numeric.hpp"
#include "Int128.hpp"

namespace gsh {

namespace internal {

    template<itype::u32> struct IsPrime8 {
        constexpr static itype::u64 flag_table[4] = { 2891462833508853932u, 9223979663092122248u, 9234666804958202376u, 577166812715155618u };
        GSH_INTERNAL_INLINE constexpr static bool calc(const itype::u8 n) noexcept { return (flag_table[n / 64] >> (n % 64)) & 1; }
    };
    template<itype::u32> struct IsPrime16 {
        constexpr static itype::u64 flag_table[512] = {
#include "internal/PrimeFlag.txt"
        };
        GSH_INTERNAL_INLINE constexpr static bool calc(const itype::u16 x) noexcept { return x == 2 || (x % 2 == 1 && (flag_table[x / 128] & (1ull << (x % 128 / 2)))); }
    };

    template<itype::u32> struct IsPrime32 {
        // clang-format off
        constexpr static itype::u16 bases[] = {
1216,1836,8885,4564,10978,5228,15613,13941,1553,173,3615,3144,10065,9259,233,2362,6244,6431,10863,5920,6408,6841,22124,2290,45597,6935,4835,7652,1051,445,5807,842,1534,22140,1282,1733,347,6311,14081,11157,186,703,9862,15490,1720,17816,10433,49185,2535,9158,2143,2840,664,29074,24924,1035,41482,1065,10189,8417,130,4551,5159,48886,
786,1938,1013,2139,7171,2143,16873,188,5555,42007,1045,3891,2853,23642,148,3585,3027,280,3101,9918,6452,2716,855,990,1925,13557,1063,6916,4965,4380,587,3214,1808,1036,6356,8191,6783,14424,6929,1002,840,422,44215,7753,5799,3415,231,2013,8895,2081,883,3855,5577,876,3574,1925,1192,865,7376,12254,5952,2516,20463,186,
5411,35353,50898,1084,2127,4305,115,7821,1265,16169,1705,1857,24938,220,3650,1057,482,1690,2718,4309,7496,1515,7972,3763,10954,2817,3430,1423,714,6734,328,2581,2580,10047,2797,155,5951,3817,54850,2173,1318,246,1807,2958,2697,337,4871,2439,736,37112,1226,527,7531,5418,7242,2421,16135,7015,8432,2605,5638,5161,11515,14949,
748,5003,9048,4679,1915,7652,9657,660,3054,15469,2910,775,14106,1749,136,2673,61814,5633,1244,2567,4989,1637,1273,11423,7974,7509,6061,531,6608,1088,1627,160,6416,11350,921,306,18117,1238,463,1722,996,3866,6576,6055,130,24080,7331,3922,8632,2706,24108,32374,4237,15302,287,2296,1220,20922,3350,2089,562,11745,163,11951};
        // clang-format on
        GSH_INTERNAL_INLINE constexpr static bool calc(const itype::u32 x) noexcept {
            internal::MontgomeryModint64Impl mint;
            mint.set(x);
            const itype::u32 h = x * 0xad625b89;
            itype::u32 d = x - 1;
            auto pow = mint.raw(bases[h >> 24]);
            itype::u32 s = std::countr_zero(d);
            d >>= s;
            const auto one = mint.one(), mone = mint.neg(one);
            auto cur = one;
            while (d) {
                auto tmp = mint.mul(pow, pow);
                if (d & 1) cur = mint.mul(cur, pow);
                pow = tmp;
                d >>= 1;
            }
            if (mint.same(cur, one)) return true;
            while (--s && !mint.same(cur, mone)) cur = mint.mul(cur, cur);
            return mint.same(cur, mone);
        }
    };

    template<bool Prob, itype::u32> struct IsPrime64;
    template<itype::u32 id> struct IsPrime64<false, id> {
        GSH_INTERNAL_INLINE constexpr static bool calc(const itype::u64 x) noexcept {
            internal::MontgomeryModint64Impl mint;
            mint.set(x);
            const itype::u32 S = std::countr_zero(x - 1);
            const itype::u64 D = (x - 1) >> S;
            const auto one = mint.one(), mone = mint.neg(one);
            auto test2 = [&](itype::u64 base1, itype::u64 base2) {
                auto a = one, b = one;
                auto c = mint.build(base1), d = mint.build(base2);
                itype::u64 ex = D;
                while (ex) {
                    auto e = mint.mul(c, c), f = mint.mul(d, d);
                    if (ex & 1) a = mint.mul(a, e), b = mint.mul(b, f);
                    c = e, d = f;
                    ex >>= 1;
                }
                bool res1 = mint.same(a, one) || mint.same(a, mone);
                bool res2 = mint.same(b, one) || mint.same(b, mone);
                if (!(res1 && res2)) {
                    for (itype::u32 i = 0; i != S - 1; ++i) {
                        a = mint.mul(a, a), b = mint.mul(b, b), c = mint.mul(c, c);
                        res1 |= mint.same(a, mone), res2 |= mint.same(b, mone);
                    }
                    if (!res1 || !res2) return false;
                }
                return true;
            };
            auto test3 = [&](itype::u64 base1, itype::u64 base2, itype::u64 base3) {
                auto a = one, b = one, c = one;
                auto d = mint.build(base1), e = mint.build(base2), f = mint.build(base3);
                itype::u64 ex = D;
                while (ex) {
                    const auto g = mint.mul(d, d), h = mint.mul(e, e), i = mint.mul(f, f);
                    if (ex & 1) a = mint.mul(a, d), b = mint.mul(b, e), c = mint.mul(c, f);
                    d = g, e = h, f = i;
                    ex >>= 1;
                }
                bool res1 = mint.same(a, one) || mint.same(a, mone);
                bool res2 = mint.same(b, one) || mint.same(b, mone);
                bool res3 = mint.same(c, one) || mint.same(c, mone);
                if (!(res1 && res2 && res3)) {
                    for (itype::u32 i = 0; i != S - 1; ++i) {
                        a = mint.mul(a, a), b = mint.mul(b, b), c = mint.mul(c, c);
                        res1 |= mint.same(a, mone), res2 |= mint.same(b, mone), res3 |= mint.same(c, mone);
                    }
                    if (!res1 || !res2 || !res3) return false;
                }
                return true;
            };
            auto test4 = [&](itype::u64 base1, itype::u64 base2, itype::u64 base3, itype::u64 base4) {
                auto a = one, b = one, c = one, d = one;
                auto e = mint.build(base1), f = mint.build(base2), g = mint.build(base3), h = mint.build(base4);
                itype::u64 ex = D;
                while (ex) {
                    auto i = mint.mul(e, e), j = mint.mul(f, f), k = mint.mul(g, g), l = mint.mul(h, h);
                    if (ex & 1) a = mint.mul(a, e), b = mint.mul(b, f), c = mint.mul(c, g), d = mint.mul(d, h);
                    e = i, f = j, g = k, h = l;
                    ex >>= 1;
                }
                bool res1 = mint.same(a, one) || mint.same(a, mone);
                bool res2 = mint.same(b, one) || mint.same(b, mone);
                bool res3 = mint.same(c, one) || mint.same(c, mone);
                bool res4 = mint.same(d, one) || mint.same(d, mone);
                if (!(res1 && res2 && res3 && res4)) {
                    for (itype::u32 i = 0; i != S - 1; ++i) {
                        a = mint.mul(a, a), b = mint.mul(b, b), c = mint.mul(c, c), d = mint.mul(d, d);
                        res1 |= mint.same(a, mone), res2 |= mint.same(b, mone), res3 |= mint.same(c, mone), res4 |= mint.same(d, mone);
                    }
                    if (!res1 || !res2 || !res3 || !res4) return false;
                }
                return true;
            };
            if (x < 585226005592931977ull) {
                if (x < 7999252175582851ull) {
                    if (x < 350269456337ull) return test3(4230279247111683200ull, 14694767155120705706ull, 16641139526367750375ull);
                    else if (x < 55245642489451ull) return test4(2ull, 141889084524735ull, 1199124725622454117ull, 11096072698276303650ull);
                    else return test2(2ull, 4130806001517ull) && test3(149795463772692060ull, 186635894390467037ull, 3967304179347715805ull);
                } else return test3(2ull, 123635709730000ull, 9233062284813009ull) && test3(43835965440333360ull, 761179012939631437ull, 1263739024124850375ull);
            } else return test3(2ull, 325ull, 9375ull) && test4(28178ull, 450775ull, 9780504ull, 1795265022ull);
        }
    };
    template<itype::u32 id> struct IsPrime64<true, id> {
        constexpr static itype::u16 bases1[] = {
#include "internal/MRbase.txt"
        };
        constexpr static itype::u64 bases2 = 15ull | (135ull << 8) | (13ull << 16) | (60ull << 24) | (15ull << 32) | (117ull << 40) | (65ull << 48) | (29ull << 56);
        GSH_INTERNAL_INLINE constexpr static bool calc(const itype::u64 x) noexcept {
            internal::MontgomeryModint64Impl mint;
            mint.set(x);
            const itype::u32 S = std::countr_zero(x - 1);
            const itype::u64 D = (x - 1) >> S;
            const auto one = mint.one(), mone = mint.neg(one);
            auto test2 = [&](itype::u32 base1, itype::u32 base2) {
                auto a = one, b = one;
                auto c = mint.raw(base1), d = mint.raw(base2);
                itype::u64 ex = D;
                while (ex) {
                    auto e = mint.mul(c, c), f = mint.mul(d, d);
                    if (ex & 1) a = mint.mul(a, e), b = mint.mul(b, f);
                    c = e, d = f;
                    ex >>= 1;
                }
                bool res1 = mint.same(a, one) || mint.same(a, mone);
                bool res2 = mint.same(b, one) || mint.same(b, mone);
                for (itype::u32 i = 0; i != S - 1; ++i) {
                    a = mint.mul(a, a), b = mint.mul(b, b);
                    res1 |= mint.same(a, mone), res2 |= mint.same(b, mone);
                }
                return res1 && res2;
            };
            auto test3 = [&](itype::u32 base1, itype::u32 base2, itype::u32 base3) {
                auto a = one, b = one, c = one;
                auto d = mint.raw(base1), e = mint.raw(base2), f = mint.raw(base3);
                itype::u64 ex = D;
                while (ex) {
                    const auto g = mint.mul(d, d), h = mint.mul(e, e), i = mint.mul(f, f);
                    if (ex & 1) a = mint.mul(a, d), b = mint.mul(b, e), c = mint.mul(c, f);
                    d = g, e = h, f = i;
                    ex >>= 1;
                }
                bool res1 = mint.same(a, one) || mint.same(a, mone);
                bool res2 = mint.same(b, one) || mint.same(b, mone);
                bool res3 = mint.same(c, one) || mint.same(c, mone);
                for (itype::u32 i = 0; i != S - 1; ++i) {
                    a = mint.mul(a, a), b = mint.mul(b, b), c = mint.mul(c, c);
                    res1 |= mint.same(a, mone), res2 |= mint.same(b, mone), res3 |= mint.same(c, mone);
                }
                return res1 && res2 && res3;
            };
            const itype::u32 base = bases1[(0xad625b89u * static_cast<itype::u32>(x)) >> 18];
            if (x < (1ull << 49)) return test2(2, base);
            else return test3(2, base, (bases2 >> (8 * (base >> 13))) & 0xff);
        }
    };

}  // namespace internal

// @brief Prime number determination
template<bool Prob = true> constexpr bool IsPrime(const itype::u64 x) noexcept {
    if (x < 65536u) {
        return internal::IsPrime16<0>::calc(x);
    } else {
        if (x % 2 == 0 || x % 3 == 0 || x % 5 == 0 || x % 7 == 0 || x % 11 == 0 || x % 13 == 0 || x % 17 == 0 || x % 19 == 0) return false;
        if (x <= 0xffffffff) return internal::IsPrime32<0>::calc(x);
        else return internal::IsPrime64<Prob, 0>::calc(x);
    }
}

constexpr itype::u32 CountPrimes(itype::u64 N) {
    if (N <= 1) return 0;
    const itype::u32 v = IntSqrt64(N);
    itype::u32 s = (v + 1) / 2;
    itype::u64* const invs = new itype::u64[s];
    itype::u32* const smalls = new itype::u32[s];
    itype::u32* const larges = new itype::u32[s];
    itype::u32* const roughs = new itype::u32[s];
    bool* const smooth = new bool[v + 1];
    for (itype::u32 i = 0; i != v; ++i) smooth[i] = false;
    for (itype::u32 i = 0; i != s; ++i) smalls[i] = i;
    for (itype::u32 i = 0; i != s; ++i) roughs[i] = 2 * i + 1;
    for (itype::u32 i = 0; i != s; ++i) invs[i] = (ftype::f64) N / roughs[i];
    for (itype::u32 i = 0; i != s; ++i) larges[i] = (invs[i] - 1) / 2;
    itype::u32 pc = 0;
    for (itype::u64 p = 3; p * p <= v; p += 2) {
        if (smooth[p]) continue;
        for (itype::u64 i = p * p; i <= v; i += 2 * p) smooth[i] = true;
        smooth[p] = true;
        const auto divide_p = [invp = 0xffffffffffffffffu / p + 1](itype::u64 inv_j) -> itype::u64 {
            return (itype::u128(inv_j) * invp) >> 64;
        };
        itype::u32 ns = 0;
        itype::u32 k = 0;
        GSH_INTERNAL_UNROLL(16)
        for (; true; ++k) {
            const itype::u32 j = roughs[k];
            if (j * p > v) break;
            if (smooth[j]) continue;
            larges[ns] = larges[k] - larges[smalls[j * p / 2] - pc] + pc;
            invs[ns] = invs[k];
            roughs[ns] = roughs[k];
            ++ns;
        }
        GSH_INTERNAL_UNROLL(16)
        for (; k < s; ++k) {
            const itype::u32 j = roughs[k];
            if (smooth[j]) continue;
            larges[ns] = larges[k] - smalls[(divide_p(invs[k]) - 1) / 2] + pc;
            invs[ns] = invs[k];
            roughs[ns] = roughs[k];
            ++ns;
        }
        s = ns;
        itype::u64 i = (v - 1) / 2;
        for (itype::u64 j = (divide_p(v) - 1) | 1; j >= p; j -= 2) {
            const itype::u32 d = smalls[j / 2] - pc;
            for (; i >= j * p / 2; --i) smalls[i] -= d;
        }
        ++pc;
    }
    itype::u32 ret = 1;
    ret += larges[0] + s * (s - 1) / 2 + (pc - 1) * (s - 1);
    for (itype::u32 k = 1; k < s; ++k) ret -= larges[k];
    for (itype::u32 k1 = 1; k1 < s; ++k1) {
        const itype::u64 p = roughs[k1];
        const auto divide_p = [invp = 0xffffffffffffffffu / p + 1](itype::u64 inv_j) -> itype::u64 {
            return (itype::u128(inv_j) * invp) >> 64;
        };
        const itype::u32 k2_max = smalls[(divide_p(invs[k1]) - 1) / 2] - pc;
        if (k2_max <= k1) break;
        for (itype::u32 k2 = k1 + 1; k2 <= k2_max; ++k2) ret += smalls[(divide_p(invs[k2]) - 1) / 2];
        ret -= (k2_max - k1) * (pc + k1 - 1);
    }
    delete[] invs;
    delete[] smalls;
    delete[] larges;
    delete[] roughs;
    delete[] smooth;
    return ret;
}

//constexpr auto EnumeratePrimes(itype::u32 N, itype::u32 gap, itype::u32 start) {}

namespace internal {
#ifndef GSH_USE_COMPILE_TIME_CALCULATION
    struct TinyPrimesT {
        itype::u16 table[6548] = {
#define GSH_INTERNAL_INCLUDE_TINYPRIMES "internal/TinyPrimes.txt"
#include GSH_INTERNAL_INCLUDE_TINYPRIMES
            , 0, 0, 0, 0, 0, 0
        };
    };
    template<itype::u32> constexpr TinyPrimesT TinyPrimes;
#else
    template<itype::u32 id> constexpr auto TinyPrimes = []() {
        struct {
            itype::u16 table[6548] = {};
        } res;
        itype::u32 cnt = 0;
        for (itype::u32 i = 0; i != (1 << 16); ++i) {
            if (IsPrime16<id>::calc(i)) res.table[cnt++] = i;
        }
        return res;
    }();
#endif
    template<itype::u32 id> constexpr auto InvPrime = []() {
        struct {
            itype::u64 table[6548] = {};
        } res;
        for (itype::u32 i = 0; i != 6542; ++i) {
            res.table[i] = 0xffffffffffffffff / TinyPrimes<id>.table[i] + 1;
        }
        return res;
    }();
    constexpr itype::u64* FactorizeSub64(itype::u64 n, itype::u64* res) noexcept {
        Assume(n % 2 != 0 && n % 3 != 0 && n % 5 != 0 && n % 7 != 0 && n % 11 != 0 && n % 13 != 0 && n % 17 != 0 && n % 19 != 0);
        if (IsPrime(n)) {
            *(res++) = n;
            return res;
        }
        if (n <= 0xffffffff) {
            for (itype::u32 i = 8; n >= 529; ++i) {
                itype::u64 m = InvPrime<0>.table[i];
                if (m * n < m) {
                    itype::u64 p = TinyPrimes<0>.table[i];
                    do {
                        *(res++) = p;
                        n = (static_cast<itype::u128>(m) * n) >> 64;
                    } while (m * n < m);
                }
            }
            if (n != 1) *(res++) = n;
            return res;
        }
        itype::u64 m = n;
        {
            internal::MontgomeryModint64Impl mint;
            mint.set(n);
            for (itype::u32 k = 2; m == n; ++k) {
                itype::u64 f = mint.raw(k);
                for (itype::u32 i = 1;; i += 64) {
                    itype::u64 prev = f;
                    itype::u64 s = mint.one();
                    for (itype::u32 j = 0; j != 64; ++j) {
                        f = mint.pow(f, i + j);
                        s = mint.mul(s, mint.dec(f));
                    }
                    itype::u64 g = GCD(mint.val(s), n);
                    if (g == 1) continue;
                    if (g == n) {
                        g = 1;
                        f = prev;
                        for (itype::u32 j = 0; g == 1; ++j) {
                            f = mint.pow(f, i + j);
                            g = GCD(mint.val(mint.dec(f)), n);
                        }
                    }
                    m = g;
                    break;
                }
            }
        }
        if (n / m < 529) *(res++) = n / m;
        else res = FactorizeSub64(n / m, res);
        if (m < 529) {
            *(res++) = m;
            return res;
        } else return FactorizeSub64(m, res);
    }
}  // namespace internal
constexpr Arr<itype::u64> Factorize(itype::u64 n) {
    if (n <= 1) [[unlikely]]
        return {};
    itype::u64 res[64];
    itype::u64* p = res;
    {
        Assume(n != 0);
        itype::u32 rz = std::countr_zero(n);
        n >>= rz;
        for (itype::u32 i = 0; i != rz; ++i) *(p++) = 2;
    }
    {
        const bool a = n % 3 == 0, b = n % 5 == 0, c = n % 7 == 0, d = n % 11 == 0, e = n % 13 == 0, f = n % 17 == 0, g = n % 19 == 0;
        if (a) [[unlikely]] {
            do {
                n /= 3;
                *(p++) = 3;
            } while (n % 3 == 0);
        }
        if (b) [[unlikely]] {
            do {
                n /= 5;
                *(p++) = 5;
            } while (n % 5 == 0);
        }
        if (c) [[unlikely]] {
            do {
                n /= 7;
                *(p++) = 7;
            } while (n % 7 == 0);
        }
        if (d) [[unlikely]] {
            do {
                n /= 11;
                *(p++) = 11;
            } while (n % 11 == 0);
        }
        if (e) [[unlikely]] {
            do {
                n /= 13;
                *(p++) = 13;
            } while (n % 13 == 0);
        }
        if (f) [[unlikely]] {
            do {
                n /= 17;
                *(p++) = 17;
            } while (n % 17 == 0);
        }
        if (g) [[unlikely]] {
            do {
                n /= 19;
                *(p++) = 19;
            } while (n % 19 == 0);
        }
    }
    if (n >= 529) [[likely]] {
        p = internal::FactorizeSub64(n, p);
    } else {
        *p = n;
        p += n != 1;
    }
    return { res, p };
}

}  // namespace gsh
