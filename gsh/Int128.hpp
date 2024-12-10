#pragma once
#include <compare>  // std::strong_ordering
#include "TypeDef.hpp"
#include "Util.hpp"
#include "internal/Operation.hpp"
#ifdef _MSC_VER
#include <intrin.h>
#include <immintrin.h>
#endif

namespace gsh {

namespace internal {
    GSH_INTERNAL_INLINE constexpr std::pair<itype::u64, itype::u64> Mulu128(itype::u64 muler, itype::u64 mulnd) noexcept {
#if defined(__SIZEOF_INT128__)
        __uint128_t tmp = static_cast<__uint128_t>(muler) * mulnd;
        return { tmp >> 64, tmp };
#else
#if defined(_MSC_VER)
        if (!std::is_constant_evaluated()) {
            itype::u64 high;
            itype::u64 low = _umul128(muler, mulnd, high);
            return { high, low };
        }
#endif
        itype::u64 ah = muler >> 32, al = muler & 0xffffffff;
        itype::u64 bh = mulnd >> 32, bl = mulnd & 0xffffffff;
        itype::u64 chh = ah * bh, cll = al * bl;
        itype::u64 clh = al * bh, chl = ah * bl;
        itype::u64 cm = clh + chl;
        chh += static_cast<itype::u64>(cm < clh) << 32;
        itype::u64 high = chh + (cm >> 32);
        itype::u64 low = cll + (cm << 32);
        high += (low < cll);
        return { high, low };
#endif
    }
    GSH_INTERNAL_INLINE constexpr itype::u64 Mulu128High(itype::u64 muler, itype::u64 mulnd) noexcept {
#if defined(__SIZEOF_INT128__)
        return static_cast<itype::u64>((static_cast<__uint128_t>(muler) * mulnd) >> 64);
#else
#if defined(_MSC_VER)
        if (!std::is_constant_evaluated()) return _umulh(muler, mulnd);
#endif
        return Mulu128(muler, mulnd).first;
#endif
    }
    GSH_INTERNAL_INLINE constexpr std::pair<itype::u64, itype::u64> Divu128(itype::u64 high, itype::u64 low, itype::u64 div) noexcept {
#if (defined(__GNUC__) || defined(__ICC)) && defined(__x86_64__)
        if constexpr (sizeof(void*) == 8) {
            if (!std::is_constant_evaluated()) {
                itype::u64 res, rem;
                __asm__("divq %[v]" : "=a"(res), "=d"(rem) : [v] "r"(div), "a"(low), "d"(high));
                return { res, rem };
            }
        }
#elif defined(_MSC_VER)
        if (!std::is_constant_evaluated()) {
            return _udiv128(high, low, div, &rem);
        }
#endif
#if defined(__SIZEOF_INT128__)
        __uint128_t n = (static_cast<__uint128_t>(high) << 64 | low);
        __uint128_t res = n / div;
        return { res, n - res * div };
#else
        itype::u64 res = 0;
        itype::u64 cur = high;
        for (itype::u64 i = 0; i != 64; ++i) {
            itype::u64 large = cur >> 63;
            cur = cur << 1 | (low >> 63);
            low <<= 1;
            large |= (cur >= div);
            res = res << 1 | large;
            cur -= div & (0 - large);
        }
        return { res, cur };
#endif
    }
}  // namespace internal

#if 0 && defined(__SIZEOF_INT128__)
namespace itype {
    using i128 = __int128_t;
    using u128 = __uint128_t;
}  // namespace itype
#else
namespace internal {
    struct LittleEndian128 {
        itype::u64 high, low;
    };
    struct BigEndian128 {
        itype::u64 low, high;
    };
    using SwitchEndian128 = std::conditional_t<std::endian::big != std::endian::native, internal::LittleEndian128, internal::BigEndian128>;
}  // namespace internal

namespace itype {
    class u128 : private internal::SwitchEndian128, public internal::ArithmeticInterface<u128> {
    public:
        constexpr u128() noexcept { high = 0, low = 0; }
        template<std::unsigned_integral T> constexpr u128(T n) noexcept { high = 0, low = n; }
        template<std::signed_integral T> constexpr u128(T n) noexcept {
            if (n < 0) {
                high = -1, low = ~-n;
                operator++();
            } else {
                high = 0, low = n;
            }
        }
        constexpr u128(const u128&) noexcept = default;
        constexpr u128& operator=(const u128&) noexcept = default;
        constexpr u128 operator-() const noexcept {
            u128 res = ~*this;
            ++res;
            return res;
        }
        constexpr u128& operator+=(const u128& n) noexcept {
            high += n.high;
            low += n.low;
            high += low < n.low;
            return *this;
        }
        constexpr u128& operator-=(const u128& n) noexcept {
            high -= n.high;
            high -= low < n.low;
            low -= n.low;
            return *this;
        }
        constexpr u128& operator*=(const u128& n) noexcept {
            auto [hi, lw] = internal::Mulu128(low, n.low);
            high = lw * n.high + high * n.low + hi;
            low = lw;
            return *this;
        }
        constexpr u128& operator/=(const u128& n) noexcept {
            if (n.high == 0) {
                if (high >= n.low) {
                    itype::u64 qh = high / n.low, r = high % n.low;
                    itype::u64 ql = internal::Divu128(r, low, n.low).first;
                    high = qh, low = ql;
                } else {
                    low = internal::Divu128(high, low, n.low).first;
                    high = 0;
                }
            } else {
                if (high >= n.high) {
                    Assume(n.high != 0);
                    itype::u64 s = 63 ^ std::countl_zero(n.high);
                    if (s != 0) {
                        itype::u64 yh = n.high << s | n.low >> (64 - s), yl = n.low << s;
                        auto [q, r] = internal::Divu128(high >> (64 - s), high << s | low >> (64 - s), yh);
                        auto [mh, ml] = internal::Mulu128(q, yl);
                        low = q - 1 + (mh >= r && (q >= (low << s) || mh != r));
                        high = 0;
                    } else {
                        low = 1 - (high <= n.high && low < n.low);
                        high = 0;
                    }
                } else {
                    low = 0;
                    high = 0;
                }
            }
            return *this;
        }
        constexpr u128& operator%=(const u128& n) noexcept {
            operator-=(*this / n * n);
            return *this;
        }
        constexpr u128& operator++() noexcept {
            ++low;
            high += (low == 0);
            return *this;
        }
        constexpr u128& operator--() noexcept {
            high -= (low == 0);
            --low;
            return *this;
        }
        constexpr u128 operator~() const noexcept {
            u128 res;
            res.high = ~high;
            res.low = ~low;
            return res;
        }
        constexpr u128& operator&=(const u128& n) noexcept {
            high &= n.high;
            low &= n.low;
            return *this;
        }
        constexpr u128& operator|=(const u128& n) noexcept {
            high |= n.high;
            low |= n.low;
            return *this;
        }
        constexpr u128& operator^=(const u128& n) noexcept {
            high ^= n.high;
            low ^= n.low;
            return *this;
        }
        constexpr u128& operator<<=(itype::i32 shift) noexcept {
            if (shift >= 64) {
                high = low << (shift - 64);
                low = 0;
            } else {
#ifdef _MSC_VER
                high = __shiftleft128(low, high, shift);
                low <<= shift;
#else
                high = high << shift | (low >> 1 >> (63 - shift));
                low <<= shift;
#endif
            }
            return *this;
        }
        constexpr u128& operator>>=(itype::i32 shift) noexcept {
            if (shift >= 64) {
                low = high >> (shift - 64);
                high = 0;
            } else {
#ifdef _MSC_VER
                low = __shiftright128(low, high, shift);
                high >>= shift;
#else
                low = low >> shift | (high << 1 << (63 - shift));
                high >>= shift;
#endif
            }
            return *this;
        }
        constexpr operator bool() const noexcept { return low != 0 || high != 0; }
        friend constexpr bool operator==(const u128& a, const u128& b) noexcept { return a.high == b.high && a.low == b.low; }
        friend constexpr std::strong_ordering operator<=>(const u128& a, const u128& b) noexcept {
            if (a.high < b.high || (a.high == b.high && a.low < b.low)) return std::strong_ordering::less;
            if (a.high == b.high && a.low == b.low) return std::strong_ordering::equal;
            if (a.high > b.high || (a.high == b.high && a.low > b.low)) return std::strong_ordering::greater;
            Unreachable();
        }
        template<std::integral T> constexpr operator T() const noexcept { return static_cast<T>(low); }
    };
    using i128 = __int128_t;
}  // namespace itype
#endif

}  // namespace gsh
