#pragma once
#include "TypeDef.hpp"
#include "Util.hpp"
#ifdef _MSC_VER
#include <intrin.h>
#include <immintrin.h>
#endif

namespace gsh {

namespace internal {
    GSH_INTERNAL_INLINE constexpr itype::u64 Divu128(itype::u64 high, itype::u64 low, itype::u64 div, itype::u64& rem) noexcept {
#if defined(__GNUC__) && defined(__x86_64__)
        if constexpr (sizeof(void*) == 8) {
            if (!std::is_constant_evaluated()) {
                itype::u64 res;
                __asm__("divq %[v]" : "=a"(res), "=d"(rem) : [v] "r"(10000000000000000), "a"(low), "d"(high));
                return res;
            }
        }
#elif defined(_MSC_VER)
        if (!std::is_constant_evaluated()) {
            return _udiv128(high, low, div, &rem);
        }
#endif
#ifdef __GNUC__
        itype::u64 n = (static_cast<__uint128_t>(high) << 64 | low);
        __uint128_t res = n / div;
        rem = n - res * div;
        return res;
#endif
    }
}  // namespace internal

namespace itype {

#ifdef false && __SIZEOF_INT128__
    using i128 = __int128_t;
    using u128 = __uint128_t;
#else
    class u128 {
        u64 a, b;
    public:
        constexpr u128() noexcept : a(0), b(0) {}
        constexpr u128(const u128&) noexcept = default;
        constexpr u128& operator=(const u128&) noexcept = default;
        constexpr u128& operator+=(const u128& n) noexcept {
            a += n.a + static_cast<u64>(0xffffffffffffffff - b < n.b);
            b += n.b;
            return *this;
        }
        constexpr u128& operator-=(const u128& n) noexcept {
            a -= n.a + static_cast<u64>(b < n.b);
            b -= n.b;
            return *this;
        }
        constexpr u128& operator*=(const u128& n) noexcept {
#ifdef _MSC_VER
            if (std::is_constant_evaluated()) {
#endif
                u64 ah = b >> 32, al = b & 0xffffffff, bh = n.b >> 32, bl = n.b & 0xffffffff;
                u64 chh = ah * bh, cll = al * bl, clh = al * bh, chl = ah * bl;
                u64 cm = clh + chl;
                chh += static_cast<u64>(cm < clh) << 32;
                u64 tmp1 = chh + (cm >> 32);
                u64 tmp2 = cll + (cm << 32);
                tmp1 += (tmp2 < cll);
                a = tmp1 + a * n.b + b * n.a;
                b = tmp2;
                return *this;
#ifdef _MSC_VER
            }
            u64 tmp1, tmp2;
            tmp2 = _umul128(b, n.b, tmp1);
            a = tmp1 + a * n.b + b * n.a;
            b = tmp2;
            return *this;
#endif
        }
        constexpr u128& operator/=(const u128& n) noexcept {
#ifdef _MSC_VER
            auto udiv128 = [](u64 a, u64 b, u64 dv) [[msvc::forceinline]] noexcept {
                u64 rem;
                return _udiv128(a, b, dv, &rem);
            };
#else
            auto udiv128 = [](u64 a, u64 b, u64 dv) noexcept {
#ifdef x64_x86
#endif
            };
#endif
            /*
            (xh #xl) = x;
            (yh #yl) = y;
            if (yh == 0) {
                if (xh >= yl) {
                    // 商が64bitに収まらない時
                    if (yl == 0) {
                        // 零除算例外を発生させる
                        rcx = 1 / yl;
                    }
                    // 長除法
                    qh = xh / yl;
                    r = xh % yl;
                    ql = (r #xl) / yl;
                    return (qh #ql);
                } else {
                    // 商が64bitに収まる
                    return (xh #xl) / yl;
                }
            } else {
                if (xh >= yh) {
                    // yh != 0なので、count_trailing_zerosを計算するbsr命令が使える
                    S = 63 ^ count_trailing_zeros(yh);
                    if (S != 0) {
                        // yの上から64bitをyhに集める
                        (yh #yl) = (yh #yl) << S;
                        // 商の上界値を求める
                        q = ((xh #xl) >> (64 - S)) / yh;
                        r = ((xh #xl) >> (64 - S)) / yh;
                        (mh #ml) = q * yl;
                        if (mh >= r) {
                            // xl << Sは、(xh#xl)>>(64-S)ではみ出した部分
                            if (q >= xl << S || mh != r) {
                                return q;
                            }
                        }
                        return q - 1;
                    } else {
                        // (yh#yl)が2の127乗以上の時
                        if (xh <= yh && xl < yl) {
                            // (xh#xl) < (yh#yl) の時（xh == yh && xl < yl）
                            return 0;
                        }
                        return 1;
                    }
                } else {
                    // (xh#xl) < (yh#yl) の時（xh < yh）
                    return 0;
                }
            }
            */
        }
    };
#endif

}  // namespace itype

}  // namespace gsh
