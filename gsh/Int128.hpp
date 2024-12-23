#pragma once
#include <compare>  // std::strong_ordering
#include "TypeDef.hpp"
#include "Util.hpp"
#include "internal/Operation.hpp"
#ifdef _MSC_VER
#include <intrin.h>
#include <immintrin.h>
#pragma intrinsic(_umul128, __umulh, _udiv128, __shiftleft128, __shiftright128)
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
            itype::u64 low = _umul128(muler, mulnd, &high);
            return { high, low };
        }
#endif
        itype::u64 u1 = (muler & 0xffffffff);
        itype::u64 v1 = (mulnd & 0xffffffff);
        itype::u64 t = (u1 * v1);
        itype::u64 w3 = (t & 0xffffffff);
        itype::u64 k = (t >> 32);
        muler >>= 32;
        t = (muler * v1) + k;
        k = (t & 0xffffffff);
        itype::u64 w1 = (t >> 32);
        mulnd >>= 32;
        t = (u1 * mulnd) + k;
        k = (t >> 32);
        return { (muler * mulnd) + w1 + k, (t << 32) + w3 };
#endif
    }
    GSH_INTERNAL_INLINE constexpr itype::u64 Mulu128High(itype::u64 muler, itype::u64 mulnd) noexcept {
#if defined(__SIZEOF_INT128__)
        return static_cast<itype::u64>((static_cast<__uint128_t>(muler) * mulnd) >> 64);
#else
#if defined(_MSC_VER)
        if (!std::is_constant_evaluated()) return __umulh(muler, mulnd);
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
            itype::u64 rem;
            itype::u64 res = _udiv128(high, low, div, &rem);
            return { res, rem };
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
    GSH_INTERNAL_INLINE constexpr std::pair<itype::u64, itype::u64> Divu128(itype::u64 high, itype::u64 low, itype::u64 dhigh, itype::u64 dlow) noexcept {
        if (dhigh == 0) {
            if (high >= dlow) {
                itype::u64 qh = high / dlow, r = high % dlow;
                itype::u64 ql = internal::Divu128(r, low, dlow).first;
                high = qh, low = ql;
            } else {
                low = internal::Divu128(high, low, dlow).first;
                high = 0;
            }
        } else if (high >= dhigh) {
            Assume(dhigh != 0);
            itype::i32 s = std::countl_zero(dhigh);
            if (s != 0) {
                itype::u64 yh = dhigh << s | dlow >> (64 - s), yl = dlow << s;
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
        return { high, low };
    }
    GSH_INTERNAL_INLINE constexpr std::pair<itype::u64, itype::u64> Modu128(itype::u64 high, itype::u64 low, itype::u64 dhigh, itype::u64 dlow) noexcept {
        if (dhigh == 0) {
            low = internal::Divu128(high % dlow, low, dlow).second;
            high = 0;
        } else if (high >= dhigh) {
            Assume(dhigh != 0);
            itype::i32 s = std::countl_zero(dhigh);
            if (s != 0) {
                itype::u64 yh = dhigh << s | dlow >> (64 - s), yl = dlow << s;
                auto [q, r] = internal::Divu128(high >> (64 - s), high << s | low >> (64 - s), yh);
                auto [mh, ml] = internal::Mulu128(q, yl);
                itype::u64 d = q - (mh >= r && (q >= (low << s) || mh != r));
                auto [dh, dl] = internal::Mulu128(d, dlow);
                high -= dh + d * dhigh;
                high -= low < dl;
                low -= dl;
            } else if (high > dhigh || low >= dlow) {
                high -= dhigh;
                high -= low < dlow;
                low -= dlow;
            }
        }
        return { high, low };
    }
    GSH_INTERNAL_INLINE constexpr itype::u64 ShiftLeft128High(itype::u64 high, itype::u64 low, itype::i32 shift) noexcept {
        Assume(0 <= shift && shift < 64);
#ifdef _MSC_VER
        if (!std::is_constant_evaluated()) return __shiftleft128(low, high, shift);
#endif
        return high << shift | (low >> 1 >> (63 - shift));
    }
    GSH_INTERNAL_INLINE constexpr itype::u64 ShiftRight128Low(itype::u64 high, itype::u64 low, itype::i32 shift) noexcept {
        Assume(0 <= shift && shift < 64);
#ifdef _MSC_VER
        if (!std::is_constant_evaluated()) return __shiftright128(low, high, shift);
#endif
        return low >> shift | (high << 1 << (63 - shift));
    }
}  // namespace internal

#if defined(__SIZEOF_INT128__)
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
    class alignas(16) i128;
    class alignas(16) u128 : protected internal::SwitchEndian128, public internal::ArithmeticInterface<u128> {
    public:
        constexpr u128() noexcept { high = 0, low = 0; }
        constexpr u128(const i128& n) noexcept;
        template<std::unsigned_integral T> constexpr u128(const T& n) noexcept { high = 0, low = n; }
        template<std::signed_integral T> constexpr u128(const T& n) noexcept {
            if (n < 0) {
                high = -1, low = ~(0 - n);
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
            high = low * n.high + high * n.low + hi;
            low = lw;
            return *this;
        }
        constexpr u128& operator/=(const u128& n) noexcept {
            auto [hi, lo] = internal::Divu128(high, low, n.high, n.low);
            high = hi, low = lo;
            return *this;
        }
        constexpr u128& operator%=(const u128& n) noexcept {
            auto [hi, lo] = internal::Modu128(high, low, n.high, n.low);
            high = hi, low = lo;
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
                high = internal::ShiftLeft128High(high, low, shift);
                low <<= shift;
            }
            return *this;
        }
        constexpr u128& operator>>=(itype::i32 shift) noexcept {
            if (shift >= 64) {
                low = high >> (shift - 64);
                high = 0;
            } else {
                low = internal::ShiftRight128Low(high, low, shift);
                high >>= shift;
            }
            return *this;
        }
        friend constexpr bool operator==(const u128& a, const u128& b) noexcept { return a.high == b.high && a.low == b.low; }
        friend constexpr std::strong_ordering operator<=>(const u128& a, const u128& b) noexcept {
            if (a.high < b.high || (a.high == b.high && a.low < b.low)) return std::strong_ordering::less;
            if (a.high == b.high && a.low == b.low) return std::strong_ordering::equal;
            if (a.high > b.high || (a.high == b.high && a.low > b.low)) return std::strong_ordering::greater;
            Unreachable();
        }
        constexpr operator bool() const noexcept { return low != 0 || high != 0; }
        template<std::integral T>
            requires(!std::same_as<T, i128>)
        constexpr operator T() const noexcept {
            return static_cast<T>(low);
        }
    };

    class i128 : private u128, public internal::ArithmeticInterface<i128> {
        friend class u128;
    public:
        constexpr i128() noexcept : u128() {}
        constexpr i128(const u128& n) noexcept : u128(n) {}
        template<std::integral T> constexpr i128(const T& n) noexcept : u128(n) {}
        constexpr i128(const i128&) noexcept = default;
        constexpr i128& operator=(const i128&) noexcept = default;
        constexpr i128 operator-() const noexcept { return u128::operator-(); }
        constexpr i128& operator+=(const i128& n) noexcept { return static_cast<i128&>(u128::operator+=(n)); }
        constexpr i128& operator-=(const i128& n) noexcept { return static_cast<i128&>(u128::operator-=(n)); }
        constexpr i128& operator*=(const i128& n) noexcept { return static_cast<i128&>(u128::operator*=(n)); }
        constexpr i128& operator/=(const i128&) noexcept {
            // TODO
            return *this;
        }
        constexpr i128& operator%=(const i128&) noexcept {
            // TODO
            return *this;
        }
        constexpr i128& operator++() noexcept { return static_cast<i128&>(u128::operator++()); }
        constexpr i128& operator--() noexcept { return static_cast<i128&>(u128::operator--()); }
        constexpr i128 operator~() const noexcept { return static_cast<i128>(u128::operator~()); }
        constexpr i128& operator&=(const i128& n) noexcept { return static_cast<i128&>(u128::operator&=(n)); }
        constexpr i128& operator|=(const i128& n) noexcept { return static_cast<i128&>(u128::operator|=(n)); }
        constexpr i128& operator^=(const i128& n) noexcept { return static_cast<i128&>(u128::operator^=(n)); }
        constexpr i128& operator<<=(itype::i32 shift) noexcept { return static_cast<i128&>(u128::operator<<=(shift)); }
        constexpr i128& operator>>=(itype::i32 shift) noexcept { return static_cast<i128&>(u128::operator>>=(shift)); }
        constexpr operator bool() const noexcept { return static_cast<bool>(static_cast<const u128&>(*this)); }
        template<std::integral T>
            requires(!std::same_as<T, u128>)
        constexpr operator T() const noexcept {
            return static_cast<T>(static_cast<const u128&>(*this));
        }
        friend constexpr bool operator==(const i128& a, const i128& b) noexcept { return a.high == b.high && a.low == b.low; }
        friend constexpr std::strong_ordering operator<=>(const i128& a, const i128& b) noexcept {
            constexpr itype::u64 mask = static_cast<itype::u64>(1) << 63;
            itype::u64 ahigh = a.high ^ mask, bhigh = b.high ^ mask;
            if (ahigh < bhigh || (ahigh == bhigh && a.low < b.low)) return std::strong_ordering::less;
            if (ahigh == bhigh && a.low == b.low) return std::strong_ordering::equal;
            if (ahigh > bhigh || (ahigh == bhigh && a.low > b.low)) return std::strong_ordering::greater;
            Unreachable();
        }
    };
    constexpr u128::u128(const i128& n) noexcept {
        high = n.high, low = n.low;
    }
}  // namespace itype
}

namespace std {
template<> struct common_type<gsh::itype::u128, gsh::itype::i128> {
    using type = gsh::itype::u128;
};
template<> struct common_type<gsh::itype::i128, gsh::itype::u128> {
    using type = gsh::itype::u128;
};
template<std::integral T> struct common_type<gsh::itype::u128, T> {
    using type = gsh::itype::u128;
};
template<std::integral T> struct common_type<T, gsh::itype::u128> {
    using type = gsh::itype::u128;
};
template<std::floating_point T> struct common_type<gsh::itype::u128, T> {
    using type = T;
};
template<std::floating_point T> struct common_type<T, gsh::itype::u128> {
    using type = T;
};
template<std::integral T> struct common_type<gsh::itype::i128, T> {
    using type = gsh::itype::i128;
};
template<std::integral T> struct common_type<T, gsh::itype::i128> {
    using type = gsh::itype::i128;
};
template<std::floating_point T> struct common_type<gsh::itype::i128, T> {
    using type = T;
};
template<std::floating_point T> struct common_type<T, gsh::itype::i128> {
    using type = T;
};
}  // namespace std

namespace gsh {

namespace internal {
    template<class T, class U> constexpr bool U128OrI128 = (std::same_as<T, itype::u128> || std::same_as<T, itype::i128> || std::same_as<U, itype::u128> || std::same_as<U, itype::i128>);
}

namespace itype {
    template<class T, class U>
        requires internal::U128OrI128<T, U>
    constexpr auto operator+(const T& a, const U& b) noexcept(noexcept(static_cast<std::common_type_t<T, U>>(a) + static_cast<std::common_type_t<T, U>>(b))) {
        return static_cast<std::common_type_t<T, U>>(a) + static_cast<std::common_type_t<T, U>>(b);
    }
    template<class T, class U>
        requires internal::U128OrI128<T, U>
    constexpr auto operator-(const T& a, const U& b) noexcept(noexcept(static_cast<std::common_type_t<T, U>>(a) - static_cast<std::common_type_t<T, U>>(b))) {
        return static_cast<std::common_type_t<T, U>>(a) - static_cast<std::common_type_t<T, U>>(b);
    }
    template<class T, class U>
        requires internal::U128OrI128<T, U>
    constexpr auto operator*(const T& a, const U& b) noexcept(noexcept(static_cast<std::common_type_t<T, U>>(a) * static_cast<std::common_type_t<T, U>>(b))) {
        return static_cast<std::common_type_t<T, U>>(a) * static_cast<std::common_type_t<T, U>>(b);
    }
    template<class T, class U>
        requires internal::U128OrI128<T, U>
    constexpr auto operator/(const T& a, const U& b) noexcept(noexcept(static_cast<std::common_type_t<T, U>>(a) / static_cast<std::common_type_t<T, U>>(b))) {
        return static_cast<std::common_type_t<T, U>>(a) / static_cast<std::common_type_t<T, U>>(b);
    }
    template<class T, class U>
        requires internal::U128OrI128<T, U>
    constexpr auto operator&(const T& a, const U& b) noexcept(noexcept(static_cast<std::common_type_t<T, U>>(a) & static_cast<std::common_type_t<T, U>>(b))) {
        return static_cast<std::common_type_t<T, U>>(a) & static_cast<std::common_type_t<T, U>>(b);
    }
    template<class T, class U>
        requires internal::U128OrI128<T, U>
    constexpr auto operator|(const T& a, const U& b) noexcept(noexcept(static_cast<std::common_type_t<T, U>>(a) | static_cast<std::common_type_t<T, U>>(b))) {
        return static_cast<std::common_type_t<T, U>>(a) | static_cast<std::common_type_t<T, U>>(b);
    }
    template<class T, class U>
        requires internal::U128OrI128<T, U>
    constexpr auto operator^(const T& a, const U& b) noexcept(noexcept(static_cast<std::common_type_t<T, U>>(a) ^ static_cast<std::common_type_t<T, U>>(b))) {
        return static_cast<std::common_type_t<T, U>>(a) ^ static_cast<std::common_type_t<T, U>>(b);
    }
    template<class T, class U>
        requires internal::U128OrI128<T, U>
    constexpr auto operator==(const T& a, const U& b) noexcept(noexcept(static_cast<std::common_type_t<T, U>>(a) == static_cast<std::common_type_t<T, U>>(b))) {
        return static_cast<std::common_type_t<T, U>>(a) == static_cast<std::common_type_t<T, U>>(b);
    }
    template<class T, class U>
        requires internal::U128OrI128<T, U>
    constexpr auto operator<=>(const T& a, const U& b) noexcept(noexcept(static_cast<std::common_type_t<T, U>>(a) <=> static_cast<std::common_type_t<T, U>>(b))) {
        return static_cast<std::common_type_t<T, U>>(a) <=> static_cast<std::common_type_t<T, U>>(b);
    }
}  // namespace itype
#endif

}  // namespace gsh
