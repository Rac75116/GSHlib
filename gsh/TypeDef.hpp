#pragma once

namespace gsh {

namespace itype {
    using i8 = signed char;
    using u8 = unsigned char;
    using i16 = short;
    using u16 = unsigned short;
    using i32 = int;
    using u32 = unsigned;
    using ilong = long;
    using ulong = unsigned long;
    using i64 = long long;
    using u64 = unsigned long long;
    using i128 = __int128_t;
    using u128 = __uint128_t;
    using isize = i32;
    using usize = u32;
    struct i4dig {
        i16 val;
        constexpr bool check() const noexcept { return val < 10000 && -val < 10000; }
    };
    struct u4dig {
        u16 val;
        constexpr bool check() const noexcept { return val < 10000; }
    };
    struct i8dig {
        i32 val;
        constexpr bool check() const noexcept { return val < 100000000 && -val < 100000000; }
    };
    struct u8dig {
        u32 val;
        constexpr bool check() const noexcept { return val < 100000000; }
    };
    struct i16dig {
        i64 val;
        constexpr bool check() const noexcept { return val < 10000000000000000 && -val < 10000000000000000; }
    };
    struct u16dig {
        u64 val;
        constexpr bool check() const noexcept { return val < 10000000000000000; }
    };
}  // namespace itype

namespace ftype {
    //using f16 = _Float16;
    using f32 = float;
    using f64 = double;
    using f128 = __float128;
    using flong = long double;
}  // namespace ftype

namespace ctype {
    using c8 = char;
    using wc = wchar_t;
    using utf8 = char8_t;
    using utf16 = char16_t;
    using utf32 = char32_t;
}  // namespace ctype

namespace simd {

    using i8x32 = __attribute__((vector_size(32))) itype::i8;
    using u8x32 = __attribute__((vector_size(32))) itype::u8;
    using i16x16 = __attribute__((vector_size(32))) itype::i16;
    using u16x16 = __attribute__((vector_size(32))) itype::u16;
    using i32x8 = __attribute__((vector_size(32))) itype::i32;
    using u32x8 = __attribute__((vector_size(32))) itype::u32;
    using i64x4 = __attribute__((vector_size(32))) itype::i64;
    using u64x4 = __attribute__((vector_size(32))) itype::u64;
    //using f16x16 = __attribute__((vector_size(32))) ftype::f16;
    using f32x8 = __attribute__((vector_size(32))) ftype::f32;
    using f64x4 = __attribute__((vector_size(32))) ftype::f64;

    using i8x64 = __attribute__((vector_size(64))) itype::i8;
    using u8x64 = __attribute__((vector_size(64))) itype::u8;
    using i16x32 = __attribute__((vector_size(64))) itype::i16;
    using u16x32 = __attribute__((vector_size(64))) itype::u16;
    using i32x16 = __attribute__((vector_size(64))) itype::i32;
    using u32x16 = __attribute__((vector_size(64))) itype::u32;
    using i64x8 = __attribute__((vector_size(64))) itype::i64;
    using u64x8 = __attribute__((vector_size(64))) itype::u64;
    //using f16x32 = __attribute__((vector_size(64))) ftype::f16;
    using f32x16 = __attribute__((vector_size(64))) ftype::f32;
    using f64x8 = __attribute__((vector_size(64))) ftype::f64;

    template<class T, class U> constexpr T VectorCast(U x) {
        return __builtin_convertvector(x, T);
    }

}  // namespace simd

namespace internal {
    template<class T, class U> constexpr bool IsSame = false;
    template<class T> constexpr bool IsSame<T, T> = true;
    template<class T, class U, class... V> constexpr bool IsSameAny = IsSame<T, U> || IsSameAny<T, V...>;
    template<class T, class U> constexpr bool IsSameAny<T, U> = IsSame<T, U>;
}  // namespace internal

namespace simd {
    template<class T> concept Is256BitVector = internal::IsSameAny<T, i8x32, i16x16, i32x8, i64x4, u8x32, u16x16, u32x8, u64x4, f32x8, f64x4>;
    template<class T> concept Is512BitVector = internal::IsSameAny<T, i8x64, i16x32, i32x16, i64x8, u8x64, u16x32, u32x16, u64x8, f32x16, f64x8>;
    template<class T> concept IsVector = Is256BitVector<T> || Is512BitVector<T>;
}  // namespace simd

class Byte {
    itype::u8 b = 0;
public:
    friend constexpr Byte operator&(Byte l, Byte r) noexcept { return Byte::from_integer(l.b & r.b); }
    constexpr Byte& operator&=(Byte r) noexcept {
        b &= r.b;
        return *this;
    }
    friend constexpr Byte operator|(Byte l, Byte r) noexcept { return Byte::from_integer(l.b | r.b); }
    constexpr Byte& operator|=(Byte r) noexcept {
        b |= r.b;
        return *this;
    }
    friend constexpr Byte operator^(Byte l, Byte r) noexcept { return Byte::from_integer(l.b ^ r.b); }
    constexpr Byte& operator^=(Byte r) noexcept {
        b ^= r.b;
        return *this;
    }
    template<class IntType> friend constexpr Byte operator<<(Byte l, IntType r) noexcept { return Byte::from_integer(l.b << r); }
    template<class IntType> constexpr Byte& operator<<=(IntType r) noexcept {
        b <<= r;
        return *this;
    }
    template<class IntType> friend constexpr Byte operator>>(Byte l, IntType r) noexcept { return Byte::from_integer(l.b >> r); }
    template<class IntType> constexpr Byte& operator>>=(IntType r) noexcept {
        b >>= r;
        return *this;
    }
    friend constexpr Byte operator~(Byte l) noexcept { return Byte::from_integer(~l.b); }
    template<class IntType> constexpr IntType to_integer() noexcept { return static_cast<IntType>(b); }
    template<class IntType> static constexpr Byte from_integer(IntType l) noexcept {
        Byte res;
        res.b = static_cast<itype::u8>(l);
        return res;
    }
};  // namespace class Byte

}  // namespace gsh