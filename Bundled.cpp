#if !defined(__clang__) && defined(__GNUC__)
#pragma GCC optimize("Ofast")
#pragma GCC optimize("unroll-loops")
#pragma GCC target("avx2")
#endif
#ifdef EVAL
#define ONLINE_JUDGE
#endif
#ifdef ONLINE_JUDGE
#define NDEBUG
#endif
#include <bit>      
#include <cstdlib>  
#include <cstring>  
#if __has_include(<unistd.h>)
#include <unistd.h>  
#endif
#ifndef _WIN32
#include <sys/mman.h>  
#include <sys/stat.h>  
#endif

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
#ifdef __SIZEOF_INT128__
    using i128 = __int128_t;
    using u128 = __uint128_t;
#endif
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
}  

namespace ftype {
    
    using f32 = float;
    using f64 = double;
#ifdef __SIZEOF_FLOAT128__
    using f128 = __float128;
#endif
    using flong = long double;
}  

namespace ctype {
    using c8 = char;
    using wc = wchar_t;
    using utf8 = char8_t;
    using utf16 = char16_t;
    using utf32 = char32_t;
}  

namespace internal {
    template<class T, class U> constexpr bool IsSame = false;
    template<class T> constexpr bool IsSame<T, T> = true;
    template<class T, class U, class... V> constexpr bool IsSameAny = IsSame<T, U> || IsSameAny<T, V...>;
    template<class T, class U> constexpr bool IsSameAny<T, U> = IsSame<T, U>;
}  

namespace simd {

#if defined __GNUC__
    using i8x32 = __attribute__((vector_size(32))) itype::i8;
    using u8x32 = __attribute__((vector_size(32))) itype::u8;
    using i16x16 = __attribute__((vector_size(32))) itype::i16;
    using u16x16 = __attribute__((vector_size(32))) itype::u16;
    using i32x8 = __attribute__((vector_size(32))) itype::i32;
    using u32x8 = __attribute__((vector_size(32))) itype::u32;
    using i64x4 = __attribute__((vector_size(32))) itype::i64;
    using u64x4 = __attribute__((vector_size(32))) itype::u64;
    
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
    
    using f32x16 = __attribute__((vector_size(64))) ftype::f32;
    using f64x8 = __attribute__((vector_size(64))) ftype::f64;

    template<class T, class U> constexpr T VectorCast(U x) {
        return __builtin_convertvector(x, T);
    }

    template<class T> concept Is256BitVector = internal::IsSameAny<T, i8x32, i16x16, i32x8, i64x4, u8x32, u16x16, u32x8, u64x4, f32x8, f64x4>;
    template<class T> concept Is512BitVector = internal::IsSameAny<T, i8x64, i16x32, i32x16, i64x8, u8x64, u16x32, u32x16, u64x8, f32x16, f64x8>;
    template<class T> concept IsVector = Is256BitVector<T> || Is512BitVector<T>;
#endif

}  

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
};  

}  


#define GSH_INTERNAL_STR(s) #s
#if defined __clang__ || defined __INTEL_COMPILER
#define GSH_INTERNAL_UNROLL(n) _Pragma(GSH_INTERNAL_STR(unroll n))
#elif defined __GNUC__
#define GSH_INTERNAL_UNROLL(n) _Pragma(GSH_INTERNAL_STR(GCC unroll n))
#else
#define GSH_INTERNAL_UNROLL(n)
#endif
#ifdef __GNUC__
#define GSH_INTERNAL_INLINE [[gnu::always_inline]]
#elif _MSC_VER
#define GSH_INTERNAL_INLINE [[msvc::forceinline]]
#else
#define GSH_INTERNAL_INLINE inline
#endif
#if defined __clang__
#define GSH_INTERNAL_ASSUME(...) [&]() { __builtin_assume(bool(__VA_ARGS__)); }()
#elif defined __GNUC__
#define GSH_INTERNAL_ASSUME(...) [&]() { if (!(__VA_ARGS__)) __builtin_unreachable(); }()
#elif _MSC_VER
#define GSH_INTERNAL_ASSUME(...) [&]() { __assume(bool(__VA_ARGS__)); }()
#else
namespace gsh { namespace internal { [[noreturn]] inline void unreachable() noexcept {} } }
#define GSH_INTERNAL_ASSUME(...) [&]() { if(!(__VA_ARGS__)) gsh::internal::unreachable(); }()
#endif


namespace gsh {

template<class T> class Parser;

namespace internal {
    template<class Stream> constexpr itype::u8 Parseu8(Stream& stream) {
        itype::u32 v;
        std::memcpy(&v, stream.current(), 4);
        v ^= 0x30303030;
        itype::i32 tmp = std::countr_zero(v & 0xf0f0f0f0) >> 3;
        v <<= (32 - (tmp << 3));
        stream.skip(tmp + 1);
        v = (v * 10 + (v >> 8)) & 0x00ff00ff;
        v = (v * 100 + (v >> 16)) & 0x0000ffff;
        return v;
    }
    template<class Stream> constexpr itype::u16 Parseu16(Stream& stream) {
        itype::u64 v;
        std::memcpy(&v, stream.current(), 8);
        v ^= 0x3030303030303030;
        itype::i32 tmp = std::countr_zero(v & 0xf0f0f0f0f0f0f0f0) >> 3;
        v <<= (64 - (tmp << 3));
        stream.skip(tmp + 1);
        v = (v * 10 + (v >> 8)) & 0x00ff00ff00ff00ff;
        v = (v * 100 + (v >> 16)) & 0x0000ffff0000ffff;
        v = (v * 10000 + (v >> 32)) & 0x00000000ffffffff;
        return v;
    }
    template<class Stream> constexpr itype::u32 Parseu32(Stream& stream) {
        itype::u32 res = 0;
        {
            itype::u64 v;
            std::memcpy(&v, stream.current(), 8);
            if (!((v ^= 0x3030303030303030) & 0xf0f0f0f0f0f0f0f0)) {
                v = (v * 10 + (v >> 8)) & 0x00ff00ff00ff00ff;
                v = (v * 100 + (v >> 16)) & 0x0000ffff0000ffff;
                v = (v * 10000 + (v >> 32)) & 0x00000000ffffffff;
                res = v;
                stream.skip(8);
            }
        }
        itype::u64 buf;
        std::memcpy(&buf, stream.current(), 8);
        {
            itype::u32 v = buf;
            if (!((v ^= 0x30303030) & 0xf0f0f0f0)) {
                buf >>= 32;
                v = (v * 10 + (v >> 8)) & 0x00ff00ff;
                v = (v * 100 + (v >> 16)) & 0x0000ffff;
                res = 10000 * res + v;
                stream.skip(4);
            }
        }
        {
            itype::u16 v = buf;
            if (!((v ^= 0x3030) & 0xf0f0)) {
                buf >>= 16;
                v = (v * 10 + (v >> 8)) & 0x00ff;
                res = 100 * res + v;
                stream.skip(2);
            }
        }
        {
            const ctype::c8 v = ctype::c8(buf) ^ 0x30;
            const bool f = !(v & 0xf0);
            res = f ? 10 * res + v : res;
            stream.skip(f + 1);
        }
        return res;
    };
    template<class Stream> constexpr itype::u64 Parseu64(Stream& stream) {
        itype::u64 res = 0;
        {
            itype::u64 v;
            std::memcpy(&v, stream.current(), 8);
            if (!((v ^= 0x3030303030303030) & 0xf0f0f0f0f0f0f0f0)) {
                stream.skip(8);
                itype::u64 u;
                std::memcpy(&u, stream.current(), 8);
                if (!((u ^= 0x3030303030303030) & 0xf0f0f0f0f0f0f0f0)) {
                    v = (v * 10 + (v >> 8)) & 0x00ff00ff00ff00ff;
                    u = (u * 10 + (u >> 8)) & 0x00ff00ff00ff00ff;
                    v = (v * 100 + (v >> 16)) & 0x0000ffff0000ffff;
                    u = (u * 100 + (u >> 16)) & 0x0000ffff0000ffff;
                    v = (v * 10000 + (v >> 32)) & 0x00000000ffffffff;
                    u = (u * 10000 + (u >> 32)) & 0x00000000ffffffff;
                    res = v * 100000000 + u;
                    stream.skip(8);
                } else {
                    v = (v * 10 + (v >> 8)) & 0x00ff00ff00ff00ff;
                    v = (v * 100 + (v >> 16)) & 0x0000ffff0000ffff;
                    v = (v * 10000 + (v >> 32)) & 0x00000000ffffffff;
                    res = v;
                }
            }
        }
        itype::u64 buf;
        std::memcpy(&buf, stream.current(), 8);
        {
            itype::u32 v = buf;
            if (!((v ^= 0x30303030) & 0xf0f0f0f0)) {
                buf >>= 32;
                v = (v * 10 + (v >> 8)) & 0x00ff00ff;
                v = (v * 100 + (v >> 16)) & 0x0000ffff;
                res = 10000 * res + v;
                stream.skip(4);
            }
        }
        {
            itype::u16 v = buf;
            if (!((v ^= 0x3030) & 0xf0f0)) {
                buf >>= 16;
                v = (v * 10 + (v >> 8)) & 0x00ff;
                res = 100 * res + v;
                stream.skip(2);
            }
        }
        {
            const ctype::c8 v = ctype::c8(buf) ^ 0x30;
            const bool f = !(v & 0xf0);
            res = f ? 10 * res + v : res;
            stream.skip(f + 1);
        }
        return res;
    }
    template<class Stream> constexpr itype::u128 Parseu128(Stream& stream) {
        itype::u128 res = 0;
        while (true) {
            itype::u64 v;
            std::memcpy(&v, stream.current(), 8);
            if (((v ^= 0x3030303030303030) & 0xf0f0f0f0f0f0f0f0) != 0) break;
            v = (v * 10 + (v >> 8)) & 0x00ff00ff00ff00ff;
            v = (v * 100 + (v >> 16)) & 0x0000ffff0000ffff;
            v = (v * 10000 + (v >> 32)) & 0x00000000ffffffff;
            res = res * 100000000 + v;
            stream.skip(8);
        }
        itype::u64 buf;
        std::memcpy(&buf, stream.current(), 8);
        {
            itype::u32 v = buf;
            if (!((v ^= 0x30303030) & 0xf0f0f0f0)) {
                buf >>= 32;
                v = (v * 10 + (v >> 8)) & 0x00ff00ff;
                v = (v * 100 + (v >> 16)) & 0x0000ffff;
                res = 10000 * res + v;
                stream.skip(4);
            }
        }
        {
            itype::u16 v = buf;
            if (!((v ^= 0x3030) & 0xf0f0)) {
                buf >>= 16;
                v = (v * 10 + (v >> 8)) & 0x00ff;
                res = 100 * res + v;
                stream.skip(2);
            }
        }
        {
            const ctype::c8 v = ctype::c8(buf) ^ 0x30;
            const bool f = !(v & 0xf0);
            res = f ? 10 * res + v : res;
            stream.skip(f + 1);
        }
        return res;
    }
    template<class Stream> constexpr itype::u8dig Parseu8dig(Stream& stream) {
        itype::u64 v;
        std::memcpy(&v, stream.current(), 8);
        v ^= 0x3030303030303030;
        itype::i32 tmp = std::countr_zero(v & 0xf0f0f0f0f0f0f0f0) >> 3;
        v <<= (64 - (tmp << 3));
        stream.skip(tmp + 1);
        v = (v * 10 + (v >> 8)) & 0x00ff00ff00ff00ff;
        v = (v * 100 + (v >> 16)) & 0x0000ffff0000ffff;
        v = (v * 10000 + (v >> 32)) & 0x00000000ffffffff;
        return itype::u8dig{ static_cast<itype::u32>(v) };
    }
}  

template<> class Parser<itype::u8> {
public:
    template<class Stream> constexpr itype::u8 operator()(Stream& stream) const {
        stream.reload(8);
        return internal::Parseu8(stream);
    }
};
template<> class Parser<itype::i8> {
public:
    template<class Stream> constexpr itype::i8 operator()(Stream& stream) const {
        stream.reload(8);
        bool neg = *stream.current() == '-';
        if (neg) stream.skip(1);
        itype::i8 tmp = internal::Parseu8(stream);
        if (neg) tmp = -tmp;
        return tmp;
    }
};
template<> class Parser<itype::u16> {
public:
    template<class Stream> constexpr itype::u16 operator()(Stream& stream) const {
        stream.reload(8);
        return internal::Parseu16(stream);
    }
};
template<> class Parser<itype::i16> {
public:
    template<class Stream> constexpr itype::i16 operator()(Stream& stream) const {
        stream.reload(8);
        bool neg = *stream.current() == '-';
        if (neg) stream.skip(1);
        itype::i16 tmp = internal::Parseu16(stream);
        if (neg) tmp = -tmp;
        return tmp;
    }
};
template<> class Parser<itype::u32> {
public:
    template<class Stream> constexpr itype::u32 operator()(Stream& stream) const {
        stream.reload(16);
        return internal::Parseu32(stream);
    }
};
template<> class Parser<itype::i32> {
public:
    template<class Stream> constexpr itype::i32 operator()(Stream& stream) const {
        stream.reload(16);
        bool neg = *stream.current() == '-';
        if (neg) stream.skip(1);
        itype::i32 tmp = internal::Parseu32(stream);
        if (neg) tmp = -tmp;
        return tmp;
    }
};
template<> class Parser<itype::u64> {
public:
    template<class Stream> constexpr itype::u64 operator()(Stream& stream) const {
        stream.reload(32);
        return internal::Parseu64(stream);
    }
};
template<> class Parser<itype::i64> {
public:
    template<class Stream> constexpr itype::i64 operator()(Stream& stream) const {
        stream.reload(32);
        bool neg = *stream.current() == '-';
        if (neg) stream.skip(1);
        itype::i64 tmp = internal::Parseu64(stream);
        if (neg) tmp = -tmp;
        return tmp;
    }
};
template<> class Parser<itype::u128> {
public:
    template<class Stream> constexpr itype::u128 operator()(Stream& stream) const {
        stream.reload(64);
        return internal::Parseu128(stream);
    }
};
template<> class Parser<itype::i128> {
public:
    template<class Stream> constexpr itype::i128 operator()(Stream& stream) const {
        stream.reload(64);
        bool neg = *stream.current() == '-';
        if (neg) stream.skip(1);
        itype::i128 tmp = internal::Parseu128(stream);
        if (neg) tmp = -tmp;
        return tmp;
    }
};
template<> class Parser<itype::u8dig> {
public:
    template<class Stream> constexpr itype::u8dig operator()(Stream& stream) const {
        stream.reload(16);
        return internal::Parseu8dig(stream);
    }
};
template<> class Parser<itype::i8dig> {
public:
    template<class Stream> constexpr itype::i8dig operator()(Stream& stream) const {
        stream.reload(16);
        bool neg = *stream.current() == '-';
        if (neg) stream.skip(1);
        itype::i32 tmp = internal::Parseu8dig(stream).val;
        if (neg) tmp = -tmp;
        return itype::i8dig{ tmp };
    }
};
template<> class Parser<ctype::c8> {
public:
    template<class Stream> constexpr ctype::c8 operator()(Stream& stream) const {
        stream.reload(2);
        ctype::c8 tmp = *stream.current();
        stream.skip(2);
        return tmp;
    }
};



template<class T> class Formatter;

namespace internal {
    template<itype::u32> constexpr auto InttoStr = [] {
        struct {
            ctype::c8 table[40004] = {};
        } res;
        for (itype::u32 i = 0; i != 10000; ++i) {
            res.table[4 * i + 0] = (i / 1000 + '0');
            res.table[4 * i + 1] = (i / 100 % 10 + '0');
            res.table[4 * i + 2] = (i / 10 % 10 + '0');
            res.table[4 * i + 3] = (i % 10 + '0');
        }
        return res;
    }();
    template<class Stream> constexpr void Formatu16(Stream& stream, itype::u16 n) {
        auto copy1 = [&](itype::u32 x) {
            itype::u32 off = (x < 10) + (x < 100) + (x < 1000);
            std::memcpy(stream.current(), InttoStr<0>.table + (4 * x + off), 4);
            stream.skip(4 - off);
        };
        auto copy2 = [&](itype::u32 x) {
            std::memcpy(stream.current(), InttoStr<0>.table + 4 * x, 4);
            stream.skip(4);
        };
        if (n < 10000) copy1(n);
        else {
            copy1(n / 10000);
            copy2(n % 10000);
        }
    }
    template<class Stream> constexpr void Formatu32(Stream& stream, itype::u32 n) {
        auto copy1 = [&](itype::u32 x) {
            itype::u32 off = (x < 10) + (x < 100) + (x < 1000);
            std::memcpy(stream.current(), InttoStr<0>.table + (4 * x + off), 4);
            stream.skip(4 - off);
        };
        auto copy2 = [&](itype::u32 x) {
            std::memcpy(stream.current(), InttoStr<0>.table + 4 * x, 4);
            stream.skip(4);
        };
        if (n < 100000000) {
            if (n < 10000) copy1(n);
            else {
                copy1(n / 10000);
                copy2(n % 10000);
            }
        } else {
            copy1(n / 100000000);
            copy2(n / 10000 % 10000);
            copy2(n % 10000);
        }
    }
    template<class Stream> constexpr void Formatu64(Stream& stream, itype::u64 n) {
        auto copy1 = [&](itype::u32 x) {
            itype::u32 off = (x < 10) + (x < 100) + (x < 1000);
            std::memcpy(stream.current(), InttoStr<0>.table + (4 * x + off), 4);
            stream.skip(4 - off);
        };
        auto copy2 = [&](itype::u32 x) {
            std::memcpy(stream.current(), InttoStr<0>.table + 4 * x, 4);
            stream.skip(4);
        };
        if (n < 10000000000000000) {
            if (n < 1000000000000) {
                if (n < 100000000) {
                    if (n < 10000) copy1(n);
                    else {
                        copy1(n / 10000);
                        copy2(n % 10000);
                    }
                } else {
                    copy1(n / 100000000);
                    copy2(n / 10000 % 10000);
                    copy2(n % 10000);
                }
            } else {
                copy1(n / 1000000000000);
                copy2(n / 100000000 % 10000);
                copy2(n / 10000 % 10000);
                copy2(n % 10000);
            }
        } else {
            copy1(n / 10000000000000000);
            copy2(n / 1000000000000 % 10000);
            copy2(n / 100000000 % 10000);
            copy2(n / 10000 % 10000);
            copy2(n % 10000);
        }
    }
    template<class Stream> constexpr void Formatu128(Stream& stream, itype::u128 n) {
        auto copy1 = [&](itype::u32 x) {
            itype::u32 off = (x < 10) + (x < 100) + (x < 1000);
            std::memcpy(stream.current(), InttoStr<0>.table + (4 * x + off), 4);
            stream.skip(4 - off);
        };
        auto copy2 = [&](itype::u32 x) {
            std::memcpy(stream.current(), InttoStr<0>.table + 4 * x, 4);
            stream.skip(4);
        };
        constexpr itype::u128 t = static_cast<itype::u128>(10000000000000000) * 10000000000000000;
        if (n >= t) {
            const itype::u32 r = n / t;
            n %= t;
            if (r >= 10000) {
                copy1(r / 10000);
                copy2(r % 10000);
            } else copy1(r / 10000);
            const itype::u64 a = n / 10000000000000000, b = n % 10000000000000000;
            const itype::u32 c = a / 100000000, d = a % 100000000, e = b / 100000000, f = b % 100000000;
            copy2(c / 10000), copy2(c % 10000);
            copy2(d / 10000), copy2(d % 10000);
            copy2(e / 10000), copy2(e % 10000);
            copy2(f / 10000), copy2(f % 10000);
        } else {
            const itype::u64 a = n / 10000000000000000, b = n % 10000000000000000;
            const itype::u32 c = a / 100000000, d = a % 100000000, e = b / 100000000, f = b % 100000000;
            const itype::u32 g = c / 10000, h = c % 10000, i = d / 10000, j = d % 10000, k = e / 10000, l = e % 10000, m = f / 10000, n = f % 10000;
            if (a == 0) {
                if (e == 0) {
                    if (m == 0) copy1(n);
                    else copy1(m), copy2(n);
                } else {
                    if (k == 0) copy1(l), copy2(m), copy2(n);
                    else copy1(k), copy2(l), copy2(m), copy2(n);
                }
            } else {
                if (c == 0) {
                    if (i == 0) copy1(j), copy2(k), copy2(l), copy2(m), copy2(n);
                    else copy1(i), copy2(j), copy2(k), copy2(l), copy2(m), copy2(n);
                } else {
                    if (g == 0) copy1(h), copy2(i), copy2(j), copy2(k), copy2(l), copy2(m), copy2(n);
                    else copy1(g), copy2(h), copy2(i), copy2(j), copy2(k), copy2(l), copy2(m), copy2(n);
                }
            }
        }
    }
    template<class Stream> constexpr void Formatu8dig(Stream& stream, itype::u8dig x) {
        const itype::u64 n = x.val;
        auto copy1 = [&](itype::u64 x) {
            itype::u32 off = (x < 10) + (x < 100) + (x < 1000);
            std::memcpy(stream.current(), InttoStr<0>.table + (4 * x + off), 4);
            stream.skip(4 - off);
        };
        auto copy2 = [&](itype::u64 x) {
            std::memcpy(stream.current(), InttoStr<0>.table + 4 * x, 4);
            stream.skip(4);
        };
        if (n < 10000) copy1(n);
        else {
            copy1(n / 10000);
            copy2(n % 10000);
        }
    }
    template<class Stream> constexpr void Formatu16dig(Stream& stream, itype::u16dig x) {
        const itype::u64 n = x.val;
        auto copy1 = [&](itype::u64 x) {
            itype::u32 off = (x < 10) + (x < 100) + (x < 1000);
            std::memcpy(stream.current(), InttoStr<0>.table + (4 * x + off), 4);
            stream.skip(4 - off);
        };
        auto copy2 = [&](itype::u64 x) {
            std::memcpy(stream.current(), InttoStr<0>.table + 4 * x, 4);
            stream.skip(4);
        };
        if (n < 1000000000000) {
            if (n < 100000000) {
                if (n < 10000) copy1(n);
                else {
                    copy1(n / 10000);
                    copy2(n % 10000);
                }
            } else {
                copy1(n / 100000000);
                copy2(n / 10000 % 10000);
                copy2(n % 10000);
            }
        } else {
            copy1(n / 1000000000000);
            copy2(n / 100000000 % 10000);
            copy2(n / 10000 % 10000);
            copy2(n % 10000);
        }
    }
}  

template<> class Formatter<itype::u16> {
public:
    template<class Stream> constexpr void operator()(Stream& stream, itype::u16 n) const {
        stream.reload(8);
        internal::Formatu16(stream, n);
    }
};
template<> class Formatter<itype::i16> {
public:
    template<class Stream> constexpr void operator()(Stream& stream, itype::i16 n) const {
        stream.reload(8);
        *stream.current() = '-';
        stream.skip(n < 0);
        internal::Formatu16(stream, n < 0 ? -n : n);
    }
};
template<> class Formatter<itype::u32> {
public:
    template<class Stream> constexpr void operator()(Stream& stream, itype::u32 n) const {
        stream.reload(16);
        internal::Formatu32(stream, n);
    }
};
template<> class Formatter<itype::i32> {
public:
    template<class Stream> constexpr void operator()(Stream& stream, itype::i32 n) const {
        stream.reload(16);
        *stream.current() = '-';
        stream.skip(n < 0);
        internal::Formatu32(stream, n < 0 ? -n : n);
    }
};
template<> class Formatter<itype::u64> {
public:
    template<class Stream> constexpr void operator()(Stream& stream, itype::u64 n) const {
        stream.reload(32);
        internal::Formatu64(stream, n);
    }
};
template<> class Formatter<itype::i64> {
public:
    template<class Stream> constexpr void operator()(Stream& stream, itype::i64 n) const {
        stream.reload(32);
        *stream.current() = '-';
        stream.skip(n < 0);
        internal::Formatu64(stream, n < 0 ? -n : n);
    }
};
template<> class Formatter<itype::u128> {
public:
    template<class Stream> constexpr void operator()(Stream& stream, itype::u128 n) const {
        stream.reload(64);
        internal::Formatu128(stream, n);
    }
};
template<> class Formatter<itype::i128> {
public:
    template<class Stream> constexpr void operator()(Stream& stream, itype::i128 n) const {
        stream.reload(64);
        *stream.current() = '-';
        stream.skip(n < 0);
        internal::Formatu128(stream, n < 0 ? -n : n);
    }
};
template<> class Formatter<itype::u8dig> {
public:
    template<class Stream> constexpr void operator()(Stream& stream, itype::u8dig n) const {
        stream.reload(8);
        internal::Formatu8dig(stream, n);
    }
};
template<> class Formatter<itype::i8dig> {
public:
    template<class Stream> constexpr void operator()(Stream& stream, itype::i8dig n) const {
        stream.reload(9);
        *stream.current() = '-';
        stream.skip(n.val < 0);
        internal::Formatu8dig(stream, itype::u8dig{ static_cast<itype::u32>(n.val < 0 ? -n.val : n.val) });
    }
};
template<> class Formatter<itype::u16dig> {
public:
    template<class Stream> constexpr void operator()(Stream& stream, itype::u16dig n) const {
        stream.reload(16);
        internal::Formatu16dig(stream, n);
    }
};
template<> class Formatter<itype::i16dig> {
public:
    template<class Stream> constexpr void operator()(Stream& stream, itype::i16dig n) const {
        stream.reload(17);
        *stream.current() = '-';
        stream.skip(n.val < 0);
        internal::Formatu16dig(stream, itype::u16dig{ static_cast<itype::u64>(n.val < 0 ? -n.val : n.val) });
    }
};
template<> class Formatter<ctype::c8> {
public:
    template<class Stream> constexpr void operator()(Stream& stream, ctype::c8 c) const {
        stream.reload(1);
        *stream.current() = c;
        stream.skip(1);
    }
};
template<> class Formatter<const ctype::c8*> {
public:
    template<class Stream> constexpr void operator()(Stream& stream, const ctype::c8* s) const {
        itype::u32 len = std::strlen(s);
        itype::u32 avail = stream.avail();
        if (avail >= len) [[likely]] {
            std::memcpy(stream.current(), s, len);
            stream.skip(len);
        } else {
            std::memcpy(stream.current(), s, avail);
            len -= avail;
            s += avail;
            stream.skip(avail);
            while (len != 0) {
                stream.reload();
                avail = stream.avail();
                const itype::u32 tmp = len < avail ? len : avail;
                std::memcpy(stream.current(), s, tmp);
                len -= tmp;
                s += tmp;
                stream.skip(tmp);
            }
        }
    }
};

class BasicReader {
    constexpr static itype::u32 Bufsize = 1 << 18;
    itype::i32 fd = 0;
    ctype::c8 buf[Bufsize + 1];
    ctype::c8 *cur = buf, *eof = buf;
public:
    BasicReader() {}
    BasicReader(itype::i32 filehandle) : fd(filehandle) {}
    BasicReader(const BasicReader& rhs) {
        fd = rhs.fd;
        std::memcpy(buf, rhs.buf, rhs.eof - rhs.cur);
        cur = buf + (rhs.cur - rhs.buf);
        eof = buf + (rhs.cur - rhs.eof);
    }
    BasicReader& operator=(const BasicReader& rhs) {
        fd = rhs.fd;
        std::memcpy(buf, rhs.buf, rhs.eof - rhs.cur);
        cur = buf + (rhs.cur - rhs.buf);
        eof = buf + (rhs.cur - rhs.eof);
        return *this;
    }
    void reload() {
        if (eof == buf + Bufsize || eof == cur || [&] {
                auto p = cur;
                while (*p >= '!') ++p;
                return p;
            }() == eof) [[likely]] {
            itype::u32 rem = eof - cur;
            std::memmove(buf, cur, rem);
            *(eof = buf + rem + read(fd, buf + rem, Bufsize - rem)) = '\0';
            cur = buf;
        }
    }
    void reload(itype::u32 len) {
        if (avail() < len) [[unlikely]]
            reload();
    }
    itype::u32 avail() const { return eof - cur; }
    const ctype::c8* current() const { return cur; }
    void skip(itype::u32 n) { cur += n; }
};
class MmapReader {
    const itype::i32 fh;
    ctype::c8* buf;
    ctype::c8 *cur, *eof;
public:
    MmapReader() : fh(0) {
#ifdef _WIN32
        write(1, "gsh::MmapReader / gsh::MmapReader is not available for Windows.\n", 64);
        std::exit(1);
#else
        struct stat st;
        fstat(0, &st);
        buf = reinterpret_cast<ctype::c8*>(mmap(nullptr, st.st_size + 64, PROT_READ, MAP_PRIVATE, 0, 0));
        cur = buf;
        eof = buf + st.st_size;
#endif
    }
    void reload() const {}
    void reload(itype::u32) const {}
    itype::u32 avail() const { return eof - cur; }
    const ctype::c8* current() const { return cur; }
    void skip(itype::u32 n) { cur += n; }
};
class StaticStrReader {
    const ctype::c8* cur;
public:
    constexpr StaticStrReader() {}
    constexpr StaticStrReader(const ctype::c8* c) : cur(c) {}
    constexpr void reload() const {}
    constexpr void reload(itype::u32) const {}
    constexpr itype::u32 avail() const { return static_cast<itype::u32>(-1); }
    constexpr const ctype::c8* current() { return cur; }
    constexpr void skip(itype::u32 n) { cur += n; }
};

class BasicWriter {
    constexpr static itype::u32 Bufsize = 1 << 18;
    itype::i32 fd = 1;
    ctype::c8 buf[Bufsize];
    ctype::c8 *cur = buf, *eof = buf + Bufsize;
public:
    BasicWriter() {}
    BasicWriter(itype::i32 filehandle) : fd(filehandle) {}
    BasicWriter(const BasicWriter& rhs) {
        fd = rhs.fd;
        std::memcpy(buf, rhs.buf, rhs.cur - rhs.buf);
        cur = buf + (rhs.cur - rhs.buf);
    }
    BasicWriter& operator=(const BasicWriter& rhs) {
        fd = rhs.fd;
        std::memcpy(buf, rhs.buf, rhs.cur - rhs.buf);
        cur = buf + (rhs.cur - rhs.buf);
        return *this;
    }
    void reload() {
        [[maybe_unused]] itype::i32 tmp = write(fd, buf, cur - buf);
        cur = buf;
    }
    void reload(itype::u32 len) {
        if (eof - cur < len) [[unlikely]]
            reload();
    }
    itype::u32 avail() const { return eof - cur; }
    ctype::c8* current() { return cur; }
    void skip(itype::u32 n) { cur += n; }
};
class StaticStrWriter {
    ctype::c8* cur;
public:
    constexpr StaticStrWriter() {}
    constexpr StaticStrWriter(ctype::c8* c) : cur(c) {}
    constexpr void reload() const {}
    constexpr void reload(itype::u32) const {}
    constexpr itype::u32 avail() const { return static_cast<itype::u32>(-1); }
    constexpr ctype::c8* current() { return cur; }
    constexpr void skip(itype::u32 n) { cur += n; }
};

}  

namespace gsh {

class Exception {
    char str[512];
    char* cur = str;
    void write(const char* x) {
        for (int i = 0; i != 512; ++i, ++cur) {
            if (x[i] == '\0') break;
            *cur = x[i];
        }
    }
    void write(long long x) {
        if (x == 0) *(cur++) = '0';
        else {
            if (x < 0) {
                *(cur++) = '-';
                x = -x;
            }
            char buf[20];
            int i = 0;
            while (x != 0) buf[i++] = x % 10 + '0', x /= 10;
            while (i--) *(cur++) = buf[i];
        }
    }
    template<class T, class... Args> void generate_message(T x, Args... args) {
        write(x);
        if constexpr (sizeof...(Args) > 0) generate_message(args...);
    }
public:
    Exception() noexcept { *cur = '\0'; }
    Exception(const Exception& x) noexcept {
        for (int i = 0; i != 512; ++i) str[i] = x.str[i];
        cur = x.cur;
    }
    explicit Exception(const char* what_arg) noexcept {
        for (int i = 0; i != 512; ++i, ++cur) {
            *cur = what_arg[i];
            if (what_arg[i] == '\0') break;
        }
    }
    template<class... Args> explicit Exception(Args... args) noexcept {
        generate_message(args...);
        *cur = '\0';
    }
    Exception& operator=(const Exception& x) noexcept {
        for (int i = 0; i != 512; ++i) str[i] = x.str[i];
        cur = x.cur;
        return *this;
    }
    const char* what() const noexcept { return str; }
};

}  

#include <type_traits>        
#include <limits>             
#include <bit>                
    
  
#include <type_traits>        
#include <utility>            
#include <initializer_list>   
#include <compare>            
#include <tuple>              
#include <memory>             


namespace gsh {

class InPlaceTag {};
constexpr InPlaceTag InPlace;

}  
       
  

namespace gsh {

class NullOpt {};
constexpr NullOpt Null;
template<class T> class Option {
    template<class U> friend class Option;
public:
    using value_type = T;
private:
    union {
        value_type val_;
    };
    bool has;
    template<class... Args> constexpr void construct(Args&&... args) { std::construct_at(std::addressof(val_), std::forward<Args>(args)...); }
    constexpr void destroy() { std::destroy_at(std::addressof(val_)); }
    template<class U> constexpr static bool is_explicit = !std::is_convertible_v<U, T>;
    constexpr static bool noexcept_swapable = []() {
        using std::swap;
        return noexcept(swap(std::declval<value_type&>(), std::declval<value_type&>()));
    }();
public:
    constexpr Option() noexcept : has(false) {}
    constexpr Option(NullOpt) noexcept : has(false) {}
    constexpr Option(const Option& rhs) {
        if (rhs.has) has = true, construct(rhs.val_);
        else has = false;
    }
    constexpr Option(Option&& rhs) noexcept {
        if (rhs.has) has = true, construct(std::move(rhs.val_));
        else has = false;
    }
    template<class... Args> constexpr explicit Option(InPlaceTag, Args&&... args) : has(true) { construct(std::forward<Args>(args)...); }
    template<class U, class... Args> constexpr explicit Option(InPlaceTag, std::initializer_list<U> il, Args&&... args) : has(true) { construct(il, std::forward<Args>(args)...); }
    template<class U = T>
        requires requires(U&& rhs) { static_cast<value_type>(std::forward<U>(rhs)); }
    explicit(is_explicit<U>) constexpr Option(U&& rhs) : has(true) {
        construct(std::forward<U>(rhs));
    }
    template<class U> explicit(is_explicit<U>) constexpr Option(const Option<U>& rhs) {
        if (rhs.has) has = true, construct(static_cast<value_type>(rhs.val_));
        else has = false;
    }
    template<class U> explicit(is_explicit<U>) constexpr Option(Option<U>&& rhs) {
        if (rhs.has) has = true, construct(static_cast<value_type>(std::move(rhs.val_)));
        else has = false;
    }
    constexpr ~Option() noexcept {
        if (has) destroy();
    }
    constexpr Option& operator=(NullOpt) noexcept {
        if (has) destroy(), has = false;
    }
    constexpr Option& operator=(const Option& rhs) {
        if (has) destroy();
        if (rhs.has) has = true, construct(rhs.ref());
        else has = false;
    }
    template<class... Args> constexpr T& emplace(Args&&... args) {
        if (has) destroy();
        has = true;
        construct(std::forward<Args>(args)...);
        return val_;
    }
    template<class U, class... Args> constexpr T& emplace(std::initializer_list<U> il, Args&&... args) {
        if (has) destroy();
        has = true;
        construct(il, std::forward<Args>(args)...);
        return val_;
    }
    constexpr void swap(Option& rhs) noexcept(noexcept_swapable) {
        using std::swap;
        swap(has, rhs.has);
        swap(val_, rhs.val_);
    }
    constexpr void reset() noexcept {
        if (has) destroy(), has = false;
    }
    constexpr T& operator*() & { return val_; }
    constexpr T&& operator*() && { return std::move(val_); }
    constexpr const T& operator*() const& { return val_; }
    constexpr const T&& operator*() const&& { return std::move(val_); }
    constexpr const T* operator->() const { return std::addressof(val_); }
    constexpr T* operator->() { return std::addressof(val_); }
    constexpr explicit operator bool() const noexcept { return has; }
    constexpr bool has_val() const noexcept { return has; }
    constexpr const T& val() const& {
        if (!has) throw gsh::Exception("gsh::Option::val / The object doesn't have value.");
        return val_;
    }
    constexpr T& val() & {
        if (!has) throw gsh::Exception("gsh::Option::val / The object doesn't have value.");
        return val_;
    }
    constexpr T&& val() && {
        if (!has) throw gsh::Exception("gsh::Option::val / The object doesn't have value.");
        return std::move(val_);
    }
    constexpr const T&& val() const&& {
        if (!has) throw gsh::Exception("gsh::Option::val / The object doesn't have value.");
        return std::move(val_);
    }
    template<class U> constexpr T val_or(U&& v) const& {
        if (has) return val_;
        else return std::forward<U>(v);
    }
    template<class U> constexpr T val_or(U&& v) && {
        if (has) return std::move(val_);
        else return std::forward<U>(v);
    }
};
template<class T> Option(T) -> Option<T>;

template<class T, class U> constexpr bool operator==(const Option<T>& x, const Option<U>& y) {
    if (x.has_val() && y.has_val()) return *x == *y;
    else return x.has_val() ^ y.has_val();
}
template<class T> constexpr bool operator==(const Option<T>& x, NullOpt) noexcept {
    return !x.has_val();
}
template<class T, class U> constexpr bool operator==(const Option<T>& x, const U& y) {
    return x.has_val() && (*x == y);
}
template<class T, class U> constexpr bool operator==(const T& x, const Option<U>& y) {
    return y.has_val() && (x == *y);
}
template<class T, std::three_way_comparable_with<T> U> constexpr std::compare_three_way_result_t<T, U> operator<=>(const Option<T>& x, const Option<U>& y) {
    return x.has_val() && y.has_val() ? *x <=> *y : x.has_val() <=> y.has_val();
}
template<class T> constexpr std::strong_ordering operator<=>(const Option<T>& x, NullOpt) noexcept {
    return x.has_val() <=> false;
}
template<class T, std::three_way_comparable_with<T> U> constexpr std::compare_three_way_result_t<T, U> operator<=>(const Option<T>& x, const U& y) {
    return x.has_val() ? *x <=> y : std::strong_ordering::less;
}

}  

namespace std {
template<class T> struct tuple_size<gsh::Option<T>> : integral_constant<size_t, 2> {};
template<class T> struct tuple_element<0, gsh::Option<T>> {
    using type = T;
};
template<class T> struct tuple_element<1, gsh::Option<T>> {
    using type = bool;
};
}  

namespace gsh {
template<std::size_t N, class T> auto get(const Option<T>& x) {
    if constexpr (N == 0) {
        if (x.has_val()) return *x;
        else return T();
    } else return x.has_val();
}
template<std::size_t N, class T> auto get(Option<T>& x) {
    if constexpr (N == 0) {
        if (x.has_val()) return *x;
        else return T();
    } else return x.has_val();
}
template<std::size_t N, class T> auto get(Option<T>&& x) {
    if constexpr (N == 0) {
        if (x.has_val()) return std::move(*x);
        else return T();
    } else return x.has_val();
}
}  

namespace gsh {

namespace internal {

    
    template<class T> constexpr T calc_gcd(T x, T y) {
        if (x == 0 || y == 0) return x | y;
        const itype::i32 n = std::countr_zero(x);
        const itype::i32 m = std::countr_zero(y);
        const itype::i32 l = n < m ? n : m;
        x >>= n;
        y >>= m;
        while (x != y) {
            const T a = y - x, b = x - y;
            const itype::i32 m = std::countr_zero(a), n = std::countr_zero(b);
            GSH_INTERNAL_ASSUME(m == n);
            const T s = y < x ? b : a;
            const T t = x < y ? x : y;
            x = s >> m;
            y = t;
        }
        return x << l;
    }

    template<class T> concept IsStaticModint = !requires(T x, typename T::value_type m) { x.set(m); };
    template<class T, itype::u32 id> class ModintBase {
    protected:
        static inline T mint{};
    };
    template<IsStaticModint T, itype::u32 id> class ModintBase<T, id> {
    protected:
        constexpr static T mint{};
    };

    template<class T, itype::u32 id = 0> class ModintImpl : public ModintBase<T, id> {
        typename T::value_type val_{};
        constexpr static auto& mint() noexcept { return ModintBase<T, id>::mint; }
        constexpr static ModintImpl construct(typename T::value_type x) noexcept {
            ModintImpl res;
            res.val_ = x;
            return res;
        }
    public:
        using value_type = typename T::value_type;
        constexpr static bool is_static_mod = IsStaticModint<T>;
        constexpr ModintImpl() noexcept {}
        template<class U> constexpr ModintImpl(U x) noexcept { operator=(x); }
        constexpr explicit operator value_type() const noexcept { return val(); }
        constexpr static void set_mod(value_type x) { mint().set(x); }
        constexpr value_type val() const noexcept { return mint().val(val_); }
        constexpr static value_type mod() noexcept { return mint().mod(); }
        template<class U> constexpr ModintImpl& operator=(U x) noexcept {
            val_ = mint().build(x);
            return *this;
        }
        constexpr static ModintImpl raw(value_type x) noexcept { return construct(mint().raw(x)); }
        constexpr ModintImpl inv() const noexcept { return construct(mint().inv(val_)); }
        constexpr ModintImpl pow(itype::u64 e) const noexcept { return construct(mint().pow(val_, e)); }
        constexpr ModintImpl operator+() const noexcept { return *this; }
        constexpr ModintImpl operator-() const noexcept { return construct(mint().neg(val_)); }
        constexpr ModintImpl& operator++() noexcept {
            val_ = mint().inc(val_);
            return *this;
        }
        constexpr ModintImpl& operator--() noexcept {
            val_ = mint().dec(val_);
            return *this;
        }
        constexpr ModintImpl operator++(int) noexcept {
            ModintImpl copy = *this;
            val_ = mint().inc(val_);
            return copy;
        }
        constexpr ModintImpl operator--(int) noexcept {
            ModintImpl copy = *this;
            val_ = mint().dec(val_);
            return copy;
        }
        constexpr ModintImpl& operator+=(ModintImpl x) noexcept {
            val_ = mint().add(val_, x.val_);
            return *this;
        }
        constexpr ModintImpl& operator-=(ModintImpl x) noexcept {
            val_ = mint().sub(val_, x.val_);
            return *this;
        }
        constexpr ModintImpl& operator*=(ModintImpl x) noexcept {
            val_ = mint().mul(val_, x.val_);
            return *this;
        }
        constexpr ModintImpl& operator/=(ModintImpl x) {
            val_ = mint().div(val_, x.val_);
            return *this;
        }
        friend constexpr ModintImpl operator+(ModintImpl l, ModintImpl r) noexcept { return construct(mint().add(l.val_, r.val_)); }
        friend constexpr ModintImpl operator-(ModintImpl l, ModintImpl r) noexcept { return construct(mint().sub(l.val_, r.val_)); }
        friend constexpr ModintImpl operator*(ModintImpl l, ModintImpl r) noexcept { return construct(mint().mul(l.val_, r.val_)); }
        friend constexpr ModintImpl operator/(ModintImpl l, ModintImpl r) { return construct(mint().div(l.val_, r.val_)); }
        friend constexpr bool operator==(ModintImpl l, ModintImpl r) noexcept { return mint().same(l.val_, r.val_); }
        friend constexpr bool operator!=(ModintImpl l, ModintImpl r) noexcept { return !mint().same(l.val_, r.val_); }
    };

    template<class D, class T> class ModintInterface {
        constexpr const D& derived() const noexcept { return *static_cast<const D*>(this); }
    public:
        using value_type = T;
        constexpr value_type val(value_type x) const noexcept { return x; }
        constexpr value_type build(itype::u32 x) const noexcept { return x % derived().mod(); }
        constexpr value_type build(itype::u64 x) const noexcept { return x % derived().mod(); }
        template<class U> constexpr value_type build(U x) const noexcept {
            static_assert(std::is_integral_v<U>, "gsh::internal::ModintInterface::build<U> / Only integer types can be assigned.");
            if constexpr (std::is_unsigned_v<U>) {
                if constexpr (std::is_same_v<U, itype::u128>) return derived().raw(static_cast<value_type>(x % derived().mod()));
                else if constexpr (std::is_same_v<U, unsigned long long> || std::is_same_v<U, unsigned long>) return derived().build(static_cast<itype::u64>(x));
                else return derived().build(static_cast<itype::u32>(x));
            } else {
                if (x < 0) {
                    if constexpr (std::is_same_v<U, itype::i128>) return derived().neg(derived().raw(static_cast<value_type>(-x % derived().mod())));
                    else if constexpr (std::is_same_v<U, long long> || std::is_same_v<U, long>) return derived().neg(derived().build(static_cast<itype::u64>(-x)));
                    else return derived().neg(derived().build(static_cast<itype::u32>(-x)));
                } else {
                    if constexpr (std::is_same_v<U, itype::i128>) return derived().raw(static_cast<value_type>(x % derived().mod()));
                    else if constexpr (std::is_same_v<U, long long> || std::is_same_v<U, long>) return derived().build(static_cast<itype::u64>(x));
                    else return derived().build(static_cast<itype::u32>(x));
                }
            }
        }
        constexpr value_type raw(value_type x) const noexcept { return x; }
        constexpr value_type zero() const noexcept { return derived().raw(0); }
        constexpr value_type one() const noexcept { return derived().raw(1); }
        constexpr value_type neg(value_type x) const noexcept {
            GSH_INTERNAL_ASSUME(x < derived().mod());
            return x == 0 ? 0 : derived().mod() - x;
        }
        constexpr value_type inc(value_type x) const noexcept {
            GSH_INTERNAL_ASSUME(x < derived().mod());
            return x + 1 == derived().mod() ? 0 : x + 1;
        }
        constexpr value_type dec(value_type x) const noexcept {
            GSH_INTERNAL_ASSUME(x < derived().mod());
            return x == 0 ? derived().mod() - 1 : x - 1;
        }
        constexpr value_type add(value_type x, value_type y) const noexcept {
            GSH_INTERNAL_ASSUME(x < derived().mod() && y < derived().mod());
            return derived().mod() - x > y ? x + y : y - (derived().mod() - x);
        }
        constexpr value_type sub(value_type x, value_type y) const noexcept {
            GSH_INTERNAL_ASSUME(x < derived().mod() && y < derived().mod());
            return x >= y ? x - y : derived().mod() - (y - x);
        }
        constexpr value_type fma(value_type x, value_type y, value_type z) const noexcept { return derived().add(derived().mul(x, y), z); }
        constexpr value_type div(value_type x, value_type y) const noexcept {
            const value_type iv = derived().inv(y);
            if (derived().same(iv, derived().zero())) [[unlikely]]
                throw gsh::Exception("gsh::internal::ModintInterface::div / Cannot calculate inverse.");
            return derived().mul(x, iv);
        }
        constexpr bool same(value_type x, value_type y) const noexcept { return x == y; }
        constexpr value_type abs(value_type x) const noexcept { return derived().val(x) > (derived().mod() / 2) ? derived().neg(x) : x; }
        constexpr value_type pow(value_type x, itype::u64 e) const noexcept {
            value_type res = derived().one();
            while (e) {
                auto tmp = derived().mul(x, x);
                if (e & 1) res = derived().mul(res, x);
                x = tmp;
                e >>= 1;
            }
            return res;
        }
        constexpr value_type inv(value_type t) const noexcept {
            auto a = 1, b = 0, x = derived().val(t), y = derived().mod();
            while (true) {
                if (x <= 1) {
                    if (x == 0) [[unlikely]]
                        return derived().zero();
                    else return derived().build(a);
                }
                b += a * (y / x);
                y %= x;
                if (y <= 1) {
                    if (y == 0) [[unlikely]]
                        return derived().zero();
                    else return derived().build(derived().mod() - b);
                }
                a += b * (x / y);
                x %= y;
            }
        }
        constexpr itype::i32 legendre(value_type x) const noexcept {
            auto res = derived().pow(x, (derived().mod() - 1) >> 1);
            const bool a = derived().same(res, derived().zero()), b = derived().same(res, derived().one());
            return a ? 0 : (b ? 1 : -1);
        }
        constexpr itype::i32 jacobi(value_type x) const noexcept {
            auto a = derived().val(x), n = derived().mod();
            if (a == 1) return 1;
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
        constexpr Option<value_type> sqrt(value_type n) const noexcept {
            const auto md = derived().mod();
            if (derived().same(n, derived().zero()) || derived().same(n, derived().one())) return n;
            if (md % 4 == 3) {
                auto res = derived().pow(n, (md + 1) >> 2);
                if (!derived().same(derived().mul(res, res), n)) return Null;
                else return derived().abs(res);
            } else if (md % 8 == 5) {
                auto res = derived().pow(n, (md + 3) >> 3);
                if (!derived().same(derived().mul(res, res), n)) {
                    const auto p = derived().pow(derived().raw(2), (md - 1) >> 2);
                    res = derived().mul(res, p);
                }
                if (!derived().same(derived().mul(res, res), n)) return Null;
                else return derived().abs(res);
            } else {
                const itype::u32 S = std::countr_zero(md - 1);
                const itype::u32 W = std::bit_width(md);
                if (false && S * S <= 12 * W) {
                    const auto Q = (md - 1) >> S;
                    const auto tmp = derived().pow(n, Q / 2);
                    auto R = derived().mul(tmp, n), t = derived().mul(tmp, R);
                    if (derived().same(t, derived().one())) return R;
                    auto u = t;
                    for (itype::u32 i = 0; i != S - 1; ++i) u = derived().mul(u, u);
                    if (!derived().same(u, derived().one())) return Null;
                    const auto z = derived().pow(
                      [&]() {
                          if (md % 3 == 2) return derived().raw(3);
                          if (auto x = md % 5; x == 2 || x == 3) return derived().raw(5);
                          if (auto x = md % 7; x == 3 || x == 5 || x == 6) return derived().raw(7);
                          if (auto x = md % 11; x == 2 || x == 6 || x == 7 || x == 8 || x == 10) return derived().build(11);
                          if (auto x = md % 13; x == 2 || x == 5 || x == 6 || x == 7 || x == 8 || x == 11) return derived().build(13);
                          for (const itype::u32 x : { 17, 19, 23, 29, 31, 37, 41, 43, 47, 53, 59, 61, 67, 71, 73, 79, 83, 89, 97 }) {
                              const auto y = derived().build(x);
                              if (derived().legendre(y) == -1) return y;
                          }
                          auto z = derived().build(101);
                          while (derived().legendre(z) != -1) z = derived().add(z, derived().raw(2));
                          return z;
                      }(),
                      Q);
                    itype::u32 M = S;
                    auto c = z;
                    do {
                        auto U = derived().mul(t, t);
                        itype::u32 i = 1;
                        while (!derived().same(U, derived().one())) U = derived().mul(U, U), ++i;
                        auto b = c;
                        for (itype::u32 j = 0, k = M - i - 1; j != k; ++j) b = derived().mul(b, b);
                        M = i, c = derived().mul(b, b), t = derived().mul(t, c), R = derived().mul(R, b);
                    } while (!derived().same(t, derived().one()));
                    return derived().abs(R);
                } else {
                    if (derived().legendre(n) != 1) return Null;
                    auto a = derived().raw(4);
                    decltype(a) w;
                    while (derived().legendre(w = derived().sub(derived().mul(a, a), n)) != -1) a = derived().inc(a);
                    auto res1 = derived().one(), res2 = derived().zero(), pow1 = a, pow2 = res1;
                    auto e = (md + 1) / 2;
                    while (true) {
                        const auto tmp2 = derived().mul(pow2, w);
                        if (e & 1) {
                            const auto tmp = res1;
                            res1 = derived().add(derived().mul(res1, pow1), derived().mul(res2, tmp2));
                            res2 = derived().add(derived().mul(tmp, pow2), derived().mul(res2, pow1));
                        }
                        e >>= 1;
                        if (e == 0) return derived().abs(res1);
                        const auto tmp = pow1;
                        pow1 = derived().add(derived().mul(pow1, pow1), derived().mul(pow2, tmp2));
                        pow2 = derived().mul(pow2, derived().add(tmp, tmp));
                    }
                }
            }
        }
    };

    template<itype::u32 mod_> class StaticModint32Impl : public ModintInterface<StaticModint32Impl<mod_>, itype::u32> {
    public:
        constexpr StaticModint32Impl() noexcept {}
        constexpr itype::u32 mod() const noexcept { return mod_; }
        constexpr itype::u32 mul(itype::u32 x, itype::u32 y) const noexcept {
            GSH_INTERNAL_ASSUME(x < mod_ && y < mod_);
            return static_cast<itype::u64>(x) * y % mod_;
        }
    };
    template<itype::u64 mod_> class StaticModint64Impl : public ModintInterface<StaticModint64Impl<mod_>, itype::u64> {
    public:
        constexpr StaticModint64Impl() noexcept {}
        constexpr itype::u64 mod() const noexcept { return mod_; }
        constexpr itype::u64 mul(itype::u64 x, itype::u64 y) const noexcept {
            GSH_INTERNAL_ASSUME(x < mod_ && y < mod_);
            constexpr itype::u128 M_ = std::numeric_limits<itype::u128>::max() / mod_ + std::has_single_bit(mod_);
            if constexpr (mod_ < (1ull << 63)) {
                const itype::u64 a = (((M_ * x) >> 64) * y) >> 64;
                const itype::u64 b = x * y;
                const itype::u64 c = a * mod_;
                const itype::u64 d = b - c;
                const bool e = d < mod_;
                const itype::u64 f = d - mod_;
                return e ? d : f;
            } else {
                const itype::u64 a = (((M_ * x) >> 64) * y) >> 64;
                const itype::u128 b = static_cast<itype::u128>(x) * y;
                const itype::u128 c = static_cast<itype::u128>(a) * mod_;
                const itype::u128 d = b - c;
                const bool e = d < mod_;
                const itype::u128 f = d - mod_;
                return e ? d : f;
            }
        }
    };
    template<itype::u64 mod_> using StaticModintImpl = std::conditional_t<(mod_ <= 0xffffffff), StaticModint32Impl<mod_>, StaticModint64Impl<mod_>>;

    class DynamicModint32Impl : public ModintInterface<DynamicModint32Impl, itype::u32> {
        itype::u32 mod_ = 0;
        itype::u64 M_ = 0;
    public:
        constexpr DynamicModint32Impl() noexcept {}
        constexpr void set(itype::u32 n) {
            if (n <= 1) [[unlikely]]
                throw Exception("gsh::internal::DynamicModint32Impl::set / Mod must be at least 2.");
            mod_ = n;
            M_ = std::numeric_limits<itype::u64>::max() / mod_ + std::has_single_bit(mod_);
        }
        constexpr itype::u32 mod() const noexcept { return mod_; }
        constexpr itype::u32 mul(itype::u32 x, itype::u32 y) const noexcept {
            GSH_INTERNAL_ASSUME(x < mod_ && y < mod_);
            const itype::u64 a = static_cast<itype::u64>(x) * y;
            const itype::u64 b = (static_cast<itype::u128>(M_) * a) >> 64;
            const itype::u64 c = a - b * mod_;
            return c < mod_ ? c : c - mod_;
        }
    };

    class DynamicModint64Impl : public ModintInterface<DynamicModint64Impl, itype::u64> {
        itype::u64 mod_ = 0;
        itype::u128 M_ = 0;
    public:
        constexpr DynamicModint64Impl() noexcept {}
        constexpr void set(itype::u64 n) {
            if (n <= 1) [[unlikely]]
                throw Exception("gsh::internal::DynamicModint64Impl::set / Mod must be at least 2.");
            mod_ = n;
            M_ = std::numeric_limits<itype::u128>::max() / mod_ + std::has_single_bit(mod_);
        }
        constexpr itype::u64 mod() const noexcept { return mod_; }
        constexpr itype::u64 mul(itype::u64 x, itype::u64 y) const noexcept {
            GSH_INTERNAL_ASSUME(x < mod_ && y < mod_);
            const itype::u64 a = (((M_ * x) >> 64) * y) >> 64;
            const itype::u64 b = x * y;
            const itype::u64 c = a * mod_;
            const itype::u64 d = b - c;
            const bool e = d < mod_;
            const itype::u64 f = d - mod_;
            return e ? d : f;
        }
    };

    class MontgomeryModint64Impl : public ModintInterface<MontgomeryModint64Impl, itype::u64> {
        itype::u64 mod_ = 0, R2 = 0, ninv = 0;
        constexpr itype::u64 reduce(const itype::u128 t) const noexcept {
            const itype::u64 a = t, b = t >> 64;
            const itype::u64 c = (static_cast<itype::u128>(a * ninv) * mod_) >> 64;
            return b + c + (a != 0);
        }
    public:
        constexpr MontgomeryModint64Impl() noexcept {}
        constexpr void set(itype::u64 n) {
            if (n <= 1) [[unlikely]]
                throw Exception("gsh::internal::MontgomeryModint64Impl::set / Mod must be at least 2.");
            if (n % 2 == 0) [[unlikely]]
                throw Exception("gsh::internal::MontgomeryModint64Impl::set / It is not allowed to set the modulo to an even number.");
            mod_ = n;
            R2 = -static_cast<itype::u128>(mod_) % mod_;
            ninv = mod_;
            for (itype::u32 i = 0; i != 5; ++i) ninv *= 2 - mod_ * ninv;
            ninv = -ninv;
        }
        constexpr itype::u64 val(itype::u64 x) const noexcept {
            const itype::u64 res = static_cast<itype::u64>((static_cast<itype::u128>(x * ninv) * mod_) >> 64) + (x != 0);
            return res == mod_ ? 0 : res;
        }
        constexpr itype::u64 mod() const noexcept { return mod_; }
        constexpr itype::u64 build(itype::u32 x) const noexcept { return reduce(static_cast<itype::u128>(x % mod_) * R2); }
        constexpr itype::u64 build(itype::u64 x) const noexcept { return reduce(static_cast<itype::u128>(x % mod_) * R2); }
        template<class U> constexpr itype::u64 build(U x) const noexcept { return ModintInterface::build(x); }
        constexpr itype::u64 raw(itype::u64 x) const noexcept { return reduce(static_cast<itype::u128>(x) * R2); }
        constexpr itype::u64 neg(itype::u64 x) const noexcept {
            const itype::u64 tmp = 2 * mod_ - x;
            return x == 0 ? 0 : tmp;
        }
        constexpr itype::u64 inc(itype::u64 x) const noexcept { return x + 1 == 2 * mod_ ? 0 : x + 1; }
        constexpr itype::u64 dec(itype::u64 x) const noexcept { return x == 0 ? 2 * mod_ - 1 : x - 1; }
        constexpr itype::u64 add(itype::u64 x, itype::u64 y) const noexcept { return x + y >= 2 * mod_ ? x + y - 2 * mod_ : x + y; }
        constexpr itype::u64 sub(itype::u64 x, itype::u64 y) const noexcept { return x - y < 2 * mod_ ? x - y : 2 * mod_ + (x - y); }
        constexpr itype::u64 mul(itype::u64 x, itype::u64 y) const noexcept { return reduce(static_cast<itype::u128>(x) * y); }
        constexpr itype::u64 fma(itype::u64 x, itype::u64 y, itype::u64 z) const noexcept {
            const itype::u128 t = static_cast<itype::u128>(x) * y;
            const itype::u64 a = t, b = t >> 64;
            const itype::u64 c = (static_cast<itype::u128>(a * ninv) * mod_) >> 64;
            const itype::u64 res = b + c + (a != 0) + z;
            return res < 2 * mod_ ? res : res - 2 * mod_;
        }
        constexpr bool same(itype::u64 x, itype::u64 y) const noexcept { return x + mod_ == y || x == y; }
    };

}  

template<itype::u32 mod_ = 998244353> using StaticModint32 = internal::ModintImpl<internal::StaticModint32Impl<mod_>>;
template<itype::u64 mod_ = 998244353> using StaticModint64 = internal::ModintImpl<internal::StaticModint64Impl<mod_>>;
template<itype::u64 mod_ = 998244353> using StaticModint = internal::ModintImpl<internal::StaticModintImpl<mod_>>;
template<itype::u32 id = 0> using DynamicModint32 = internal::ModintImpl<internal::DynamicModint32Impl, id>;
template<itype::u32 id = 0> using DynamicModint64 = internal::ModintImpl<internal::DynamicModint64Impl, id>;
template<itype::u32 id = 0> using MontgomeryModint64 = internal::ModintImpl<internal::MontgomeryModint64Impl, id>;

}  

#include <type_traits>
#include <cmath>
#include <new>


#include <iterator>           
#include <algorithm>          
#include <initializer_list>   
#include <type_traits>        
#include <concepts>           
#include <cstring>            
#include <utility>            
#include <tuple>              
    
  
#include <type_traits>      
#include <concepts>         
#include <utility>          
#include <iterator>         
#include <ranges>           
#include <tuple>            
  

namespace gsh {

template<class R> concept Range = std::ranges::range<R>;
template<class R, class T> concept Rangeof = Range<R> && std::same_as<T, std::ranges::range_value_t<R>>;
template<class R> concept InputRange = std::ranges::input_range<R>;
template<class R, class T> concept OutputRange = Range<R> && std::ranges::output_range<R, T>;
template<class R> concept ForwardRange = std::ranges::forward_range<R>;
template<class R> concept BidirectionalRange = std::ranges::bidirectional_range<R>;
template<class R> concept RandomAccessRange = std::ranges::random_access_range<R>;
enum class RangeKind { Sized, Unsized };

namespace internal {
    template<class T, class U> concept same_ncvr = std::same_as<std::remove_cvref_t<T>, std::remove_cvref_t<U>>;
}
template<Range R> class RangeTraits {
public:
    using value_type = std::ranges::range_value_t<R>;
    using iterator = std::ranges::iterator_t<R>;
    using sentinel = std::ranges::sentinel_t<R>;
    using const_iterator = decltype(std::ranges::cbegin(std::declval<R&>()));
    using const_sentinel = decltype(std::ranges::cend(std::declval<R&>()));
    using size_type = std::ranges::range_size_t<R>;
    using difference_type = std::ranges::range_difference_t<R>;
    using reference = std::ranges::range_reference_t<R>;
    using const_reference = std::common_reference_t<const std::iter_value_t<iterator>&&, std::iter_reference_t<iterator>>;
    using rvalue_reference = std::ranges::range_rvalue_reference_t<R>;
    using range_type = std::remove_cvref_t<R>;
    constexpr static RangeKind range_kind = std::ranges::sized_range<R> ? RangeKind::Sized : RangeKind::Unsized;
    constexpr static bool pointer_obtainable = requires(R r) { std::ranges::data(r); };

    template<internal::same_ncvr<R> T> static constexpr auto size(T&& r) { return std::ranges::size(std::forward<T>(r)); }
    template<internal::same_ncvr<R> T> static constexpr auto ssize(T&& r) { return std::ranges::ssize(std::forward<T>(r)); }
    template<internal::same_ncvr<R> T> static constexpr auto empty(T&& r) { return std::ranges::empty(std::forward<T>(r)); }

    template<internal::same_ncvr<R> T> static constexpr auto begin(T&& r) { return std::ranges::begin(std::forward<T>(r)); }
    template<internal::same_ncvr<R> T> static constexpr auto end(T&& r) { return std::ranges::end(std::forward<T>(r)); }
    template<internal::same_ncvr<R> T> static constexpr auto cbegin(T&& r) { return std::ranges::cbegin(std::forward<T>(r)); }
    template<internal::same_ncvr<R> T> static constexpr auto cend(T&& r) { return std::ranges::cend(std::forward<T>(r)); }
    template<internal::same_ncvr<R> T> static constexpr auto rbegin(T&& r) { return std::ranges::rbegin(std::forward<T>(r)); }
    template<internal::same_ncvr<R> T> static constexpr auto rend(T&& r) { return std::ranges::rend(std::forward<T>(r)); }
    template<internal::same_ncvr<R> T> static constexpr auto crbegin(T&& r) { return std::ranges::crbegin(std::forward<T>(r)); }
    template<internal::same_ncvr<R> T> static constexpr auto crend(T&& r) { return std::ranges::crend(std::forward<T>(r)); }

    template<internal::same_ncvr<R> T> static constexpr auto mbegin(T&& r) { return std::move_iterator(begin(std::forward<T>(r))); }
    template<internal::same_ncvr<R> T> static constexpr auto mend(T&& r) { return std::move_sentinel(end(std::forward<T>(r))); }
    template<internal::same_ncvr<R> T> static constexpr auto mcbegin(T&& r) { return std::move_iterator(cbegin(std::forward<T>(r))); }
    template<internal::same_ncvr<R> T> static constexpr auto mcend(T&& r) { return std::move_sentinel(cend(std::forward<T>(r))); }
    template<internal::same_ncvr<R> T> static constexpr auto mrbegin(T&& r) { return std::move_iterator(rbegin(std::forward<T>(r))); }
    template<internal::same_ncvr<R> T> static constexpr auto mrend(T&& r) { return std::move_sentinel(rend(std::forward<T>(r))); }
    template<internal::same_ncvr<R> T> static constexpr auto mcrbegin(T&& r) { return std::move_iterator(crbegin(std::forward<T>(r))); }
    template<internal::same_ncvr<R> T> static constexpr auto mcrend(T&& r) { return std::move_sentinel(crend(std::forward<T>(r))); }

    template<internal::same_ncvr<R> T> static constexpr auto fbegin(T&& r) {
        if constexpr (!std::ranges::borrowed_range<std::remove_cvref_t<T>>) return begin(std::forward<T>(r));
        else return mbegin(std::forward<T>(r));
    }
    template<internal::same_ncvr<R> T> static constexpr auto fend(T&& r) {
        if constexpr (!std::ranges::borrowed_range<std::remove_cvref_t<T>>) return end(std::forward<T>(r));
        else return mend(std::forward<T>(r));
    }
    template<internal::same_ncvr<R> T> static constexpr auto fcbegin(T&& r) {
        if constexpr (!std::ranges::borrowed_range<std::remove_cvref_t<T>>) return cbegin(std::forward<T>(r));
        else return mcbegin(std::forward<T>(r));
    }
    template<internal::same_ncvr<R> T> static constexpr auto fcend(T&& r) {
        if constexpr (!std::ranges::borrowed_range<std::remove_cvref_t<T>>) return cend(std::forward<T>(r));
        else return mcend(std::forward<T>(r));
    }
    template<internal::same_ncvr<R> T> static constexpr auto frbegin(T&& r) {
        if constexpr (!std::ranges::borrowed_range<std::remove_cvref_t<T>>) return rbegin(std::forward<T>(r));
        else return mrbegin(std::forward<T>(r));
    }
    template<internal::same_ncvr<R> T> static constexpr auto frend(T&& r) {
        if constexpr (!std::ranges::borrowed_range<std::remove_cvref_t<T>>) return rend(std::forward<T>(r));
        else return mrend(std::forward<T>(r));
    }
    template<internal::same_ncvr<R> T> static constexpr auto fcrbegin(T&& r) {
        if constexpr (!std::ranges::borrowed_range<std::remove_cvref_t<T>>) return crbegin(std::forward<T>(r));
        else return mcrbegin(std::forward<T>(r));
    }
    template<internal::same_ncvr<R> T> static constexpr auto fcrend(T&& r) {
        if constexpr (!std::ranges::borrowed_range<std::remove_cvref_t<T>>) return crend(std::forward<T>(r));
        else return mcrend(std::forward<T>(r));
    }

    template<internal::same_ncvr<R> T> static constexpr auto data(T&& r) { return std::ranges::data(std::forward<T>(r)); }
    template<internal::same_ncvr<R> T> static constexpr auto cdata(T&& r) { return std::ranges::cdata(std::forward<T>(r)); }
};

template<class D, class V>
    requires std::is_class_v<D> && std::same_as<D, std::remove_cv_t<D>>
class ViewInterface;

template<class Iter> class SlicedRange : public ViewInterface<SlicedRange<Iter>, std::iter_value_t<Iter>> {
public:
    using iterator = Iter;
    using value_type = std::iter_value_t<Iter>;
    static_assert(std::sentinel_for<iterator, iterator>, "gsh::SlicedRange / The iterator cannot behave as sentinel.");
private:
    iterator first, last;
public:
    constexpr SlicedRange(iterator beg, iterator end) : first(beg), last(end) {}
    constexpr iterator begin() const { return first; }
    constexpr iterator end() const { return last; }
    constexpr auto rbegin() const { return std::reverse_iterator{ last }; }
    constexpr auto rend() const { return std::reverse_iterator{ first }; }
};

template<class D, class V>
    requires std::is_class_v<D> && std::same_as<D, std::remove_cv_t<D>>
class ViewInterface {
    constexpr D& get_ref() { return *reinterpret_cast<D*>(this); }
    constexpr const D& get_ref() const { return *reinterpret_cast<const D*>(this); }
    constexpr auto get_begin() { return get_ref().begin(); }
    constexpr auto get_begin() const { return get_ref().cbegin(); }
    constexpr auto get_end() { return get_ref().end(); }
    constexpr auto get_end() const { return get_ref().cend(); }
    constexpr auto get_rbegin() { return get_ref().rbegin(); }
    constexpr auto get_rbegin() const { return get_ref().crbegin(); }
    constexpr auto get_rend() { return get_ref().rend(); }
    constexpr auto get_rend() const { return get_ref().crend(); }
public:
    using derived_type = D;
    using value_type = V;
    constexpr derived_type copy() const& { return get_ref(); }
    constexpr derived_type copy() & { return get_ref(); }
    constexpr derived_type copy() && { return std::move(get_ref()); }
    constexpr auto slice(itype::u32 a, itype::u32 b) {
        auto beg = std::next(get_begin(), a);
        auto end = std::next(beg, b - a);
        return SlicedRange{ beg, end };
    }
    constexpr auto slice(itype::u32 a, itype::u32 b) const {
        auto beg = std::next(get_begin(), a);
        auto end = std::next(beg, b - a);
        return SlicedRange{ beg, end };
    }
    constexpr auto slice(itype::u32 a) { return SlicedRange{ std::next(get_begin(), a), get_end() }; }
    constexpr auto slice(itype::u32 a) const { return SlicedRange{ std::next(get_begin(), a), get_end() }; }
    template<std::predicate<value_type> Pred> constexpr bool all_of(Pred f) const {
        for (const auto& el : get_ref())
            if (!f(el)) return false;
        return true;
    }
    constexpr bool all_of(const value_type& x) const {
        for (const auto& el : get_ref())
            if (!(el == x)) return false;
        return true;
    }
    template<std::predicate<value_type> Pred> constexpr bool any_of(Pred f) const {
        for (const auto& el : get_ref())
            if (f(el)) return true;
        return false;
    }
    constexpr bool any_of(const value_type& x) const {
        for (const auto& el : get_ref())
            if (el == x) return true;
        return false;
    }
    template<std::predicate<value_type> Pred> constexpr bool none_of(Pred f) const {
        for (const auto& el : get_ref())
            if (f(el)) return false;
        return true;
    }
    constexpr bool none_of(const value_type& x) const {
        for (const auto& el : get_ref())
            if (el == x) return false;
        return true;
    }
    constexpr bool contains(const value_type& x) const {
        for (const auto& el : get_ref())
            if (el == x) return true;
        return false;
    }
    constexpr auto find(const value_type& x) const {
        const auto end = get_end();
        for (auto itr = get_begin(); itr != end; ++itr)
            if (*itr == x) return itr;
        return end;
    }
    constexpr itype::u32 count(const value_type& x) const {
        itype::u32 res = 0;
        for (const auto& el : get_ref()) res += (el == x);
        return res;
    }
};

namespace internal {
    template<class T, class U> concept difference_from = std::same_as<std::remove_cvref_t<T>, std::remove_cvref_t<U>>;
    template<class From, class To> concept convertible_to_non_slicing = std::convertible_to<From, To> && !(std::is_pointer_v<std::decay_t<From>> && std::is_pointer_v<std::decay_t<To>> && !std::convertible_to<std::remove_pointer_t<std::decay_t<From>> (*)[], std::remove_pointer_t<std::decay_t<To>> (*)[]>);
    template<class T> concept pair_like =  std::tuple_size_v<std::remove_cvref_t<T>> == 2;
    template<class T, class U, class V> concept pair_like_convertible_from = !std::ranges::range<T> && !std::is_reference_v<T> && pair_like<T> && std::constructible_from<T, U, V> && convertible_to_non_slicing<U, std::tuple_element_t<0, T>> && std::convertible_to<V, std::tuple_element_t<1, T>>;
}  
template<std::input_or_output_iterator I, std::sentinel_for<I> S = I, RangeKind K = std::sized_sentinel_for<S, I> ? RangeKind::Sized : RangeKind::Unsized>
    requires(K == RangeKind::Sized || !std::sized_sentinel_for<S, I>)
class Subrange : public ViewInterface<Subrange<I, S, K>, std::iter_value_t<I>> {
    I itr;
    S sent;
    static constexpr bool StoreSize = (K == RangeKind::Sized && !std::sized_sentinel_for<S, I>);
    struct empty_sz {};
    [[no_unique_address]] std::conditional_t<StoreSize, std::make_unsigned_t<std::iter_difference_t<I>>, empty_sz> sz;
public:
    constexpr Subrange() = default;
    constexpr Subrange(internal::convertible_to_non_slicing<I> auto i, S s)
        requires(!StoreSize)
      : itr(i),
        sent(s) {}
    constexpr Subrange(internal::convertible_to_non_slicing<I> auto i, S s, std::make_unsigned_t<std::iter_difference_t<I>> n)
        requires(K == RangeKind::Sized)
      : itr(i),
        sent(s) {
        if constexpr (StoreSize) sz = n;
    }
    template<internal::difference_from<Subrange> R>
        requires std::ranges::borrowed_range<R> && internal::convertible_to_non_slicing<std::ranges::iterator_t<R>, I>
                 && std::convertible_to<std::ranges::sentinel_t<R>, S>
                 constexpr Subrange(R&& r)
                     requires(!StoreSize || std::ranges::sized_range<R>)
      : itr(std::ranges::begin(r)),
        sent(std::ranges::end(r)) {}
    template<std::ranges::borrowed_range R>
        requires internal::convertible_to_non_slicing<std::ranges::iterator_t<R>, I>
                 && std::convertible_to<std::ranges::sentinel_t<R>, S>
                 constexpr Subrange(R&& r, std::make_unsigned_t<std::iter_difference_t<I>> n)
                     requires(K == RangeKind::Sized)
      : Subrange{ std::ranges::begin(r), std::ranges::end(r), n } {}
    template<internal::difference_from<Subrange> PairLike>
        requires internal::pair_like_convertible_from<PairLike, const I&, const S&>
    constexpr operator PairLike() const {
        return PairLike(itr, sent);
    }
    constexpr I begin() const
        requires std::copyable<I>
    {
        return itr;
    }
    [[nodiscard]] constexpr I begin()
        requires(!std::copyable<I>)
    {
        return std::move(itr);
    }
    constexpr S end() const { return sent; }
    constexpr bool empty() const { return itr == sent; }
    constexpr I data() const
        requires(std::is_pointer_v<I> && std::copyable<I>)
    {
        return itr;
    }
    constexpr I data() const
        requires(std::is_pointer_v<I> && !std::copyable<I>)
    {
        return std::move(itr);
    }
    [[nodiscard]] constexpr Subrange next(std::iter_difference_t<I> n = 1) const&
        requires std::forward_iterator<I>
    {
        auto tmp = *this;
        tmp.advance(n);
        return tmp;
    }
    [[nodiscard]] constexpr Subrange next(std::iter_difference_t<I> n = 1) && {
        advance(n);
        return std::move(*this);
    }
    [[nodiscard]] constexpr Subrange prev(std::iter_difference_t<I> n = 1) const
        requires std::bidirectional_iterator<I>
    {
        auto tmp = *this;
        tmp.advance(-n);
        return tmp;
    }
    constexpr Subrange& advance(std::iter_difference_t<I> n) {
        if constexpr (StoreSize) {
            auto d = n - std::ranges::advance(itr, n, sent);
            if (d >= 0) sz -= static_cast<std::make_unsigned_t<std::remove_cvref_t<decltype(d)>>>(d);
            else sz += static_cast<std::make_unsigned_t<std::remove_cvref_t<decltype(d)>>>(d);
            return *this;
        } else {
            std::ranges::advance(itr, n, sent);
            return *this;
        }
    }
};
template<std::input_or_output_iterator I, std::sentinel_for<I> S> Subrange(I, S) -> Subrange<I, S>;
template<std::input_or_output_iterator I, std::sentinel_for<I> S> Subrange(I, S, std::make_unsigned_t<std::iter_difference_t<I>>) -> Subrange<I, S, RangeKind::Sized>;
template<std::ranges::borrowed_range R> Subrange(R&&) -> Subrange<std::ranges::iterator_t<R>, std::ranges::sentinel_t<R>, (std::ranges::sized_range<R> || std::sized_sentinel_for<std::ranges::sentinel_t<R>, std::ranges::iterator_t<R>>) ? RangeKind::Sized : RangeKind::Unsized>;
template<std::ranges::borrowed_range R> Subrange(R&&, std::make_unsigned_t<std::ranges::range_difference_t<R>>) -> Subrange<std::ranges::iterator_t<R>, std::ranges::sentinel_t<R>, RangeKind::Sized>;

}  

namespace std::ranges {
template<class I, class S, gsh::RangeKind K> constexpr bool enable_borrowed_range<gsh::Subrange<I, S, K>> = true;
}      
#include <type_traits>      
#include <limits>           
#include <utility>          
#include <iterator>         
#include <cstdlib>          
#include <new>              
#include <memory>           
  

namespace gsh {

namespace internal {
    template<class T, class U> struct GetPtr {
        using type = U*;
    };
    template<class T, class U>
        requires requires { typename T::pointer; }
    struct GetPtr<T, U> {
        using type = typename T::pointer;
    };
    template<class T, class U> struct RepFirst {};
    template<template<class, class...> class SomeTemplate, class U, class T, class... Types> struct RepFirst<SomeTemplate<T, Types...>, U> {
        using type = SomeTemplate<U, Types...>;
    };
    template<class T, class U> struct Rebind {
        using type = typename RepFirst<T, U>::type;
    };
    template<class T, class U>
        requires requires { typename T::template rebind<U>; }
    struct Rebind<T, U> {
        using type = typename T::template rebind<U>;
    };
    template<class T, class U> struct GetRebindPtr {
        using type = typename Rebind<T, U>::type;
    };
    template<class T, class U> struct GetRebindPtr<T*, U> {
        using type = U;
    };
    template<class T, class U, class V> struct GetConstPtr {
        using type = typename GetRebindPtr<U, const V*>::type;
    };
    template<class T, class U, class V>
        requires requires { typename T::const_pointer; }
    struct GetConstPtr<T, U, V> {
        using type = typename T::const_pointer;
    };
    template<class T, class U> struct GetVoidPtr {
        using type = typename GetRebindPtr<U, void*>::type;
    };
    template<class T, class U>
        requires requires { typename T::void_pointer; }
    struct GetVoidPtr<T, U> {
        using type = typename T::void_pointer;
    };
    template<class T, class U> struct GetConstVoidPtr {
        using type = typename GetRebindPtr<U, const void*>::type;
    };
    template<class T, class U>
        requires requires { typename T::const_void_pointer; }
    struct GetConstVoidPtr<T, U> {
        using type = typename T::const_void_pointer;
    };
    template<class T> struct GetDifferenceTypeSub {
        using type = itype::i32;
    };
    template<class T>
        requires requires { typename T::difference_type; }
    struct GetDifferenceTypeSub<T> {
        using type = typename T::difference_type;
    };
    template<class T, class U> struct GetDifferenceType {
        using type = typename GetDifferenceTypeSub<U>::type;
    };
    template<class T, class U>
        requires requires { typename T::difference_type; }
    struct GetDifferenceType<T, U> {
        using type = typename T::difference_type;
    };
    template<class T, class U> struct GetSizeType {
        using type = std::make_unsigned_t<U>;
    };
    template<class T, class U>
        requires requires { typename T::size_type; }
    struct GetSizeType<T, U> {
        using type = typename T::size_type;
    };
    template<class T> struct IsPropCopy {
        using type = std::false_type;
    };
    template<class T>
        requires requires { typename T::propagate_on_container_copy_assignment; }
    struct IsPropCopy<T> {
        using type = typename T::propagate_on_container_copy_assignment;
    };
    template<class T> struct IsPropMove {
        using type = std::false_type;
    };
    template<class T>
        requires requires { typename T::propagate_on_container_move_assignment; }
    struct IsPropMove<T> {
        using type = typename T::propagate_on_container_move_assignment;
    };
    template<class T> struct IsPropSwap {
        using type = std::false_type;
    };
    template<class T>
        requires requires { typename T::propagate_on_container_swap; }
    struct IsPropSwap<T> {
        using type = typename T::propagate_on_container_swap;
    };
    template<class T> struct IsAlwaysEqual {
        using type = typename std::is_empty<T>::type;
    };
    template<class T>
        requires requires { typename T::is_always_equal; }
    struct IsAlwaysEqual<T> {
        using type = typename T::is_always_equal;
    };
    template<class T, class U> struct RebindAlloc {
        using type = typename internal::RepFirst<T, U>::type;
    };
    template<class T, class U>
        requires requires { typename T::template rebind<U>::other; }
    struct RebindAlloc<T, U> {
        using type = typename T::template rebind<U>::other;
    };
}  

template<class Alloc> class AllocatorTraits {
public:
    using allocator_type = Alloc;
    using value_type = typename Alloc::value_type;
    using pointer = typename internal::GetPtr<Alloc, value_type>::type;
    using const_pointer = typename internal::GetConstPtr<Alloc, pointer, value_type>::type;
    using void_pointer = typename internal::GetVoidPtr<Alloc, pointer>::type;
    using const_void_pointer = typename internal::GetConstVoidPtr<Alloc, pointer>::type;
    using difference_type = typename internal::GetDifferenceType<Alloc, pointer>::type;
    using size_type = typename internal::GetSizeType<Alloc, difference_type>::type;
    using propagate_on_container_copy_assignment = typename internal::IsPropCopy<Alloc>::type;
    using propagate_on_container_move_assignment = typename internal::IsPropMove<Alloc>::type;
    using propagate_on_container_swap = typename internal::IsPropSwap<Alloc>::type;
    using is_always_equal = typename internal::IsAlwaysEqual<Alloc>::type;
    template<class U> using rebind_alloc = typename internal::RebindAlloc<Alloc, U>::type;
    template<class U> using rebind_traits = AllocatorTraits<typename internal::RebindAlloc<Alloc, U>::type>;
    [[nodiscard]] static constexpr pointer allocate(Alloc& a, size_type n) { return a.allocate(n); }
    [[nodiscard]] static constexpr pointer allocate(Alloc& a, size_type align, size_type n) { return a.allocate(align, n); }
    [[nodiscard]] static constexpr pointer allocate(Alloc& a, size_type n, const_void_pointer hint) {
        if constexpr (requires { a.allocate(n, hint); }) return a.allocate(n, hint);
        else return a.allocate(n);
    }
    [[nodiscard]] static constexpr pointer allocate(Alloc& a, size_type align, size_type n, const_void_pointer hint) {
        if constexpr (requires { a.allocate(align, n, hint); }) return a.allocate(align, n, hint);
        else return a.allocate(align, n);
    }
    static constexpr void deallocate(Alloc& a, pointer p, size_type n) { a.deallocate(p, n); }
    [[nodiscard]] static constexpr pointer reallocate(Alloc& a, pointer p, size_type prev_size, size_type new_size) {
        if constexpr (requires { a.reallocate(p, prev_size, new_size); }) return a.reallocate(p, prev_size, new_size);
        else {
            deallocate(a, p, prev_size);
            return allocate(a, new_size, p);
        }
    }
    static constexpr size_type max_size(const Alloc& a) noexcept {
        if constexpr (requires { a.max_size(); }) return a.max_size();
        else return std::numeric_limits<size_type>::max() / sizeof(value_type);
    }
    template<class T, class... Args> static constexpr void construct(Alloc& a, T* p, Args&&... args) {
        if constexpr (requires { a.construct(p, std::forward<Args>(args)...); }) a.construct(p, std::forward<Args>(args)...);
        else std::construct_at(p, std::forward<Args>(args)...);
    }
    template<class T> static constexpr void destroy(Alloc& a, T* p) {
        if constexpr (requires { a.destroy(p); }) a.destroy(p);
        else std::destroy_at(p);
    }
    static constexpr Alloc select_on_container_copy_construction(const Alloc& a) {
        if constexpr (requires { a.select_on_container_copy_construction(); }) return a.select_on_container_copy_construction();
        else return a;
    }
};

template<class T> class Allocator {
public:
    using value_type = T;
    using propagate_on_container_move_assignment = std::true_type;
    using size_type = itype::u32;
    using difference_type = itype::i32;
    using is_always_equal = std::true_type;
    constexpr Allocator() noexcept = default;
    constexpr Allocator(const Allocator&) noexcept = default;
    template<class U> constexpr Allocator(const Allocator<U>&) noexcept {}
    constexpr ~Allocator() = default;
    [[nodiscard]] constexpr T* allocate(size_type n) {
        if constexpr (alignof(T) > __STDCPP_DEFAULT_NEW_ALIGNMENT__) return static_cast<T*>(::operator new(sizeof(T) * n, static_cast<std::align_val_t>(alignof(T))));
        else return static_cast<T*>(::operator new(sizeof(T) * n));
    }
    [[nodiscard]] constexpr T* allocate(size_type align, size_type n) { return static_cast<T*>(::operator new(sizeof(T) * n, static_cast<std::align_val_t>(align))); }
    constexpr void deallocate(T* p, [[maybe_unused]] size_type n) {
#ifdef __cpp_sized_deallocation
        if constexpr (alignof(T) > __STDCPP_DEFAULT_NEW_ALIGNMENT__) ::operator delete(p, n, static_cast<std::align_val_t>(alignof(T)));
        else ::operator delete(p, n);
#else
        if constexpr (alignof(T) > __STDCPP_DEFAULT_NEW_ALIGNMENT__) ::operator delete(p, static_cast<std::align_val_t>(alignof(T)));
        else ::operator delete(p);
#endif
    }
    
    constexpr Allocator& operator=(const Allocator&) = default;
    template<class U> friend constexpr bool operator==(const Allocator&, const Allocator<U>&) noexcept { return true; }
};

template<itype::u32 Size> class MemoryPool {
    template<class T> friend class PoolAllocator;
    itype::u32 cnt = 0;
    itype::u32 ref = 0;
    ctype::c8 buf[Size];
public:
    constexpr ~MemoryPool() noexcept(false) {
        if (ref != 0) throw Exception("gsh::MemoryPool::~MemoryPool / There are some gsh::PoolAllocator tied to this object have not yet been destroyed.");
    }
};
template<class T> class PoolAllocator {
    template<class U> friend class PoolAllocator;
    itype::u32* cnt;
    itype::u32* ref;
    ctype::c8* buf;
public:
    using value_type = T;
    using propagate_on_container_copy_assignmant = std::true_type;
    using propagate_on_container_move_assignment = std::true_type;
    using propagate_on_container_swap = std::true_type;
    using size_type = itype::u32;
    using difference_type = itype::i32;
    using is_always_equal = std::false_type;
    constexpr PoolAllocator() noexcept : cnt(nullptr), ref(nullptr), buf(nullptr) {}
    constexpr PoolAllocator(const PoolAllocator& a) : cnt(a.cnt), ref(a.ref), buf(a.buf) { ++*ref; }
    template<class U> constexpr PoolAllocator(const PoolAllocator<U>& a) : cnt(a.cnt), ref(a.ref), buf(a.buf) { ++*ref; }
    template<itype::u32 Size> constexpr PoolAllocator(MemoryPool<Size>& p) noexcept : cnt(&p.cnt), ref(&p.ref), buf(p.buf) { ++*ref; }
    constexpr ~PoolAllocator() noexcept {
        if (ref != nullptr) --*ref;
    }
    constexpr T* allocate(size_type n) {
        auto res = reinterpret_cast<T*>(buf + *cnt);
        *cnt += sizeof(T) * n;
        return res;
    }
    constexpr void deallocate(T*, size_type) {}
    constexpr PoolAllocator& operator=(const PoolAllocator& a) {
        if (ref != nullptr) --*ref;
        cnt = a.cnt, ref = a.ref, buf = a.buf;
        ++*ref;
        return *this;
    }
    template<class U> friend constexpr bool operator==(const PoolAllocator& a, const PoolAllocator<U>& b) { return a.cnt == b.cnt && a.ref == b.ref && a.buf == b.buf; }
};

}  
     

namespace gsh {

template<class T, class Allocator = Allocator<T>>
    requires std::same_as<T, typename AllocatorTraits<Allocator>::value_type> && std::same_as<T, std::remove_cv_t<T>>
class Arr : public ViewInterface<Arr<T, Allocator>, T> {
    using traits = AllocatorTraits<Allocator>;
public:
    using reference = T&;
    using const_reference = const T&;
    using iterator = T*;
    using const_iterator = const T*;
    using size_type = itype::u32;
    using difference_type = itype::i32;
    using value_type = T;
    using allocator_type = Allocator;
    using pointer = typename traits::pointer;
    using const_pointer = typename traits::const_pointer;
    using reverse_iterator = std::reverse_iterator<iterator>;
    using const_reverse_iterator = std::reverse_iterator<const_iterator>;
private:
    [[no_unique_address]] allocator_type alloc;
    pointer ptr = nullptr;
    size_type len = 0;
public:
    constexpr Arr() noexcept(noexcept(Allocator())) : Arr(Allocator()) {}
    constexpr explicit Arr(const allocator_type& a) noexcept : alloc(a) {}
    constexpr explicit Arr(size_type n, const Allocator& a = Allocator()) : alloc(a) {
        if (n == 0) [[unlikely]]
            return;
        ptr = traits::allocate(alloc, n);
        len = n;
        if constexpr (!std::is_trivially_default_constructible_v<value_type>)
            for (size_type i = 0; i != n; ++i) traits::construct(alloc, ptr + i);
        else std::memset(ptr, 0, sizeof(value_type) * n);
    }
    constexpr explicit Arr(const size_type n, const value_type& value, const allocator_type& a = Allocator()) : alloc(a) {
        if (n == 0) [[unlikely]]
            return;
        ptr = traits::allocate(alloc, n);
        len = n;
        for (size_type i = 0; i != n; ++i) traits::construct(alloc, ptr + i, value);
    }
    template<std::input_iterator InputIter> constexpr Arr(const InputIter first, const InputIter last, const allocator_type& a = Allocator()) : alloc(a) {
        const size_type n = std::distance(first, last);
        if (n == 0) [[unlikely]]
            return;
        ptr = traits::allocate(alloc, n);
        len = n;
        size_type i = 0;
        for (InputIter itr = first; i != n; ++itr, ++i) traits::construct(alloc, ptr + i, *itr);
    }
    constexpr Arr(const Arr& x) : Arr(x, traits::select_on_container_copy_construction(x.alloc)) {}
    constexpr Arr(Arr&& x) noexcept : alloc(std::move(x.alloc)), ptr(x.ptr), len(x.len) { x.ptr = nullptr, x.len = 0; }
    constexpr Arr(const Arr& x, const allocator_type& a) : alloc(a), len(x.len) {
        if (len == 0) [[unlikely]]
            return;
        ptr = traits::allocate(alloc, len);
        for (size_type i = 0; i != len; ++i) traits::construct(alloc, ptr + i, *(x.ptr + i));
    }
    constexpr Arr(Arr&& x, const allocator_type& a) : alloc(a) {
        if (traits::is_always_equal || x.get_allocator() == a) {
            ptr = x.ptr, len = x.len;
            x.ptr = nullptr, x.len = 0;
        } else {
            if (x.len == 0) [[unlikely]]
                return;
            len = x.len;
            ptr = traits::allocate(alloc, len);
            for (size_type i = 0; i != len; ++i) traits::construct(alloc, ptr + i, std::move(*(x.ptr + i)));
            traits::deallocate(x.alloc, x.ptr, x.len);
            x.ptr = nullptr, x.len = 0;
        }
    }
    constexpr Arr(std::initializer_list<value_type> il, const allocator_type& a = Allocator()) : Arr(il.begin(), il.end(), a) {}
    template<Rangeof<value_type> R> constexpr Arr(R&& r, const allocator_type& a = Allocator()) : Arr(RangeTraits<R>::fbegin(r), RangeTraits<R>::fend(r), a) {}
    constexpr ~Arr() {
        if (len != 0) {
            if constexpr (!std::is_trivially_destructible_v<value_type>)
                for (size_type i = 0; i != len; ++i) traits::destroy(alloc, ptr + i);
            traits::deallocate(alloc, ptr, len);
        }
    }
    constexpr Arr& operator=(const Arr& x) {
        if (&x == this) return *this;
        if constexpr (!std::is_trivially_destructible_v<value_type>)
            for (size_type i = 0; i != len; ++i) traits::destroy(alloc, ptr + i);
        if (traits::propagate_on_container_copy_assignment::value || len != x.len) {
            if (len != 0) traits::deallocate(alloc, ptr, len);
            if constexpr (traits::propagate_on_container_copy_assignment::value) alloc = x.alloc;
            ptr = traits::allocate(alloc, x.len);
        }
        len = x.len;
        for (size_type i = 0; i != len; ++i) *(ptr + i) = *(x.ptr + i);
        return *this;
    }
    constexpr Arr& operator=(Arr&& x) noexcept(traits::propagate_on_container_move_assignment::value || traits::is_always_equal::value) {
        if (&x == this) return *this;
        if (len != 0) {
            if constexpr (!std::is_trivially_destructible_v<value_type>)
                for (size_type i = 0; i != len; ++i) traits::destroy(alloc, ptr + i);
            traits::deallocate(alloc, ptr, len);
        }
        if constexpr (traits::propagate_on_container_move_assignment::value) alloc = std::move(x.alloc);
        ptr = x.ptr, len = x.len;
        x.ptr = nullptr, x.len = 0;
        return *this;
    }
    constexpr Arr& operator=(std::initializer_list<value_type> init) {
        assign(init.begin(), init.end());
        return *this;
    }
    constexpr iterator begin() noexcept { return ptr; }
    constexpr const_iterator begin() const noexcept { return ptr; }
    constexpr iterator end() noexcept { return ptr + len; }
    constexpr const_iterator end() const noexcept { return ptr + len; }
    constexpr const_iterator cbegin() const noexcept { return ptr; }
    constexpr const_iterator cend() const noexcept { return ptr + len; }
    constexpr reverse_iterator rbegin() noexcept { return reverse_iterator(ptr + len); }
    constexpr const_reverse_iterator rbegin() const noexcept { return const_reverse_iterator(ptr + len); }
    constexpr reverse_iterator rend() noexcept { return reverse_iterator(ptr); }
    constexpr const_reverse_iterator rend() const noexcept { return const_reverse_iterator(ptr); }
    constexpr const_reverse_iterator crbegin() const noexcept { return const_reverse_iterator(ptr + len); }
    constexpr const_reverse_iterator crend() const noexcept { return const_reverse_iterator(ptr); }
    constexpr size_type size() const noexcept { return len; }
    constexpr size_type max_size() const noexcept {
        const auto tmp = traits::max_size(alloc);
        return tmp < 2147483647 ? tmp : 2147483647;
    }
    constexpr void resize(const size_type sz) {
        if (len == sz) return;
        if (sz == 0) {
            clear();
            return;
        }
        const pointer new_ptr = traits::allocate(alloc, sz);
        const size_type mn = len < sz ? len : sz;
        if (len != 0) {
            for (size_type i = 0; i != mn; ++i) traits::construct(alloc, new_ptr + i, std::move(*(ptr + i)));
            if constexpr (!std::is_trivially_destructible_v<value_type>)
                for (size_type i = 0; i != len; ++i) traits::destroy(alloc, ptr + i);
            traits::deallocate(alloc, ptr, len);
        }
        ptr = new_ptr;
        if constexpr (!std::is_trivially_default_constructible_v<value_type>)
            for (size_type i = len; i < sz; ++i) traits::construct(alloc, ptr + i);
        else if (len < sz) std::memset(ptr + len, 0, sizeof(value_type) * (sz - len));
        len = sz;
    }
    constexpr void resize(const size_type sz, const value_type& c) {
        if (len == sz) return;
        if (sz == 0) {
            clear();
            return;
        }
        const pointer new_ptr = traits::allocate(alloc, sz);
        const size_type mn = len < sz ? len : sz;
        if (len != 0) {
            for (size_type i = 0; i != mn; ++i) traits::construct(alloc, new_ptr + i, std::move(*(ptr + i)));
            if constexpr (!std::is_trivially_destructible_v<value_type>)
                for (size_type i = 0; i != len; ++i) traits::destroy(alloc, ptr + i);
            traits::deallocate(alloc, ptr, len);
        }
        ptr = new_ptr;
        for (size_type i = len; i < sz; ++i) traits::construct(alloc, ptr + i, c);
        len = sz;
    }
    [[nodiscard]] constexpr bool empty() const noexcept { return len == 0; }
    __attribute__((always_inline)) constexpr reference operator[](const size_type n) {
#ifndef NDEBUG
        if (n >= len) [[unlikely]]
            throw gsh::Exception("gsh::Arr::operator[] / The index is out of range. ( n=", n, ", size=", len, " )");
#endif
        return *(ptr + n);
    }
    __attribute__((always_inline)) constexpr const_reference operator[](const size_type n) const {
#ifndef NDEBUG
        if (n >= len) [[unlikely]]
            throw gsh::Exception("gsh::Arr::operator[] / The index is out of range. ( n=", n, ", size=", len, " )");
#endif
        return *(ptr + n);
    }
    __attribute__((always_inline)) constexpr reference at(const size_type n) {
        if (n >= len) [[unlikely]]
            throw gsh::Exception("gsh::Arr::at / The index is out of range. ( n=", n, ", size=", len, " )");
        return *(ptr + n);
    }
    __attribute__((always_inline)) constexpr const_reference at(const size_type n) const {
        if (n >= len) [[unlikely]]
            throw gsh::Exception("gsh::Arr::at / The index is out of range. ( n=", n, ", size=", len, " )");
        return *(ptr + n);
    }
    __attribute__((always_inline)) constexpr reference at_unchecked(const size_type n) { return *(ptr + n); }
    constexpr const_reference at_unchecked(const size_type n) const { return *(ptr + n); }
    constexpr pointer data() noexcept { return ptr; }
    constexpr const_pointer data() const noexcept { return ptr; }
    constexpr reference front() { return *ptr; }
    constexpr const_reference front() const { return *ptr; }
    constexpr reference back() { return *(ptr + len - 1); }
    constexpr const_reference back() const { return *(ptr + len - 1); }
    template<std::input_iterator InputIter> constexpr void assign(const InputIter first, const InputIter last) {
        const size_type n = std::distance(first, last);
        if (n == 0) {
            clear();
        } else if (len == n) {
            InputIter itr = first;
            for (size_type i = 0; i != len; ++itr, ++i) *(ptr + i) = *itr;
        } else {
            if constexpr (!std::is_trivially_destructible_v<value_type>)
                for (size_type i = 0; i != len; ++i) traits::destroy(alloc, ptr + i);
            traits::deallocate(alloc, ptr, len);
            ptr = traits::allocate(alloc, n);
            len = n;
            InputIter itr = first;
            for (size_type i = 0; i != n; ++itr, ++i) traits::construct(alloc, ptr + i, *itr);
        }
    }
    constexpr void assign(const size_type n, const value_type& t) {
        if (n == 0) {
            clear();
        } else if (len == n) {
            for (size_type i = 0; i != len; ++i) *(ptr + i) = t;
        } else if (n != 0) {
            if constexpr (!std::is_trivially_destructible_v<value_type>)
                for (size_type i = 0; i != len; ++i) traits::destroy(alloc, ptr + i);
            traits::deallocate(alloc, ptr, len);
            ptr = traits::allocate(alloc, n);
            len = n;
            for (size_type i = 0; i != n; ++i) traits::construct(alloc, ptr + i, t);
        }
    }
    constexpr void assign(std::initializer_list<value_type> il) { assign(il.begin(), il.end()); }
    constexpr void swap(Arr& x) noexcept(traits::propagate_on_container_swap::value || traits::is_always_equal::value) {
        using std::swap;
        swap(ptr, x.ptr);
        swap(len, x.len);
        if constexpr (traits::propagate_on_container_swap::value) swap(alloc, x.alloc);
    }
    constexpr void clear() {
        if (len != 0) {
            if constexpr (!std::is_trivially_destructible_v<value_type>)
                for (size_type i = 0; i != len; ++i) traits::destroy(alloc, ptr + i);
            traits::deallocate(alloc, ptr, len);
            ptr = nullptr, len = 0;
        }
    }
    constexpr allocator_type get_allocator() const noexcept { return alloc; }
    friend constexpr bool operator==(const Arr& x, const Arr& y) {
        if (x.len != y.len) return false;
        bool res = true;
        for (size_type i = 0; i != x.len;) {
            const bool f = *(x.ptr + i) == *(y.ptr + i);
            res &= f;
            i = f ? i + 1 : x.len;
        }
        return res;
    }
    friend constexpr auto operator<=>(const Arr& x, const Arr& y) { return std::lexicographical_compare_three_way(x.begin(), x.end(), y.begin(), y.end()); }
    friend constexpr void swap(Arr& x, Arr& y) noexcept(noexcept(x.swap(y))) { x.swap(y); }
};
template<std::input_iterator InputIter, class Alloc = Allocator<typename std::iterator_traits<InputIter>::value_type>> Arr(InputIter, InputIter, Alloc = Alloc()) -> Arr<typename std::iterator_traits<InputIter>::value_type, Alloc>;
template<Range R, class Alloc = Allocator<typename RangeTraits<R>::value_type>> Arr(R, Alloc = Alloc()) -> Arr<typename RangeTraits<R>::value_type, Alloc>;

template<class T>
    requires std::same_as<T, std::remove_cv_t<T>>
class ArrInitTag {};
template<class T = void> constexpr ArrInitTag<T> ArrInit;
class ArrNoInitTag {};
constexpr ArrNoInitTag ArrNoInit;

template<class T, itype::u32 N>
    requires std::same_as<T, std::remove_cv_t<T>>
class StaticArr : public ViewInterface<StaticArr<T, N>, T> {
    union {
        T elems[(N == 0 ? 1 : N)];
    };
public:
    using reference = T&;
    using const_reference = const T&;
    using iterator = T*;
    using const_iterator = const T*;
    using size_type = itype::u32;
    using difference_type = itype::i32;
    using value_type = T;
    using pointer = T*;
    using const_pointer = const T*;
    using reverse_iterator = std::reverse_iterator<iterator>;
    using const_reverse_iterator = std::reverse_iterator<const_iterator>;
    constexpr StaticArr() noexcept(noexcept(value_type{})) : elems{} {}
    constexpr StaticArr(ArrNoInitTag) noexcept {}
    template<class U, class... Args> constexpr StaticArr(ArrInitTag<U>, Args&&... args) : elems{ static_cast<U>(std::forward<Args>(args))... } {
        static_assert(std::is_same_v<T, U>, "gsh::StaticArr::StaticArr / The type specified in gsh::ArrInitTag is different from value_type.");
        static_assert(sizeof...(Args) <= N, "gsh::StaticArr::StaticArr / The number of arguments is greater than the length of the array.");
    }
    template<class... Args> constexpr StaticArr(ArrInitTag<void>, Args&&... args) : elems{ static_cast<T>(std::forward<Args>(args))... } { static_assert(sizeof...(Args) <= N, "gsh::StaticArr::StaticArr / The number of arguments is greater than the length of the array."); }
    constexpr explicit StaticArr(const value_type& value) {
        for (itype::u32 i = 0; i != N; ++i) ConstructAt(elems + i, value);
    }
    template<std::input_iterator InputIter> constexpr explicit StaticArr(InputIter first) {
        for (itype::u32 i = 0; i != N; ++first, ++i) ConstructAt(elems + i, *first);
    }
    template<std::input_iterator InputIter> constexpr StaticArr(InputIter first, const InputIter last) {
        const itype::u32 n = std::distance(first, last);
        if (n != N) throw gsh::Exception("gsh::StaticArr::StaticArr / The size of the given range differs from the size of the array.");
        for (itype::u32 i = 0; i != N; ++first, ++i) ConstructAt(elems + i, *first);
    }
    template<Rangeof<value_type> R> constexpr StaticArr(R&& r) : StaticArr(RangeTraits<R>::fbegin(r), RangeTraits<R>::fend(r)) {}
    constexpr StaticArr(const value_type (&a)[N]) {
        for (itype::u32 i = 0; i != N; ++i) ConstructAt(elems + i, a[i]);
    }
    constexpr StaticArr(value_type (&&a)[N]) {
        for (itype::u32 i = 0; i != N; ++i) ConstructAt(elems + i, std::move(a[i]));
    }
    constexpr StaticArr(const StaticArr& x) {
        for (itype::u32 i = 0; i != N; ++i) ConstructAt(elems + i, x.elems[i]);
    }
    constexpr StaticArr(StaticArr&& y) {
        for (itype::u32 i = 0; i != N; ++i) ConstructAt(elems + i, std::move(y.elems[i]));
    }
    constexpr StaticArr(std::initializer_list<value_type> il) : StaticArr(il.begin(), il.end()) {}
    constexpr ~StaticArr() noexcept {
        if constexpr (!std::is_trivially_destructible_v<value_type>)
            for (itype::u32 i = 0; i != N; ++i) DestroyAt(elems + i);
    }
    constexpr StaticArr& operator=(const StaticArr& x) {
        for (itype::u32 i = 0; i != N; ++i) elems[i] = x.elems[i];
        return *this;
    }
    constexpr StaticArr& operator=(StaticArr&& x) noexcept {
        for (itype::u32 i = 0; i != N; ++i) elems[i] = std::move(x.elems[i]);
        return *this;
    }
    constexpr StaticArr& operator=(std::initializer_list<value_type> init) {
        assign(init.begin(), init.end());
        return *this;
    }
    constexpr iterator begin() noexcept { return elems; }
    constexpr const_iterator begin() const noexcept { return elems; }
    constexpr iterator end() noexcept { return elems + N; }
    constexpr const_iterator end() const noexcept { return elems + N; }
    constexpr const_iterator cbegin() const noexcept { return elems; }
    constexpr const_iterator cend() const noexcept { return elems + N; }
    constexpr reverse_iterator rbegin() noexcept { return reverse_iterator(elems + N); }
    constexpr const_reverse_iterator rbegin() const noexcept { return const_reverse_iterator(elems + N); }
    constexpr reverse_iterator rend() noexcept { return reverse_iterator(elems); }
    constexpr const_reverse_iterator rend() const noexcept { return const_reverse_iterator(elems); }
    constexpr const_reverse_iterator crbegin() const noexcept { return const_reverse_iterator(elems + N); }
    constexpr const_reverse_iterator crend() const noexcept { return const_reverse_iterator(elems); }
    constexpr size_type size() const noexcept { return N; }
    constexpr size_type max_size() const noexcept { return N; }
    [[nodiscard]] constexpr bool empty() const noexcept { return N != 0; }
    __attribute__((always_inline)) constexpr reference operator[](const size_type n) {
#ifndef NDEBUG
        if (n >= N) [[unlikely]]
            throw gsh::Exception("gsh::StaticArr::operator[] / The index is out of range. ( n=", n, ", size=", N, " )");
#endif
        return elems[n];
    }
    __attribute__((always_inline)) constexpr const_reference operator[](const size_type n) const {
#ifndef NDEBUG
        if (n >= N) [[unlikely]]
            throw gsh::Exception("gsh::StaticArr::operator[] / The index is out of range. ( n=", n, ", size=", N, " )");
#endif
        return elems[n];
    }
    __attribute__((always_inline)) constexpr reference at(const size_type n) {
        if (n >= N) [[unlikely]]
            throw gsh::Exception("gsh::StaticArr::at / The index is out of range. ( n=", n, ", size=", N, " )");
        return elems[n];
    }
    __attribute__((always_inline)) constexpr const_reference at(const size_type n) const {
        if (n >= N) [[unlikely]]
            throw gsh::Exception("gsh::StaticArr::at / The index is out of range. ( n=", n, ", size=", N, " )");
        return elems[n];
    }
    __attribute__((always_inline)) constexpr reference at_unchecked(const size_type n) { return elems[n]; }
    constexpr const_reference at_unchecked(const size_type n) const { return elems[n]; }
    constexpr pointer data() noexcept { return elems; }
    constexpr const_pointer data() const noexcept { return elems; }
    constexpr reference front() { return elems[0]; }
    constexpr const_reference front() const { return elems[0]; }
    constexpr reference back() { return elems[N - 1]; }
    constexpr const_reference back() const { return elems[N - 1]; }
    template<std::input_iterator InputIter> constexpr void assign(InputIter first) {
        for (itype::u32 i = 0; i != N; ++first, ++i) elems[i] = *first;
    }
    template<std::input_iterator InputIter> constexpr void assign(InputIter first, const InputIter last) {
        const itype::u32 n = std::distance(first, last);
        if (n != N) throw gsh::Exception("gsh::StaticArr::assign / The size of the given range differs from the size of the array.");
        for (itype::u32 i = 0; i != N; ++first, ++i) elems[i] = *first;
    }
    constexpr void assign(const value_type& value) {
        for (itype::u32 i = 0; i != N; ++i) elems[i] = value;
    }
    constexpr void assign(std::initializer_list<value_type> il) { assign(il.begin(), il.end()); }
    constexpr void swap(StaticArr& x) {
        using std::swap;
        for (itype::u32 i = 0; i != N; ++i) swap(elems[i], x.elems[i]);
    }
    friend constexpr bool operator==(const StaticArr& x, const StaticArr& y) {
        bool res = true;
        for (size_type i = 0; i != N;) {
            const bool f = x.elems[i] == y.elems[i];
            res &= f;
            i = f ? i + 1 : N;
        }
        return res;
    }
    friend constexpr auto operator<=>(const StaticArr& x, const StaticArr& y) { return std::lexicographical_compare_three_way(x.begin(), x.end(), y.begin(), y.end()); }
    friend constexpr void swap(StaticArr& x, StaticArr& y) noexcept(noexcept(x.swap(y))) { x.swap(y); }
};
template<class U, class... Args> StaticArr(ArrInitTag<U>, Args...) -> StaticArr<std::conditional_t<std::is_void_v<U>, std::common_type_t<Args...>, U>, sizeof...(Args)>;

}  

namespace std {
template<class T, gsh::itype::u32 N> struct tuple_size<gsh::StaticArr<T, N>> : integral_constant<size_t, N> {};
template<std::size_t M, class T, gsh::itype::u32 N> struct tuple_element<M, gsh::StaticArr<T, N>> {
    static_assert(M < N, "std::tuple_element<gsh::StaticArr<T, N>> / The index is out of range.");
    using type = T;
};
}  

namespace gsh {
template<std::size_t M, class T, itype::u32 N> const T& get(const StaticArr<T, N>& a) {
    static_assert(M < N, "gsh::get(gsh::StaticArr<T, N>) / The index is out of range.");
    return a[M];
}
template<std::size_t M, class T, itype::u32 N> T& get(StaticArr<T, N>& a) {
    static_assert(M < N, "gsh::get(gsh::StaticArr<T, N>) / The index is out of range.");
    return a[M];
}
template<std::size_t M, class T, itype::u32 N> T&& get(StaticArr<T, N>&& a) {
    static_assert(M < N, "gsh::get(gsh::StaticArr<T, N>) / The index is out of range.");
    return std::move(a[M]);
}
}  


namespace gsh {


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
        
        constexpr static itype::u64 table[147] = {0x2001002010213u,0x200001000020001u,0x20100010000u,0x10000200000010u,0x200000001u,0x20000000010u,0x200000000010000u,0x1200000000u,0x20000u,0x2000002000201u,0x1000000201u,0x20002100000u,0x10000000010000u,0x1000000000200u,0x2000000000010u,0x2010002u,0x100001u,0x20002u,0x210u,0x1000200000200u,0x110000u,0x2000000u,0x201001100000000u,0x2000100000000u,0x2000002000000u,0x201u,
        0x20002u,0x10001000000002u,0x200000000000000u,0x2100000u,0x10012u,0x200020100000000u,0x20100000000u,0x2000000000010u,0x1000200100200u,0u,0x10001000000003u,0x1200000000u,0x10000000000000u,0x2000002000010u,0x21000000001u,0x20100000000u,0x10000000010000u,0x200000200000000u,0u,0x2001000010200u,0x1000020000u,0x20000u,0x12000000000000u,0x1000200000201u,0x2020000100000u,0x10000002010000u,0x1001000000000u,0x20000u,
        0x2000000u,0x1u,0x10000000130000u,0x2u,0x201000300000200u,0x2000000100010u,0x2000010u,0x200001000000001u,0x100000002u,0x2000000000000u,0x1000000000201u,0x2010000u,0x10000000000002u,0x200020100000000u,0x100020010u,0x10u,0x200u,0x20100100000u,0x1000010000u,0x201000200020200u,0x2000000u,0x2000000000002u,0x21000000000u,0x20000000000u,0x13000000000010u,0x1u,0x20000000002u,0x10000002010001u,0x200000200020000u,
        0x100020000u,0x2000200000000u,0x1000000000u,0x120000u,0x211000000000000u,0x1000200000200u,0x100000u,0x2010201u,0x1000020001u,0x10020000020000u,0u,0x200000001u,0x100010u,0x200000000000002u,0x201001200000000u,0x100020000u,0x2000210u,0x1000000201u,0x10000100100000u,0x200000002u,0x1000000000200u,0x2000000000010u,0x2000000000012u,0x200000000000000u,0x20100020000u,0x10000000000010u,0x1000000000200u,0x20000110000u,
        0x10000u,0x201000200000000u,0x2000100000000u,0x3000000000000u,0x1000100000u,0x20000000000u,0x10001000010002u,0x200000000020000u,0x2000000u,0x2010010u,0x200000000000001u,0x20100020000u,0x203000000000000u,0x200100000u,0x100000u,0x10001002000001u,0x1001200000000u,0u,0x2000000u,0x1000000201u,0x20000020000u,0x200000000010002u,0x200000000u,0x100000u,0x212u,0x200001000000000u,0x100030000u,0x200000010u,0x1000000000201u,
        0x2000000100000u,0x2000002u,0x1000000000000u,0x20000u,0x2000000000011u,0u,0u};
        
        constexpr static bool calc(const itype::u16 x) { return (table[x / 64] >> (x % 64)) & 1; }
    };
}  
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

template<class T, class... Args> constexpr auto GCD(T x, Args... y) {
    return GCD(x, GCD(y...));
}

template<class T, class U> constexpr auto LCM(T x, U y) {
    return static_cast<std::common_type_t<T, U>>(x < 0 ? -x : x) / GCD(x, y) * static_cast<std::common_type_t<T, U>>(y < 0 ? -y : y);
}

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
}  
using BinCoeffTable32 = internal::BinCoeffTable<internal::DynamicModint32Impl>;
using BinCoeffTable64 = internal::BinCoeffTable<internal::DynamicModint64Impl>;
template<itype::u64 mod = 998244353> using BinCoeffTableStaticMod = internal::BinCoeffTable<internal::StaticModintImpl<mod>>;

}  

#include <algorithm>
#include <bit>


#include <iterator>           
#include <algorithm>          
#include <initializer_list>   
#include <type_traits>        
#include <cstring>            
#include <utility>            
    
  
      
     

namespace gsh {

template<class T, class Allocator = Allocator<T>>
    requires std::is_same_v<T, typename AllocatorTraits<Allocator>::value_type> && (!std::is_const_v<T>)
class Vec : public ViewInterface<Vec<T, Allocator>, T> {
    using traits = AllocatorTraits<Allocator>;
public:
    using reference = T&;
    using const_reference = const T&;
    using iterator = T*;
    using const_iterator = const T*;
    using size_type = itype::u32;
    using difference_type = itype::i32;
    using value_type = T;
    using allocator_type = Allocator;
    using pointer = typename traits::pointer;
    using const_pointer = typename traits::const_pointer;
    using reverse_iterator = std::reverse_iterator<iterator>;
    using const_reverse_iterator = std::reverse_iterator<const_iterator>;
private:
    [[no_unique_address]] allocator_type alloc;
    pointer ptr = nullptr;
    size_type len = 0, cap = 0;
public:
    constexpr Vec() noexcept(noexcept(Allocator())) : Vec(Allocator()) {}
    constexpr explicit Vec(const allocator_type& a) noexcept : alloc(a) {}
    constexpr explicit Vec(size_type n, const Allocator& a = Allocator()) : alloc(a) {
        if (n == 0) [[unlikely]]
            return;
        ptr = traits::allocate(alloc, n);
        len = n, cap = n;
        if constexpr (!std::is_trivially_default_constructible_v<value_type>)
            for (size_type i = 0; i != n; ++i) traits::construct(alloc, ptr + i);
        else std::memset(ptr, 0, sizeof(value_type) * n);
    }
    constexpr explicit Vec(const size_type n, const value_type& value, const allocator_type& a = Allocator()) : alloc(a) {
        if (n == 0) [[unlikely]]
            return;
        ptr = traits::allocate(alloc, n);
        len = n, cap = n;
        for (size_type i = 0; i != n; ++i) traits::construct(alloc, ptr + i, value);
    }
    template<std::input_iterator InputIter> constexpr Vec(const InputIter first, const InputIter last, const allocator_type& a = Allocator()) : alloc(a) {
        const size_type n = std::distance(first, last);
        if (n == 0) [[unlikely]]
            return;
        ptr = traits::allocate(alloc, n);
        len = n, cap = n;
        size_type i = 0;
        for (InputIter itr = first; i != n; ++itr, ++i) traits::construct(alloc, ptr + i, *itr);
    }
    constexpr Vec(const Vec& x) : Vec(x, traits::select_on_container_copy_construction(x.alloc)) {}
    constexpr Vec(Vec&& x) noexcept : alloc(std::move(x.alloc)), ptr(x.ptr), len(x.len), cap(x.cap) { x.ptr = nullptr, x.len = 0, x.cap = 0; }
    constexpr Vec(const Vec& x, const allocator_type& a) : alloc(a), len(x.len), cap(x.len) {
        if (len == 0) [[unlikely]]
            return;
        ptr = traits::allocate(alloc, cap);
        for (size_type i = 0; i != len; ++i) traits::construct(alloc, ptr + i, *(x.ptr + i));
    }
    constexpr Vec(Vec&& x, const allocator_type& a) : alloc(a) {
        if (traits::is_always_equal || x.get_allocator() == a) {
            ptr = x.ptr, len = x.len, cap = x.cap;
            x.ptr = nullptr, x.len = 0, x.cap = 0;
        } else {
            if (x.len == 0) [[unlikely]]
                return;
            len = x.len, cap = x.cap;
            ptr = traits::allocate(alloc, len);
            for (size_type i = 0; i != len; ++i) traits::construct(alloc, ptr + i, std::move(*(x.ptr + i)));
            traits::deallocate(x.alloc, x.ptr, x.cap);
            x.ptr = nullptr, x.len = 0, x.cap = 0;
        }
    }
    constexpr Vec(std::initializer_list<value_type> il, const allocator_type& a = Allocator()) : Vec(il.begin(), il.end(), a) {}
    template<Rangeof<value_type> R> constexpr Vec(R&& r, const allocator_type& a = Allocator()) : Vec(RangeTraits<R>::fbegin(r), RangeTraits<R>::fend(r), a) {}
    constexpr ~Vec() {
        if (cap != 0) {
            if constexpr (!std::is_trivially_destructible_v<value_type>)
                for (size_type i = 0; i != len; ++i) traits::destroy(alloc, ptr + i);
            traits::deallocate(alloc, ptr, cap);
        }
    }
    constexpr Vec& operator=(const Vec& x) {
        if (&x == this) return *this;
        if constexpr (!std::is_trivially_destructible_v<value_type>)
            for (size_type i = 0; i != len; ++i) traits::destroy(alloc, ptr + i);
        if (traits::propagate_on_container_copy_assignment::value || cap < x.len) {
            if (cap != 0) traits::deallocate(alloc, ptr, cap);
            if constexpr (traits::propagate_on_container_copy_assignment::value) alloc = x.alloc;
            cap = x.len;
            ptr = traits::allocate(alloc, cap);
        }
        len = x.len;
        for (size_type i = 0; i != len; ++i) *(ptr + i) = *(x.ptr + i);
        return *this;
    }
    constexpr Vec& operator=(Vec&& x) noexcept(traits::propagate_on_container_move_assignment::value || traits::is_always_equal::value) {
        if (&x == this) return *this;
        if (cap != 0) {
            if constexpr (!std::is_trivially_destructible_v<value_type>)
                for (size_type i = 0; i != len; ++i) traits::destroy(alloc, ptr + i);
            traits::deallocate(alloc, ptr, cap);
        }
        if constexpr (traits::propagate_on_container_move_assignment::value) alloc = std::move(x.alloc);
        ptr = x.ptr, len = x.len, cap = x.cap;
        x.ptr = nullptr, x.len = 0, x.cap = 0;
        return *this;
    }
    constexpr Vec& operator=(std::initializer_list<value_type> init) {
        assign(init.begin(), init.end());
        return *this;
    }
    constexpr iterator begin() noexcept { return ptr; }
    constexpr const_iterator begin() const noexcept { return ptr; }
    constexpr iterator end() noexcept { return ptr + len; }
    constexpr const_iterator end() const noexcept { return ptr + len; }
    constexpr const_iterator cbegin() const noexcept { return ptr; }
    constexpr const_iterator cend() const noexcept { return ptr + len; }
    constexpr reverse_iterator rbegin() noexcept { return reverse_iterator(ptr + len); }
    constexpr const_reverse_iterator rbegin() const noexcept { return const_reverse_iterator(ptr + len); }
    constexpr reverse_iterator rend() noexcept { return reverse_iterator(ptr); }
    constexpr const_reverse_iterator rend() const noexcept { return const_reverse_iterator(ptr); }
    constexpr const_reverse_iterator crbegin() const noexcept { return const_reverse_iterator(ptr + len); }
    constexpr const_reverse_iterator crend() const noexcept { return const_reverse_iterator(ptr); }
    constexpr size_type size() const noexcept { return len; }
    constexpr size_type max_size() const noexcept {
        const auto tmp = traits::max_size(alloc);
        return tmp < 2147483647 ? tmp : 2147483647;
    }
    constexpr void resize(const size_type sz) {
        if (cap < sz) {
            const pointer new_ptr = traits::allocate(alloc, sz);
            if (cap != 0) {
                for (size_type i = 0; i != len; ++i) traits::construct(alloc, new_ptr + i, std::move(*(ptr + i)));
                if constexpr (!std::is_trivially_destructible_v<value_type>)
                    for (size_type i = 0; i != len; ++i) traits::destroy(alloc, ptr + i);
                traits::deallocate(alloc, ptr, cap);
            }
            ptr = new_ptr;
            if constexpr (!std::is_trivially_default_constructible_v<value_type>)
                for (size_type i = len; i != sz; ++i) traits::construct(alloc, ptr + i);
            else std::memset(ptr + len, 0, sizeof(value_type) * (sz - len));
            len = sz, cap = sz;
        } else if (len < sz) {
            if constexpr (!std::is_trivially_default_constructible_v<value_type>)
                for (size_type i = len; i != sz; ++i) traits::construct(alloc, ptr + i);
            else std::memset(ptr + len, 0, sizeof(value_type) * (sz - len));
            len = sz;
        } else {
            if constexpr (!std::is_trivially_destructible_v<value_type>)
                for (size_type i = sz; i != len; ++i) traits::destroy(alloc, ptr + i);
            len = sz;
        }
    }
    constexpr void resize(const size_type sz, const value_type& c) {
        if (cap < sz) {
            const pointer new_ptr = traits::allocate(sz);
            if (cap != 0) {
                for (size_type i = 0; i != len; ++i) traits::construct(alloc, new_ptr + i, std::move(*(ptr + i)));
                if constexpr (!std::is_trivially_destructible_v<value_type>)
                    for (size_type i = 0; i != len; ++i) traits::destroy(alloc, ptr + i);
                traits::deallocate(alloc, ptr, cap);
            }
            ptr = new_ptr;
            for (size_type i = len; i != sz; ++i) traits::construct(alloc, *(ptr + i), c);
            len = sz, cap = sz;
        } else if (len < sz) {
            for (size_type i = len; i != sz; ++i) traits::construct(alloc, *(ptr + i), c);
            len = sz;
        } else {
            if constexpr (!std::is_trivially_destructible_v<value_type>)
                for (size_type i = sz; i != len; ++i) traits::destroy(alloc, ptr + i);
            len = sz;
        }
    }
    constexpr size_type capacity() const noexcept { return cap; }
    [[nodiscard]] constexpr bool empty() const noexcept { return len == 0; }
    constexpr void reserve(const size_type n) {
        if (n > cap) {
            const pointer new_ptr = traits::allocate(alloc, n);
            if (cap != 0) {
                if constexpr (std::is_trivially_move_constructible_v<value_type>) {
                    for (size_type i = 0; i != len; ++i) new_ptr[i] = ptr[i];
                } else {
                    for (size_type i = 0; i != len; ++i) traits::construct(alloc, new_ptr + i, std::move(*(ptr + i)));
                }
                if constexpr (!std::is_trivially_destructible_v<value_type>)
                    for (size_type i = 0; i != len; ++i) traits::destroy(alloc, ptr + i);
                traits::deallocate(alloc, ptr, cap);
            }
            ptr = new_ptr, cap = n;
        }
    }
    constexpr void shrink_to_fit() {
        if (len == 0) {
            if (cap != 0) traits::deallocate(alloc, ptr, cap);
            ptr = nullptr, cap = 0;
            return;
        }
        if (len != cap) {
            const pointer new_ptr = traits::allocate(alloc, len);
            for (size_type i = 0; i != len; ++i) traits::construct(alloc, new_ptr + i, std::move(*(ptr + i)));
            if constexpr (!std::is_trivially_destructible_v<value_type>)
                for (size_type i = 0; i != len; ++i) traits::destroy(alloc, ptr + i);
            traits::deallocate(alloc, ptr, cap);
            ptr = new_ptr, cap = len;
        }
    }
    __attribute__((always_inline)) constexpr reference operator[](const size_type n) {
#ifndef NDEBUG
        if (n >= len) [[unlikely]]
            throw gsh::Exception("gsh::Vec::operator[] / The index is out of range. ( n=", n, ", size=", len, " )");
#endif
        return *(ptr + n);
    }
    __attribute__((always_inline)) constexpr const_reference operator[](const size_type n) const {
#ifndef NDEBUG
        if (n >= len) [[unlikely]]
            throw gsh::Exception("gsh::Vec::operator[] / The index is out of range. ( n=", n, ", size=", len, " )");
#endif
        return *(ptr + n);
    }
    __attribute__((always_inline)) constexpr reference at(const size_type n) {
        if (n >= len) [[unlikely]]
            throw gsh::Exception("gsh::Vec::at / The index is out of range. ( n=", n, ", size=", len, " )");
        return *(ptr + n);
    }
    __attribute__((always_inline)) constexpr const_reference at(const size_type n) const {
        if (n >= len) [[unlikely]]
            throw gsh::Exception("gsh::Vec::at / The index is out of range. ( n=", n, ", size=", len, " )");
        return *(ptr + n);
    }
    __attribute__((always_inline)) constexpr reference at_unchecked(const size_type n) { return *(ptr + n); }
    constexpr const_reference at_unchecked(const size_type n) const { return *(ptr + n); }
    constexpr pointer data() noexcept { return ptr; }
    constexpr const_pointer data() const noexcept { return ptr; }
    constexpr reference front() { return *ptr; }
    constexpr const_reference front() const { return *ptr; }
    constexpr reference back() { return *(ptr + len - 1); }
    constexpr const_reference back() const { return *(ptr + len - 1); }
    template<std::input_iterator InputIter> constexpr void assign(const InputIter first, const InputIter last) {
        const size_type n = std::distance(first, last);
        if (n > cap) {
            if constexpr (!std::is_trivially_destructible_v<value_type>)
                for (size_type i = 0; i != len; ++i) traits::destroy(alloc, ptr + i);
            traits::deallocate(alloc, ptr, cap);
            ptr = traits::allocate(alloc, n);
            cap = n;
            InputIter itr = first;
            for (size_type i = 0; i != n; ++itr, ++i) traits::construct(alloc, ptr + i, *itr);
        } else if (n > len) {
            size_type i = 0;
            InputIter itr = first;
            for (; i != len; ++itr, ++i) *(ptr + i) = *itr;
            for (; i != n; ++itr, ++i) traits::construct(alloc, ptr + i, *itr);
        } else {
            for (size_type i = n; i != len; ++i) traits::destroy(alloc, ptr + i);
            InputIter itr = first;
            for (size_type i = 0; i != n; ++itr, ++i) *(ptr + i) = *itr;
        }
        len = n;
    }
    constexpr void assign(const size_type n, const value_type& t) {
        if (n > cap) {
            if constexpr (!std::is_trivially_destructible_v<value_type>)
                for (size_type i = 0; i != len; ++i) traits::destroy(alloc, ptr + i);
            traits::deallocate(alloc, ptr, cap);
            ptr = traits::allocate(alloc, n);
            cap = n;
            for (size_type i = 0; i != n; ++i) traits::construct(alloc, ptr + i, t);
        } else if (n > len) {
            size_type i = 0;
            for (; i != len; ++i) *(ptr + i) = t;
            for (; i != n; ++i) traits::construct(alloc, ptr + i, t);
        } else {
            if constexpr (!std::is_trivially_destructible_v<value_type>)
                for (size_type i = n; i != len; ++i) traits::destroy(alloc, ptr + i);
            for (size_type i = 0; i != n; ++i) *(ptr + i) = t;
        }
        len = n;
    }
    constexpr void assign(std::initializer_list<value_type> il) { assign(il.begin(), il.end()); }
private:
    constexpr void extend_one() {
        if (len == cap) {
            const pointer new_ptr = traits::allocate(alloc, cap * 2 + 8);
            if (cap != 0) {
                for (size_type i = 0; i != len; ++i) traits::construct(alloc, new_ptr + i, std::move(*(ptr + i)));
                if constexpr (!std::is_trivially_destructible_v<value_type>)
                    for (size_type i = 0; i != len; ++i) traits::destroy(alloc, ptr + i);
                traits::deallocate(alloc, ptr, cap);
            }
            ptr = new_ptr, cap = cap * 2 + 8;
        }
    }
public:
    constexpr void push_back(const T& x) {
        extend_one();
        traits::construct(alloc, ptr + (len++), x);
    }
    constexpr void push_back(T&& x) {
        extend_one();
        traits::construct(alloc, ptr + (len++), std::move(x));
    }
    template<class... Args> constexpr reference emplace_back(Args&&... args) {
        extend_one();
        traits::construct(alloc, ptr + len, std::forward<Args>(args)...);
        return *(ptr + (len++));
    }
    constexpr void pop_back() {
#ifndef NDEBUG
        if (len == 0) [[unlikely]]
            throw gsh::Exception("gsh::Vec::pop_back / The container is empty.");
#endif
        if constexpr (std::is_trivially_destructible_v<value_type>) --len;
        else traits::destroy(alloc, ptr + (--len));
    }
    
    constexpr void swap(Vec& x) noexcept(traits::propagate_on_container_swap::value || traits::is_always_equal::value) {
        using std::swap;
        swap(ptr, x.ptr);
        swap(len, x.len);
        swap(cap, x.cap);
        if constexpr (traits::propagate_on_container_swap::value) swap(alloc, x.alloc);
    }
    constexpr void clear() {
        if constexpr (!std::is_trivially_destructible_v<value_type>)
            for (size_type i = 0; i != len; ++i) traits::destroy(alloc, ptr + i);
        len = 0;
    }
    constexpr allocator_type get_allocator() const noexcept { return alloc; }
    friend constexpr bool operator==(const Vec& x, const Vec& y) {
        if (x.len != y.len) return false;
        bool res = true;
        for (size_type i = 0; i != x.len;) {
            const bool f = *(x.ptr + i) == *(y.ptr + i);
            res &= f;
            i = f ? i + 1 : x.len;
        }
        return res;
    }
    friend constexpr auto operator<=>(const Vec& x, const Vec& y) { return std::lexicographical_compare_three_way(x.begin(), x.end(), y.begin(), y.end()); }
    friend constexpr void swap(Vec& x, Vec& y) noexcept(noexcept(x.swap(y))) { x.swap(y); }
};
template<std::input_iterator InputIter, class Alloc = Allocator<typename std::iterator_traits<InputIter>::value_type>> Vec(InputIter, InputIter, Alloc = Alloc()) -> Vec<typename std::iterator_traits<InputIter>::value_type, Alloc>;
template<Range R, class Alloc = Allocator<typename RangeTraits<R>::value_type>> Vec(R, Alloc = Alloc()) -> Vec<typename RangeTraits<R>::value_type, Alloc>;


}  


namespace gsh {

namespace internal {

    template<itype::u32> struct isPrime8 {
        constexpr static itype::u64 flag_table[4] = { 2891462833508853932u, 9223979663092122248u, 9234666804958202376u, 577166812715155618u };
        constexpr static bool calc(const itype::u8 n) { return (flag_table[n / 64] >> (n % 64)) & 1; }
    };
    template<itype::u32> struct isPrime16 {
        
        constexpr static itype::u64 flag_table[512] = {
0x816d129a64b4cb6eu,0x2196820d864a4c32u,0xa48961205a0434c9u,0x4a2882d129861144u,0x834992132424030u,0x148a48844225064bu,0xb40b4086c304205u,0x65048928125108a0u,0x80124496804c3098u,0xc02104c941124221u,0x804490000982d32u,0x220825b082689681u,0x9004265940a28948u,0x6900924430434006u,0x12410da408088210u,0x86122d22400c060u,0x110d301821b0484u,0x14916022c044a002u,0x92094d204a6400cu,0x4ca2100800522094u,0xa48b081051018200u,0x34c108144309a25u,0x2084490880522502u,0x241140a218003250u,0xa41a00101840128u,0x2926000836004512u,0x10100480c0618283u,0xc20c26584822006du,0x4520582024894810u,0x10c0250219002488u,0x802832ca01140868u,0x60901300264b0400u,
0x32100100d0258082u,0x430800112186430cu,0x92900c10480424u,0x24880906002d2043u,0x530082090932c040u,0x4000814196800880u,0x2058489608481048u,0x926094022080c329u,0x5a0104422812000u,0xa042049019040u,0xc02c801348348924u,0x800084524002982u,0x4d0048452043698u,0x1865328244908a00u,0x28024001020a0090u,0x861104309204a440u,0xc90804522c004208u,0x4424990912486084u,0x1000211403002400u,0x4040208805321a01u,0x6030014084c30906u,0xa2020c9011680218u,0x8224148929860004u,0x880190480084102u,0x20004a442681210u,0x120100100c061061u,0x6512422194032010u,0x140128040a0c9418u,0x14000d040a40a29u,0x4882402d20410490u,0x24080130100020c1u,0x8229020024845904u,
0x4816814802586100u,0xa0ca000611210010u,0x4200b09104000240u,0x2514480906810c04u,0x860a00a011252092u,0x84520004802c10cu,0x22130406980032u,0x1282441481480482u,0xd028804340101824u,0x2c00d86424812004u,0x20000a241081209u,0x180110c04120ca41u,0x20941220a41804a4u,0x48044320240a083u,0x8a6086400c001800u,0x82010512886400u,0x4096110c101a24au,0x840b40160008801u,0x494400880030106u,0x2520c028029208au,0x264848000844201u,0x2122404430004832u,0x20d004a0c3080200u,0x5228004040161840u,0x810180114820890u,0x809320a00a408209u,0x10500522000c008u,0x820c06114010u,0x908028009a44904bu,0x28024309064a04u,0x4480096500180134u,0x1448618202240003u,
0x5108340028120041u,0x6084892890120504u,0x8249402610491012u,0x8840240a01109100u,0x2ca2500004104c10u,0x125001b00a489040u,0x9228a00904a40008u,0x4120022110430002u,0x520c0408003281u,0x8101021020844921u,0x6984010122404810u,0x884402c80130c1u,0x6112c02d02010cu,0x812014030c000a0u,0x840140948000200bu,0xb00841000320040u,0x41848a2906010024u,0x80034c9408081080u,0x5020204140964001u,0x20a44040a2892522u,0x104a212001288602u,0x4225044008140008u,0x2100920410432102u,0x84030922184ca011u,0x124228204108941u,0x900c10884080814u,0x368000028a41b042u,0x200009124a04904u,0x806080102924194u,0x80892816d0010009u,0x500c900168000060u,0x4130424080400120u,
0x49400681252000u,0x1820a00049120108u,0x28241000a6010530u,0x12880020c8200200u,0x420126020092900cu,0x102422404004916u,0x1008801a0c8088u,0x1169008844940260u,0x841324a0120830u,0x30002810c0650082u,0xc801061101200304u,0xc82100820c20080u,0xb0004006520c0213u,0x1004869801104061u,0x4180416014920884u,0x204140228104101au,0x1060340841005229u,0x884004010012800u,0x252040448209042u,0xd820004200800u,0x4020480510024082u,0xc0240601000099u,0x844101221048268u,0x916d020a6400004u,0x92090c20024124c9u,0x4309004000001240u,0x24110102982084u,0x3041089003002443u,0x100882804c205824u,0x2010094106812524u,0x244a001080441018u,0xc00030802894010du,
0x900020c84106002u,0x20c2041008018202u,0x1100001804060968u,0xc028221100b0890u,0x24100260008b610u,0x8024201a21244a01u,0x2402d00024400u,0xa69020001020948bu,0x16186112c001340u,0x4830810402104180u,0x108a218050282048u,0x4248101009100804u,0x520c06092820ca0u,0x82080400014020d2u,0x484180480002822du,0x84030404910010u,0x22c06400006804c2u,0x9100860944320840u,0x2400486400012802u,0x8652210043009010u,0x8808204020908b41u,0x6084020020134404u,0x1008003040249081u,0x4320041001020808u,0x4c800168129040b4u,0x10404912c0080018u,0x104c248941001a24u,0x41204a0910520400u,0x610081411692248u,0x4000100028848024u,0x2806480826080110u,0x200a048442011400u,
0x1224820008820100u,0x4109040a0404004u,0x10802c2010402290u,0x8101005804004328u,0x4832120094810u,0xa0106c000044a442u,0xc948808300804844u,0x4b0100502000000u,0x408409210290413u,0x1900201900228244u,0x41008a6090810120u,0xa2020004104502c0u,0x4201204921104009u,0x422014414002c30u,0x1080210489089202u,0x4804140200105u,0x1325864b0400912u,0x80c1090441009008u,0x124009a00900861u,0x806820526020812u,0x2418002048200008u,0x9001100020348u,0x4009801104a0184u,0x80812000c0008618u,0x4a0cb40005301004u,0x4420002802912982u,0xa2014080912c00c0u,0x80020c309041200u,0x2c00000422100c02u,0x32120000c0008611u,0x5005024040808940u,0x4d120a60a4826086u,
0x1402098012089080u,0x9044008a20240148u,0x12d10002010404u,0x248121320040040au,0x8908040220841908u,0x4482186802022480u,0x8001280040210042u,0x20c801140208245u,0x2020400190402400u,0x2009400019282050u,0x820804060048008u,0x2424110034094930u,0x2920400c2410082u,0x100a0020c008024u,0x100d02104416006u,0x1291048412480001u,0x1841120044240008u,0x2004520080410c26u,0x218482090240009u,0x8a0014d009a20300u,0x40149820004a2584u,0x144000000005a200u,0x90084802c205801u,0x41b0020802912020u,0x218001009003008u,0x844240000020221u,0xc021244b2006012u,0x20500420c84080c0u,0x5329040b04b00005u,0x2920820030486100u,0x1043202253001600u,0x4000d204800048u,
0x8040029800344a2u,0x84092830406404c0u,0xc000920221805044u,0x800822886010u,0x2081009683048418u,0x5100848845000205u,0x944b4186512020u,0x80584c2011080080u,0x805008920060304u,0x982004000900522u,0x20c241a000000050u,0xd021264008160008u,0x4402004190810890u,0x49009860a0c1008u,0x8920300804a0c800u,0x800402c22110084u,0x200901024801b002u,0x4260028000040304u,0x20944104a2130u,0xa480218212002401u,0x1840a09104021020u,0x500096906020004u,0x480000010258u,0xc801340020920300u,0x2080420830084820u,0x212400401689091u,0x1100a00108120061u,0xc00922404482104u,0x9612010000048401u,0x8828228841a00140u,0x114122480424400u,0x108104101a609042u,
0x240028329060848u,0x4010800510806424u,0x2009018442080202u,0x1340301160005004u,0x4520080900810402u,0x2080c269061104au,0x200040260009121u,0x884480806080c00u,0x205a00a480000211u,0x9000204048800u,0x400c82014490814u,0x101200805940a091u,0x4000065808000u,0x6084032100194080u,0x808061121a2404c0u,0x820124209040208u,0xa0010120900434u,0x340240929108000bu,0x4000021961108840u,0x2104086880c02504u,0x84010ca000042280u,0x8a20008a08004120u,0x882110404884800u,0x100040a449098640u,0x800c805004a20101u,0x41121801a0824800u,0x1240041480401u,0x168000200148800u,0x808308224a0820u,0x34000000c2010489u,0x4a41020228820004u,0x424800902820590u,
0x1401288092010041u,0x4304b0104c205000u,0x44000201049021a4u,0x2042000608640048u,0x5020004a01920208u,0x800090422902532u,0x3200051001218011u,0xc10d240808948808u,0x4121840200b4080u,0x82c1052610402200u,0x841220224300100u,0x2812d225001a4824u,0x200413040040042u,0x890884d124201300u,0xa4184400520480u,0x2042091091200600u,0x4040840028304024u,0x4004080904100880u,0x8000000219002208u,0x402090012102022cu,0x120584834000c00u,0x90001480200443u,0x30020400000116du,0x65004a0530884010u,0x8003288418082410u,0x1969100040b04220u,0x4c20480000004u,0x9608252200050001u,0x12910d000220204u,0x44160104100860a0u,0x8440488202280210u,0x4000048028229020u,
0x6010032980002404u,0x205100a081000048u,0x920420410100d10cu,0x504420092100000u,0x2052201080408601u,0xd000020a48100021u,0x4800000480484112u,0x1043002400042209u,0x82c201244000a60u,0x806400984004420u,0x12980020804000c1u,0xc048840020a21048u,0x82980812902010u,0xc328000304a00au,0x40040804104244u,0x480032100100500u,0x408040010691288u,0x1820044948840204u,0x2010830806402u,0x1088412008491252u,0xd005860100340848u,0x4102402184830000u,0x5120a240488010u,0x1840209001004900u,0x880400522024002u,0x8201050018201082u,0x129908104005840u,0xa20140220064a0u,0x94806000000d0418u,0x120c30800d108260u,0x2120c04012000020u,0x203448010410258u,
0xc044000829901304u,0x1801a0026002100u,0x320020140a201413u,0x8009204240000861u,0x6800426080810106u,0x8002048042088290u,0x810c009800040b09u,0x92032884484406u,0x2810c000a408001u,0x920029028045108u,0xca0810900006010u,0x208028020009a400u,0x4148104020200u,0x120406012110904u,0x860a080011403048u,0xd001048160040000u,0x200a0090184102u,0x10ca6480080106c1u,0x5020820148809904u,0x22902084804890u,0x8610242018040019u,0x4410122400c240u,0x106120024100816u,0x80104d0212009008u,0x1104300225040u,0x140100000a2130u,0xa2910c1048410u,0x490c120120008a01u,0x6004014800810420u,0x44a4810080c1280u,0x5045844028001028u,0x980014406106010u,
0x9000a042018600u,0x8008004140229005u,0x4930580100c00802u,0x80020c0241001409u,0x9005100824008940u,0x61120008820a4032u,0x2410042200210400u,0x4020001001040a08u,0x12902022880484u,0x140b400401240653u,0x80c90100d028260u,0x2480800914000920u,0x2001440201400082u,0x41100a4084400cu,0x2020084480090530u,0x2000212043490002u,0x208044008b60100u,0x2410084080410180u,0x12c0098612042000u,0x8920020004148121u,0x6900d100801244b4u,0x418001242008040u,0x228040221064900u,0x820006810c00184u,0x2481011091080040u,0x100086884c10d204u,0x40908a0014020c80u,0x245800a480212018u,0x484130c160101020u,0x502094000094802u,0x21824204a208211u,0x300040a0c22100cu,
0x2100020404484806u,0x12020c0018008480u,0x8941108205140001u,0x48840121a4400812u,0x1400280240601002u,0xc200125120a04008u,0x4c128940301a0100u,0xa001011400008002u,0x140061821221821u,0x430024804900080u,0x448082488050008u,0x8000060224u,0x4820a0090116510u,0x2920424486004c3u,0x8029061840808844u,0x2110c84400000110u,0x141001a04b003089u,0x65200040940200u,0x2012812022400ca2u,0x88080010010482u,0x4140804204801100u,0x424802c32400014u,0x83200091000019u,0x4040840109204005u,0x2090414000112020u,0x618489290400000u,0x1024340148808108u,0x2d06180420000420u,0x220a009000011090u,0x101841100220001u,0x122004400882000u,0x1120060240a600u,
0x1928008a04a0c801u,0x9121224a0520080u,0x2400040048012408u,0x4048040008840240u,0x8148801220a6090u,0x90c02000d3080201u,0xa08b00100001024u,0x20000901008000a0u,0x8402042400250252u,0x40a00240921024u,0x22010804110822u,0x3000219009001442u,0x900922000c00006cu,0x20c02000402810u,0x1212058201400090u,0x812802806104c109u,0x2986100804490024u,0x908849300a218041u,0x941808129044100u,0x4010004010124000u,0x2040210280050248u,0x48900060205800u,0x4400004880c02880u,0x212000609000280u,0x1245108308100001u,0x2020004404082c00u,0x20c80500012010c0u,0x224001008109804u,0x2412886100884016u,0x61008004200a680u,0x8104205000a04048u,0x1801008001840a4u};
        
        constexpr static bool calc(const itype::u16 x) { return x == 2 || (x % 2 == 1 && (flag_table[x / 128] & (1ull << (x % 128 / 2)))); }
    };

    template<itype::u32> struct isPrime32 {
        
        constexpr static itype::u16 bases[] = {
1216,1836,8885,4564,10978,5228,15613,13941,1553,173,3615,3144,10065,9259,233,2362,6244,6431,10863,5920,6408,6841,22124,2290,45597,6935,4835,7652,1051,445,5807,842,1534,22140,1282,1733,347,6311,14081,11157,186,703,9862,15490,1720,17816,10433,49185,2535,9158,2143,2840,664,29074,24924,1035,41482,1065,10189,8417,130,4551,5159,48886,
786,1938,1013,2139,7171,2143,16873,188,5555,42007,1045,3891,2853,23642,148,3585,3027,280,3101,9918,6452,2716,855,990,1925,13557,1063,6916,4965,4380,587,3214,1808,1036,6356,8191,6783,14424,6929,1002,840,422,44215,7753,5799,3415,231,2013,8895,2081,883,3855,5577,876,3574,1925,1192,865,7376,12254,5952,2516,20463,186,
5411,35353,50898,1084,2127,4305,115,7821,1265,16169,1705,1857,24938,220,3650,1057,482,1690,2718,4309,7496,1515,7972,3763,10954,2817,3430,1423,714,6734,328,2581,2580,10047,2797,155,5951,3817,54850,2173,1318,246,1807,2958,2697,337,4871,2439,736,37112,1226,527,7531,5418,7242,2421,16135,7015,8432,2605,5638,5161,11515,14949,
748,5003,9048,4679,1915,7652,9657,660,3054,15469,2910,775,14106,1749,136,2673,61814,5633,1244,2567,4989,1637,1273,11423,7974,7509,6061,531,6608,1088,1627,160,6416,11350,921,306,18117,1238,463,1722,996,3866,6576,6055,130,24080,7331,3922,8632,2706,24108,32374,4237,15302,287,2296,1220,20922,3350,2089,562,11745,163,11951};
        
        template<class Modint = internal::DynamicModint32Impl> constexpr static bool calc(const itype::u32 x) {
            if (x % 2 == 0 || x % 3 == 0 || x % 5 == 0 || x % 7 == 0 || x % 11 == 0 || x % 13 == 0 || x % 17 == 0 || x % 19 == 0) return false;
            Modint mint;
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
            if (cur == one) return true;
            while (--s && cur != mone) cur = mint.mul(cur, cur);
            return cur == mone;
        }
    };

    template<bool Prob, itype::u32> struct isPrime64;
    template<itype::u32 id> struct isPrime64<false, id> {
        template<class Modint = internal::MontgomeryModint64Impl> constexpr static bool calc(const itype::u64 x) {
            if (x % 2 == 0 || x % 3 == 0 || x % 5 == 0 || x % 7 == 0 || x % 11 == 0 || x % 13 == 0 || x % 17 == 0 || x % 19 == 0) return false;
            Modint mint;
            mint.set(x);
            const itype::u32 S = std::countr_zero(x - 1);
            const itype::u64 D = (x - 1) >> S;
            const auto one = mint.one(), mone = mint.neg(one);
            auto test2 = [&](itype::u64 base1, itype::u64 base2) GSH_INTERNAL_INLINE {
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
            auto test3 = [&](itype::u64 base1, itype::u64 base2, itype::u64 base3) GSH_INTERNAL_INLINE {
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
            auto test4 = [&](itype::u64 base1, itype::u64 base2, itype::u64 base3, itype::u64 base4) GSH_INTERNAL_INLINE {
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
    template<itype::u32 id> struct isPrime64<true, id> {
        constexpr static itype::u16 bases1[] = {
1814,1307,1714,6838,1801,179,219,1317,6834,3281,8345,4034,1899,5746,4977,117,12578,18141,4434,4236,543,9046,4005,385,4231,3425,859,1450,2199,4262,7025,1562,717,5611,5298,871,1959,258,4543,8969,1075,15030,21041,7917,913,15929,3450,9218,518,11775,7993,175,107,407,4738,14388,86,6360,3535,3818,8083,11,4337,19601,32654,2566,974,1549,5047,1542,1941,3887,933,295,106,823,1529,7999,8424,2958,17995,1430,8541,5213,341,13034,4437,4058,21281,715,1898,434,1298,8930,33,3466,397,14181,2685,2179,15160,271,175,5403,
1303,2091,4133,466,2143,3908,2819,159,358,302,7778,2001,1029,182,7405,13580,26511,11758,743,4841,12448,3585,3123,1959,3999,1906,597,8293,2508,8231,8676,485,4925,12605,22286,1099,1385,382,10310,746,293,415,2947,61,295,4797,11775,2902,4254,12326,1335,1541,17775,8067,13739,4254,685,3963,2258,3827,1151,2962,1981,1043,2910,8555,2478,3181,1399,12907,5843,2056,7,598,17287,609,1823,9968,12866,9614,1177,1799,8135,8861,6242,3655,8700,11136,3693,229,6637,8587,4915,820,6594,8899,3012,833,5439,402,387,9998,10074,
401,9281,2246,12470,7919,1533,2725,947,6314,6681,2733,10132,1535,7066,17066,1529,507,29293,5893,6323,551,506,1686,394,2355,2394,199,8744,3451,1759,12630,8251,21127,603,1349,16013,1916,657,3143,10920,344,1522,3123,10732,635,4927,2641,2786,554,77,8759,2047,8638,233,19015,237,29990,4033,110,3949,351,293,12792,2370,3878,9235,1477,14719,4439,4571,3606,6895,1256,8271,2189,73,1006,28711,1970,8162,1034,1258,3018,5062,2381,1326,19172,17936,17446,4714,7939,975,11261,17179,1138,9907,7654,8844,213,3378,7937,
118,3421,1217,7742,52,18125,381,1131,545,9748,8528,1219,2206,415,6235,50334,74,397,1773,1150,16385,1419,8899,811,3813,2511,10359,17270,22652,9646,34497,3474,141,1554,1287,4642,1571,934,2834,101,749,258,302,26159,1291,5105,1783,7310,415,1558,2393,13393,263,1362,7985,9569,8240,2279,5018,8430,1961,6005,17635,3622,4213,36293,9985,11,1254,2251,27396,239,17364,3863,4615,7899,3674,5531,199,8996,1293,6098,514,8538,17311,2015,6226,4459,5731,887,5449,8085,1851,206,2925,8337,2363,9490,1085,590,9094,78,7190,
2843,86,497,1658,113,24990,5302,3141,18856,16074,5754,9497,1757,1353,4013,3739,6930,6179,813,6894,2527,2218,14648,397,2979,3074,2314,6521,18390,2157,10642,1177,5007,391,1982,4373,26,1917,1567,3091,22352,9374,935,3263,8242,3239,3558,18600,9839,2440,5325,6414,1303,2995,109,5731,9966,1478,8867,9094,284,433,1750,277,9773,22660,181,127,279,641,851,199,539,1190,446,10526,2566,677,127,2404,9715,8319,15005,3086,2779,3951,7,6929,461,27337,8909,9543,5150,6703,3327,769,901,851,2227,819,1879,3130,4927,2302,
802,7599,1163,71,33,925,5550,2971,8201,23865,114,5886,1145,19851,6219,1877,613,827,11302,5726,1237,769,590,9156,9048,6049,7379,5462,12522,5706,823,8431,3349,1623,2245,5198,1329,14794,6193,2266,9722,3716,10294,2009,1193,233,3979,1077,418,941,4613,8635,26786,1507,8508,258,1419,5225,7485,1051,13020,31711,22593,3331,3338,12526,161,5338,7302,78,23889,571,26734,8013,8305,9053,1566,2482,191,12703,1703,3385,654,26390,3647,2639,8498,3813,1161,7777,117,2265,813,8069,2053,2091,5033,6637,8588,15954,325,1713,
9719,1983,619,107,419,9104,9019,8688,577,1773,237,3379,2946,1151,13815,1119,1393,11565,2177,530,265,1663,4058,11031,12256,2443,173,1405,1258,8592,4364,4883,1201,2470,119,2775,37084,6690,1067,10479,337,4147,19565,853,4005,1790,855,2329,481,585,2616,1741,10908,17057,9267,4403,10597,4963,7154,3079,8245,3271,10807,1513,691,2821,5285,681,9822,690,566,46,639,141,6706,9308,1009,7433,24590,29,25809,710,4839,2857,17910,383,4630,2993,1059,39,13218,10597,6607,3466,3029,8639,1646,8895,231,4885,3305,5734,10310,
5954,7773,3329,587,14302,9806,2130,189,1267,7397,12364,953,299,5297,3354,1921,2486,6827,2202,2369,15279,871,713,8273,1227,9691,3475,469,865,2230,353,6249,9310,1546,38417,2199,1789,9051,6555,3766,1701,10210,1270,3279,8620,3039,929,2029,618,6641,7122,4215,34,89,5251,1293,6401,409,537,935,26421,2882,4129,55,7427,369,12218,9918,1870,4888,1449,1430,4430,5291,1699,2074,67,12096,6687,8414,573,2803,887,2435,1138,2413,4129,205,2190,1654,3697,3170,3538,1574,555,1338,2822,1077,610,8391,1539,11,6637,1053,179,
698,1495,1217,1436,9935,778,310,801,718,1066,3066,261,839,651,3293,20997,8527,8721,2146,37272,3607,9206,698,9983,28805,221,3686,11549,348,181,1293,3846,2717,6926,5133,137,1785,9727,2325,2723,8678,607,3010,8749,8855,10668,2675,301,7899,8191,9141,6622,9435,1579,1539,3261,5773,218,1654,4466,739,3366,2047,557,3189,7558,2867,2810,3842,1663,4635,34027,269,9256,3533,4074,1022,3365,1633,15096,8323,9249,1767,1326,39,9211,165,8797,9519,1006,5738,18521,323,1746,10996,1517,8204,6547,4359,1834,2966,716,715,2470,
1443,8518,2938,1962,2387,10961,851,5533,553,5150,934,554,157,4429,15949,2194,1657,1449,7391,377,2065,3307,6695,2731,1141,813,3238,8318,2255,9003,618,3697,24981,4401,1523,219,721,6284,6220,7747,333,12835,5619,949,4346,1206,846,8063,6167,1383,6843,10299,3085,9018,277,221,12774,1173,1926,21591,4182,2194,8223,3983,550,2254,8371,606,1202,29,1273,6557,8808,7918,3797,2941,1566,15003,1955,2796,2583,233,10105,1418,2889,24939,9590,3573,613,70,247,934,1822,895,4197,30570,8511,9367,11295,4715,3145,3103,57,5810,
11354,6406,1111,30851,2838,2639,8673,18562,2999,9448,11625,1773,9272,13946,1498,4613,21674,30566,5053,11644,3701,2654,4693,19,1986,14647,1615,1711,1259,4569,2798,1070,25016,1461,358,1457,807,1018,9256,1249,9589,8615,2290,2073,21,1363,321,3059,6965,865,12695,287,4322,6173,8206,1967,457,59,1417,15449,5635,2734,5122,1630,1115,4194,3458,10494,5373,2266,7884,8637,6114,8655,4279,6438,1383,3838,2078,1721,2485,2447,541,8289,2291,221,138,567,7194,1141,1941,1311,1522,1687,3510,2446,8578,10349,25626,32066,
1654,413,129,11851,4902,547,2014,4427,11738,1661,2081,10933,3226,438,4433,4679,339,617,9451,6134,6822,8957,5171,14095,11767,2697,3351,9461,2207,1501,930,617,435,8966,3185,6603,10993,908,881,1033,254,1538,1383,7890,1489,18615,1070,20332,3867,5666,1513,2661,7297,3263,37879,5883,11163,1081,2589,5033,10067,56007,8629,53493,5443,5749,3982,791,5573,3967,549,3371,727,6788,4382,607,4111,2837,4565,2273,2567,286,39,1441,4254,27375,5061,442,3597,8308,8035,3041,967,27275,1939,1034,1038,913,4649,426,87,36883,
4918,2381,478,3965,883,3855,4366,7214,3562,681,763,8538,1229,3223,5371,773,1825,2061,446,629,8832,2269,3393,6712,9365,11001,1459,9795,1247,1966,1117,717,1547,19975,3428,253,421,10245,933,926,3789,3351,5138,262,2193,1910,1183,193,1497,151,1343,7343,2817,351,1767,6540,6179,1983,2985,14,2811,17706,1271,737,10887,305,2794,4717,12567,1507,17602,1523,5209,463,8866,11923,4955,5265,5288,6043,5641,595,282,446,6497,911,1661,6946,609,8322,457,3393,7889,649,5663,2237,8149,5867,1249,7135,1079,17165,32290,26529,
26547,483,4357,4110,1893,4482,1930,3294,9811,35577,2541,11035,1091,129,513,445,12330,3553,2689,9003,29917,23793,8814,8380,738,6846,1693,127,2667,5245,787,2058,217,1073,462,311,5087,10108,291,2431,951,41,16869,9010,10398,1531,4053,2456,727,4636,2689,1526,2126,3736,279,4038,1005,2186,9358,5537,3729,693,530,749,4379,1583,34038,2590,457,311,3077,12423,1006,1923,3071,811,10642,8501,4835,1735,9606,35324,85,8232,2425,177,8384,933,341,2481,2017,3694,1381,131,713,4213,13802,959,1343,3674,325,4090,265,1147,
4209,785,1970,110,4262,297,6074,4769,5299,2309,4433,3833,17427,6914,9188,2246,7726,8747,7501,10637,278,655,10539,349,414,2871,31558,3958,10727,3338,1703,11716,22687,8919,17340,2461,231,1994,797,12341,307,5114,691,20111,532,2286,12881,4121,2453,874,2947,5494,1866,2543,665,3864,1581,11513,35676,4961,2715,447,73,5211,818,4286,5691,6021,466,5606,1077,5970,1293,4499,11210,2006,9794,1141,4861,21132,1068,2033,251,6987,5718,3218,629,755,453,3259,457,5181,6482,25951,8372,233,1397,511,7157,10090,8219,10289,
9257,29842,670,4025,2471,35572,1035,354,831,1713,2215,3393,16440,16076,32137,2806,723,1223,2007,137,463,38,1195,3566,1462,33400,101,785,89,6471,3595,187,7006,3323,322,20079,413,3115,12292,575,2635,5479,9336,5319,10302,1801,2925,1530,1139,6043,5654,565,6333,842,67,16995,3923,9546,1697,4877,239,55,5989,9439,366,3723,839,3097,10983,395,4135,40693,619,1310,19017,7057,9101,7513,2173,1314,9023,13279,281,82,1007,8011,706,3061,6470,7503,1547,33138,183,345,2533,118,10472,563,505,11464,5123,8943,4149,2203,
489,2511,682,8525,4387,1127,4066,14636,2982,5560,2310,7825,497,2938,3787,1306,3791,10147,215,9734,12660,1397,1079,1907,10889,97,11659,3126,8857,2495,1895,8348,1209,42,4622,11471,313,1022,1185,1039,5813,15838,2995,7481,8107,2118,8554,117,13263,1122,38,6378,767,7859,1361,1854,9849,26881,1763,2830,6158,1041,1789,1939,756,1075,239,1054,1823,50612,11794,19210,6947,33782,4174,5451,786,2006,2653,1909,963,14187,2429,1199,2971,11327,2971,4419,31837,195,2493,2237,509,119,1023,11335,6753,6021,8385,1474,11087,
8289,237,3473,998,19,1106,13188,159,413,2579,8123,4488,1839,1770,3765,1842,237,1026,5031,23543,5823,6053,295,1930,1685,2193,4344,5410,1097,4995,3765,3055,13144,1283,2734,1147,9486,17,1339,3065,7703,3295,5405,1071,617,938,2505,437,3921,671,5842,12540,3681,11,5790,534,3755,313,1334,6903,4343,1101,7133,885,6518,6879,4589,469,1625,1774,737,15813,13743,266,775,131,2799,97,1845,5810,593,10268,4270,3309,6339,2702,9644,1209,1073,7107,2903,253,1978,643,3573,233,5741,4018,3811,326,5714,1574,6794,122,843,661,
1771,2651,2858,16330,4957,8522,5097,2353,1022,5733,3676,7539,609,1351,78,2735,971,1931,2539,4190,1782,8817,3710,4515,3725,7182,5795,13535,4921,9587,1785,9627,2295,5750,2849,1537,23868,327,899,9475,10305,241,5939,4223,3187,171,4191,3480,530,5411,674,2931,679,15237,1378,1426,1714,33684,3330,2195,14478,27430,1275,4773,1117,8516,415,1441,5859,8501,547,1108,13905,1422,8908,5698,10789,1779,1385,2635,14718,10378,6153,2606,9453,2246,7302,493,10989,21648,1127,8798,24057,330,1487,8675,739,1811,4445,3301,3065,
5115,1415,7959,267,3129,14051,6209,382,12483,522,1113,10089,9521,495,6150,11568,3754,13268,377,5559,933,3819,1205,1077,8766,2076,9137,31061,1954,53,5651,4578,9272,11578,497,309,747,6139,8729,10329,4226,563,59,3143,9909,2246,1727,39,17041,9150,51,629,8416,7,1287,781,7213,16098,9173,965,2581,558,467,2525,6350,8344,4895,2474,119,875,1261,753,9806,15075,16324,9171,1286,2177,9456,5126,18290,1771,6631,21720,8167,4961,6151,5018,33874,8865,4519,7159,1447,4395,3021,327,3634,1002,7650,3822,6341,2194,1275,
391,4459,7047,3450,4442,36922,12597,493,33779,825,5291,914,8348,1765,1589,222,5454,1971,1163,1686,8996,12473,15722,8998,175,15597,24939,1651,7307,19984,9696,8462,39,1367,1613,5949,14266,1082,1883,40249,2402,2015,598,595,22769,9766,1577,3587,2687,4298,1282,17860,4337,3388,9544,9354,4267,431,3235,215,3639,2697,8034,739,3783,6515,8602,4357,9085,1046,9463,10105,230,233,708,3571,958,663,9174,26220,1573,939,1001,30407,30199,1398,1793,297,8259,788,1354,714,10163,9262,1334,3673,3162,262,2351,2454,634,9888,
395,3559,3582,447,4889,665,549,7653,26390,2295,24008,1633,9844,12067,12190,7551,783,1117,1013,1037,4387,3831,1138,2870,186,697,463,42723,1145,553,3938,6759,8930,2799,36308,36438,2725,42,1239,173,4602,2487,1617,5117,309,8552,14077,586,836,4454,2467,257,4959,7583,1193,4358,1509,1817,5155,149,425,12379,369,2101,9459,2394,86,12155,6134,3214,298,306,4315,2825,4293,1891,16369,683,10840,4667,2701,587,1505,7923,9416,5773,3259,2331,11122,5395,1419,58,891,4811,202,3553,2077,1809,2158,3711,6714,510,6598,7886,
951,618,402,11766,7775,22833,8300,3505,10198,8075,1002,6285,8594,5978,8843,6469,14349,241,2094,4745,474,1657,337,6702,3382,4769,37346,698,6654,25229,7710,2623,2254,6547,5689,395,7087,1130,1091,562,5539,7751,7313,9250,19891,1546,1613,618,1558,2593,1623,4433,7893,11064,326,1797,12623,8202,10434,10708,37382,1191,1194,9848,1314,633,6040,3406,12332,258,4457,16136,301,323,5859,13959,235,39884,2137,1197,1277,194,446,1201,5813,2339,4690,1391,8238,203,2991,5099,2990,6082,9708,8726,6583,10146,26158,63,10238,
3027,3771,2353,1098,631,3460,23197,4667,2866,618,1779,1579,9118,8157,703,5327,7355,10858,2709,4483,627,965,9160,2531,2049,899,61,3467,3802,5255,1561,1071,16171,2199,1413,14344,611,2195,12869,3357,804,3062,2314,2811,18751,515,929,4110,1675,42,427,5573,2259,9557,1339,10390,8577,230,9168,1141,3961,871,8420,20283,1515,517,6293,11102,1611,390,5177,2447,1315,663,2449,475,2027,646,3834,3827,781,8977,418,9111,9859,426,91,12846,1063,770,3490,8514,3347,997,8977,11043,1306,2622,2577,1629,69,7413,3215,2579,
3069,1383,2091,1311,1674,4042,7946,3803,479,1819,7459,18973,445,653,3867,5101,3039,8274,11634,3449,514,394,3519,949,4981,574,401,1235,67,8170,455,2743,251,546,12009,1709,3859,3287,577,7854,16114,9549,4082,2550,2059,167,488,267,581,4065,1411,3847,21190,1245,2198,11123,862,114,3949,15247,4743,35399,4162,725,1476,13148,3917,206,12255,347,687,1293,8266,9033,1219,4573,2449,8535,4527,1987,417,957,5311,1265,5650,2517,7873,7498,839,9386,249,4850,8543,751,3527,1825,10317,1783,9334,2127,3661,14659,11433,2114,
13388,137,1837,4443,3900,938,1226,5603,8117,10264,7001,1755,2501,10772,5086,1649,3169,2211,1659,453,2594,501,6654,18817,3429,3661,7,885,7502,10083,5807,2830,267,1001,411,8372,2831,3614,2655,6689,14038,893,1685,1195,2550,1207,10292,110,1830,4846,3643,967,8456,1630,57,742,805,2961,13600,3798,6915,870,2946,16517,173,11537,4598,963,573,8726,10825,1881,8082,1887,86,17,7595,3190,3975,2909,12673,1517,4855,641,3262,8263,6967,6011,1189,1571,257,1315,77,8267,3467,327,846,264,913,2242,3433,671,1527,2906,6153,
1827,8189,789,5359,2982,2529,4874,2643,563,3279,3389,13080,390,790,4329,1018,965,19849,3150,8249,6066,7167,1002,4491,3695,759,5705,6221,555,1949,12784,701,1073,4209,3175,14126,4345,5795,22149,1027,946,8944,8358,9324,3558,995,27113,8524,47,1379,1549,601,7491,2903,16198,759,11501,11500,11814,8453,7286,1478,8931,2082,12974,1763,347,9355,13892,8893,24825,757,2132,1158,3180,1398,1129,4631,6898,4737,319,10443,1199,23935,8504,12169,1486,8178,573,4097,5029,3939,3369,19764,325,2174,595,1254,897,4815,3606,
3337,2873,6127,2942,2337,9571,103,1957,1045,14285,2657,873,3869,8366,2182,3381,5063,2647,2975,23863,2263,101,2849,9470,4109,1108,151,2599,1015,2005,785,733,2227,5655,13902,1714,2282,3243,6619,16141,129,18228,7298,7093,1063,6823,293,5473,627,6214,3317,6771,6165,38900,1937,1746,1269,2329,6082,11224,9379,4795,13866,8956,57,5566,6489,3943,233,1847,967,530,8733,606,2279,4942,9920,3239,10307,6246,10483,9803,4429,695,25931,2564,46,983,3847,1989,1834,9705,1434,202,9976,8081,8452,3414,733,3821,3333,235,167,
909,9333,1318,438,10315,414,5023,3955,1181,3917,1415,1959,2497,1647,2755,879,4652,2483,6246,282,31,4319,7063,783,2797,23,5899,4177,62,9908,3337,11189,4829,9221,16324,6483,7851,5371,761,5237,8922,3055,4417,5103,290,1357,1822,91,302,341,2555,11573,9424,911,14920,6234,12781,9614,935,9285,8612,730,930,379,8435,681,11055,7465,8658,7713,7702,9099,5203,5379,4263,8561,413,2879,7978,15744,9028,3512,873,3490,1766,633,5802,2775,14459,10577,2141,5493,553,14709,9927,1417,1953,1567,825,15910,618,743,491,12034,
13580,5527,426,1027,653,2246,3845,3229,2894,1553,11580,8707,4040,13748,10361,9733,4958,1742,265,550,238,202,3795,1051,5658,506,7594,3198,2091,2294,935,16882,3201,9416,5213,2127,20447,8697,5386,11993,1507,2510,1087,55335,8709,9199,8866,5014,687,13,17066,811,3991,846,457,73,11930,5761,1186,1169,37,615,4593,1026,3778,5699,6297,9825,1337,1859,5881,2778,12511,773,1077,7301,137,4731,3667,1262,12028,25454,16558,13560,1598,5126,8409,229,1121,1901,514,8789,13127,2699,1090,8327,113,2233,537,866,1077,20083,
1255,47,17516,939,2101,1553,9535,206,12130,802,13290,6747,8003,10517,1463,183,666,1567,2002,153,805,629,1094,201,5135,25282,7605,87,3135,3521,11759,467,2722,6878,10398,6786,1953,9705,9372,7025,902,5838,59,1847,193,11466,4729,10970,16215,6321,1209,3211,8629,1243,10586,9955,4002,359,8848,5218,12110,598,5086,83,11617,10312,6661,2635,4674,3973,802,4703,365,1195,11558,2534,1401,874,8972,5011,4079,875,7418,1915,2701,36501,15472,7298,47,7287,813,905,5062,609,12056,3777,8965,531,325,1167,1557,1935,9162,
702,4539,3663,197,4978,26779,8005,3843,157,14203,2135,4702,4129,4259,1410,3821,10397,5794,7841,2786,9906,2562,12754,3955,8611,721,3353,783,7,1821,1138,8238,14464,869,662,43,2957,1005,17632,406,4090,822,1527,6209,3349,865,3039,5455,6874,2999,14114,5829,2751,11968,3533,6987,1726,271,5609,2191,8495,4259,705,7475,9945,4209,10238,6104,21039,9559,905,16581,2357,2237,7981,20161,1021,1155,27047,6559,2485,1318,27706,23,5589,33998,11175,3685,949,13682,5542,1295,745,2621,1614,3291,2263,143,6819,8629,15753,
12906,13988,5135,8711,10817,1429,27903,7673,2906,1229,3347,119,2915,3223,53,1282,37910,1605,2518,1165,4131,3502,3123,1185,238,9935,10470,1695,1149,14237,5054,5291,11489,1045,2929,414,2223,10223,2966,6545,563,1286,14409,11185,12491,3010,10548,1307,5458,2065,3297,3357,8111,9016,9077,145,651,8528,3250,451,2015,5755,1498,1631,2393,8545,9415,2203,921,9723,6077,2917,7459,5486,563,2105,29726,8745,5577,710,30187,10675,1321,123,9696,2855,9497,31085,12340,3729,374,1393,2491,5015,8972,6741,2126,3627,2003,8514,
2414,2006,753,633,3395,5730,10219,871,4499,2833,4241,1955,1807,15988,8726,4207,6014,10213,16722,5717,8636,365,3691,62,8212,14814,18465,989,1121,13774,927,202,2053,1593,2181,29,1867,30613,8467,1773,101,3062,2613,3653,329,28245,2589,982,4060,16311,30745,1119,6010,835,378,2994,897,9784,1761,8749,3386,7918,295,2493,17637,11666,9997,2685,9932,1030,573,2954,377,6009,4897,21,709,1122,921,77,7061,13576,14489,6062,5031,451,691,290,1074,5409,9630,1433,10274,1413,4982,5602,5995,1638,449,5785,264,1797,5006,
4698,5183,505,87,2171,10058,1045,501,20861,34822,14,11442,1603,3038,9343,8990,8641,8325,94,279,2307,3686,1993,4382,15155,1410,781,2379,3898,10256,7110,97,8710,6809,482,3034,133,3145,1410,3013,1618,6349,414,2214,1010,22666,821,1313,9340,14471,12239,8847,1011,381,1045,5457,7503,1414,943,394,13675,823,2521,402,18213,2701,689,1302,61,1857,2718,575,8842,1791,442,12622,3901,5723,1187,10006,5825,9071,5650,95,3963,9193,191,2570,8271,2246,1538,395,5571,12520,1758,1223,10003,3241,4551,427,5003,39,2798,5739,
842,6010,89,2794,5395,9557,3446,5694,41868,5679,32901,2569,1399,4565,9746,40747,5403,1362,2851,26,886,442,55,3053,4895,3871,739,2550,5690,505,642,1505,2270,483,13414,4307,16633,1583,847,12303,5367,159,87,701,10519,17861,3958,3691,15220,2397,282,161,1315,8977,5734,351,1209,2285,8804,4451,2393,4807,1267,1610,10303,1002,12929,6767,2339,7550,7741,8209,369,7914,8074,1686,1918,5909,2081,18317,3356,4491,197,10102,7895,1624,922,7383,1002,8880,138,6581,370,13034,894,25887,2325,1494,1773,383,3099,849,1877,
25954,3202,1059,2431,9101,4681,18662,13765,3605,4063,11196,8445,2478,6899,6939,11226,11177,979,11548,23549,1513,417,7742,379,2961,3497,2869,3830,1657,52,8248,11582,8285,389,32764,8021,4138,1229,7167,9013,3771,2998,1335,1611,46,501,1374,8470,15179,3035,28253,886,7479,4641,70,8194,8633,2141,7881,1891,20741,1747,9536,8202,9296,917,1247,3954,1971,730,1779,8967,971,2695,3006,8217,4887,20668,3661,15431,11327,381,742,6026,5730,8637,1501,358,9862,1829,5231,27105,858,366,3143,1046,574,62,1218,9024,349,10419,
1180,4561,159,2257,349,5095,107,9573,653,18543,5974,14512,8637,8215,5393,3575,657,5965,239,153,407,305,5491,5463,12680,8734,334,8331,4754,1337,3762,2935,111,3891,2957,137,451,627,4439,1827,31,3727,4949,6078,2425,2522,1282,3113,1583,4573,6745,4878,6647,2665,4627,51,2743,341,6714,2261,397,1999,1633,7221,11216,7299,7259,1961,3106,951,2730,1732,2382,1391,2470,4338,591,7507,4565,1208,2207,353,1671,3807,853,11611,645,4703,4021,501,2232,2419,497,12029,3099,893,2062,5942,4934,13277,985,873,2821,8330,8327,
10300,451,723,8459,1974,28847,2910,839,2915,12869,4722,981,4333,3005,17779,2114,8429,2943,10274,9504,767,65,358,582,3753,2626,6373,974,295,1633,12690,6353,35178,1413,1243,755,173,1967,13079,2022,1019,9769,187,11060,28711,9109,4595,7191,803,99,11030,275,11962,6434,8528,4039,6034,123,1244,2513,2986,3873,65,36075,8717,21884,429,6963,4582,149,9695,9616,1413,2915,1094,9006,119,2201,367,2170,2605,2061,1634,925,2665,1913,11438,17811,12610,6675,1671,1711,3329,7053,5223,28562,1981,2382,8039,12156,2747,319,
10973,4918,3694,499,843,2497,171,4953,1031,643,14139,1911,7190,572,1613,677,1463,3735,17967,10549,1979,6554,2648,1427,615,17315,10873,3142,430,1893,1911,1882,8713,4782,8145,113,8242,937,5890,925,7062,1911,8542,20615,5831,22307,2255,9299,463,714,2146,627,4314,227,174,9569,14120,12664,383,9932,299,982,8029,8332,3854,8741,7014,1695,3781,26865,6086,2547,3598,1181,2303,1067,10842,3655,398,9584,12828,590,1367,3059,1114,5014,479,7647,1623,9567,5076,495,4086,14425,6711,1307,715,822,933,1195,354,36824,8230,
8340,3347,16803,12920,2821,8551,2374,17680,10366,7287,1265,1751,14409,1599,1697,9785,17774,7194,6335,1917,8883,1579,5621,210,221,7342,9521,2051,393,12370,3804,3801,9565,219,86,6658,3949,2917,5790,89,3395,6371,3219,1306,3046,3651,8561,7806,2771,5493,2111,7182,57,155,4849,4505,1163,10309,247,15878,6307,9605,7194,3357,9719,253,46,2677,30597,2623,4935,418,126,1242,2107,561,922,277,11177,10390,1333,3145,14550,2369,789,9821,185,1229,4695,1333,3382,2679,4817,2747,737,1322,218,7237,1683,8725,10068,8769,
189,25475,5293,3198,3731,10601,6277,2797,5979,9052,45502,9507,3171,13942,263,3673,2739,1334,4293,4626,313,10054,12940,1725,454,3198,365,4377,10102,3183,9266,5710,37,4549,222,16997,13589,149,5643,17881,2990,1414,3789,421,2746,1157,975,1074,233,1580,237,359,7186,2051,6747,8676,1325,3857,1286,17648,2823,10024,1576,558,2517,13656,6330,2995,787,1926,11892,6353,14206,998,3448,122,987,373,4290,2669,615,2577,489,413,4582,26571,374,1493,13595,703,7612,2006,8433,3797,10567,5550,16144,7753,6635,805,3715,430,
685,893,3633,11093,3293,59,1063,2369,6891,19833,6874,8103,5725,1755,1247,2954,11876,2862,590,2819,302,8975,285,1999,2577,7539,117,8952,6281,10643,373,7538,2629,946,1889,89,6275,1251,8555,11469,3297,993,10921,3242,3964,1913,949,1262,3,10139,916,7571,1599,730,262,8596,757,965,630,4189,8286,16811,1755,12610,5961,7799,71,1966,6010,29047,26831,35,9879,3693,4785,1953,357,238,3406,714,1064,6518,3729,1787,2718,167,1819,2819,6329,301,310,4497,4421,834,8267,9281,3787,2322,3754,102,3129,2163,9946,1757,5022,
8035,1229,145,103,399,417,599,1627,1627,1221,4150,6327,10037,339,2455,259,14586,790,23973,3126,1453,1114,89,3801,319,13816,8482,16661,5359,331,2773,2301,11510,9440,3203,1141,786,7451,5354,2489,12788,3562,1093,14944,1101,1793,9152,263,167,10356,8370,6545,51,2004,2031,498,2027,2657,2358,183,1569,1084,4003,12548,241,12409,4161,218,481,1543,1115,1311,5409,9748,2839,21673,6149,8001,25953,6043,3965,1223,2334,745,16238,865,89,1047,5284,1816,477,12767,11022,10422,2523,5139,14177,823,2108,25035,983,73,4089,
5113,6821,3595,29,1595,833,1323,1051,4677,26134,22220,2635,3907,2417,1574,5134,7999,9241,1754,5441,14097,5378,8468,3562,5091,8370,442,4235,8125,34286,12416,3547,510,4351,15718,82,2751,4807,2297,4923,287,114,1404,1161,909,19956,13860,2434,470,2232,1277,6811,819,4038,654,51464,5854,4159,1942,1965,215,6530,671,617,2872,5135,709,7311,3244,3678,1857,4465,8558,13793,5542,386,16310,1911,2130,10441,1290,2565,3061,1013,890,947,763,4793,7436,5674,106,1499,1981,10536,5271,3058,1410,10060,91,7474,9137,7443,
3685,4477,8368,1079,5101,2274,110,2226,8528,4826,19291,815,745,2173,3307,5798,817,1230,21666,296,1263,24593,10068,9621,9965,8668,7498,7,5789,10364,14145,5510,18956,1147,489,4091,146,3745,9559,883,545,1097,2963,1625,4574,713,1758,3725,3302,6190,11530,2233,6217,17799,6635,9811,20174,137,506,23655,247,1478,3931,449,5637,2243,161,3597,8515,9857,10421,5223,2239,970,19578,2094,11281,449,9749,10105,1929,59,33,2266,195,7854,5387,19083,3437,4111,5013,2444,5442,2839,1479,799,693,571,126,307,205,3837,7781,
6036,8969,483,555,3593,35838,55,594,265,10451,1285,2771,1314,3207,8218,3501,339,263,14423,1157,610,5861,4445,12435,2614,8441,4642,10842,385,9606,3963,1461,37,4786,253,1733,13,5727,341,1693,7494,11075,385,15670,2033,579,927,16444,151,2563,3114,7149,7964,6299,12218,4604,1966,522,7119,36972,178,5270,443,1483,1793,6810,2675,9798,202,3762,621,39118,2367,333,2907,3710,945,10161,12315,3663,16115,8345,755,1997,1895,6485,17010,1463,33579,565,24958,2113,1834,20989,603,1431,13,4211,1453,10090,18288,12109,2378,
1363,26914,1478,797,654,6415,891,1722,1923,2270,235,3859,469,4207,2516,1119,1630,1159,2814,1483,37797,2774,3037,1861,1723,9514,9593,1583,9644,37,8409,513,3809,111,2005,1054,5804,1571,69,1921,7069,2298,3473,1071,18609,12922,9594,1582,218,3931,3317,2487,2885,25991,333,518,7999,1238,8163,99,8469,2153,12751,5492,10292,40456,1251,339,951,6814,6401,1373,2977,425,755,12431,3399,1378,8745,14073,1154,8391,221,8023,9781,4166,8607,391,8802,6589,9945,8424,4347,5771,873,1769,16271,1006,7858,21522,11540,4867,
7687,27993,6762,773,2845,4705,1927,9806,2099,3683,9130,3226,2873,611,1050,70,13307,7191,1775,9649,101,949,2231,1745,914,2445,27670,398,1090,6870,1097,3467,1143,12770,1649,7643,3013,315,11541,3995,412,2310,197,1175,8302,9892,4213,339,34843,3131,1079,1442,1443,5517,935,9399,2454,1661,589,1869,5577,11259,7527,1651,4011,9601,8264,1486,2498,787,20393,5443,3483,373,38723,8183,6503,1457,2175,10699,287,9257,21455,2033,443,10893,2977,349,1869,2195,23932,3242,1740,651,4413,469,15581,23742,16439,4841,1199,
2731,2005,1258,442,753,446,8255,741,2435,1526,7585,4383,1647,573,1429,1367,377,351,1693,3854,3255,6761,2433,10323,5111,3767,869,1501,15477,4473,11073,13655,1363,18339,549,145,12270,1601,2575,20056,1162,603,693,2659,3210,6107,3213,6453,2565,5529,883,4701,107,6318,378,8884,5825,8424,865,8293,11579,301,838,303,137,6474,3563,2793,2533,2293,153,10827,9226,2157,1507,9695,2434,8934,5081,14100,18396,1914,12594,39,1886,195,1605,877,2821,19912,23,2777,3306,366,8418,462,2843,786,6381,3837,510,2865,358,233,
913,2607,663,2691,3669,2069,2543,1478,7780,149,1769,1547,3900,1958,1295,1745,1481,2619,7594,7039,1428,7438,4007,9686,3410,2357,44,9595,217,3751,1911,19066,10085,3083,8634,2681,2609,1545,9774,129,114,71,11211,758,3751,19929,9500,826,2111,977,13772,7941,1831,28299,12462,10282,4159,1547,2317,527,1893,3862,33473,11277,3342,521,28677,2994,1870,975,26609,321,5303,7943,1166,1477,8527,615,10684,1117,981,209,1617,5453,4364,3950,6922,12727,14739,34165,12715,954,3733,6222,1595,6119,239,11497,23844,1489,11647,
482,9127,2717,370,239,1247,1374,12559,10695,1153,2619,4120,14839,29918,6411,6622,8660,9829,4222,1203,1663,10390,19209,1959,297,2650,4838,1291,1147,26050,14069,6563,2162,5277,4979,851,10544,12007,3163,4365,3978,6162,119,290,6039,5099,8609,17077,4610,7529,1063,178,1147,643,3651,438,4334,142,15128,1537,1839,1905,323,4927,9408,12163,1026,437,1559,15830,18505,330,1485,47,73,3547,4286,57,521,15517,1179,41,3329,17751,7325,319,2797,396,8359,7482,9615,10750,3532,3999,2521,4327,41,10332,257,2238,2574,14066,
14582,23,215,1391,2182,3489,526,2950,13569,609,8707,6325,842,118,1507,2568,719,49509,1771,1926,818,4733,2938,73,449,19066,1393,4955,526,2447,11433,5590,1349,951,4233,10278,118,14668,1707,561,18388,2067,346,2078,228,8198,1066,690,3205,7863,5073,4511,1291,5861,4475,9566,4151,5971,1479,6073,502,2351,15292,59,1502,302,14837,1150,621,783,9658,2594,9436,11140,8083,8614,519,5522,7827,630,15445,679,9485,10167,6249,4730,1773,8082,12915,1305,8127,17144,4817,3245,527,4601,663,8408,3771,7363,683,9157,157,9047,
798,1119,509,12457,2331,1054,1251,3111,3946,1974,2898,2307,1665,5749,1551,2062,1069,1065,3853,903,1973,249,30830,3182,130,2056,903,2226,8025,873,969,205,2095,1569,11114,1791,46,1357,5907,15633,4738,6515,865,5613,6547,7527,3865,440,5559,6674,27439,9853,1223,221,9047,3986,478,2309,3015,1299,547,13990,10117,3494,2622,8946,401,9050,4762,9835,1187,1405,801,365,4129,261,10057,37933,15284,1407,8756,8206,2670,8181,337,775,5243,374,3379,323,1534,1638,10553,4343,977,406,95,1355,1271,3349,3329,1405,6513,5779,
86,22289,25119,1455,1095,20870,1054,12848,26989,1823,9005,2485,16240,7718,2326,4115,43,606,2029,3434,4470,1638,4081,966,1569,389,55,2435,9878,7378,3229,11218,478,914,1634,607,1391,7947,2753,197,397,2571,4191,5317,11120,1269,11016,13372,1395,463,9008,4741,91,1499,10647,1035,4638,10893,9570,1175,3185,934,161,8329,25714,19077,2279,3483,4573,775,2822,1111,9282,1661,621,8741,982,1081,534,2885,919,9879,1685,629,6489,4671,10515,3185,846,807,2626,5662,958,2963,4065,1355,545,1423,6246,170,153,215,43,5849,
6989,550,1054,1555,4430,10544,40324,783,17382,4198,1059,1422,354,2301,7095,2305,398,634,10147,1453,2782,577,9793,2255,290,3839,13838,14932,11919,6987,3535,89,973,93,1695,805,2687,4051,3551,3989,17180,2749,651,7627,8398,4199,2606,721,305,8642,10101,4439,30733,3602,3771,1902,2987,1927,2687,4049,8213,6307,11400,11612,818,2777,4021,40185,6593,13200,670,5985,11212,66,890,3411,437,13445,8749,1379,1879,2138,907,1421,3261,1703,10645,3275,1079,9765,4679,1261,11870,8414,87,7522,7039,10589,2616,298,1249,6941,
10817,7317,2509,9977,1382,179,3503,2658,3961,1249,473,31983,9819,6623,19379,2263,37,66,4758,1301,1625,2451,20156,10907,1893,1986,2964,3455,12563,1987,8673,6974,13852,5550,1153,15110,4797,9311,9230,3799,398,8447,8494,603,2129,3917,2338,2058,6137,6267,4734,7665,1462,10183,2274,4783,2073,8207,14275,9197,247,1555,1602,179,1053,1190,15233,12075,14577,2951,1409,2737,1745,11054,24941,2854,9399,347,1019,117,349,4095,6808,5281,909,2311,2203,513,1833,31731,8459,1971,8550,3382,4123,683,3577,1037,6875,8483,
17092,483,6993,633,3399,218,1459,1630,829,10455,403,8291,24536,599,271,27199,3023,8503,626,566,9493,5238,3781,5567,382,12526,11596,9404,10863,8311,2862,1102,89,3857,15788,3081,1827,13293,1457,1949,4183,12474,1325,2237,3813,3927,701,1405,87,3258,319,5803,1731,8426,542,423,2247,9539,4690,13762,8167,379,3531,965,1881,897,2279,21513,16587,11605,5462,4619,5099,33763,8591,437,4849,626,3397,889,74,3779,8476,13412,6711,475,5186,859,2414,3302,725,823,4527,10127,1098,1703,891,15887,1683,9632,1137,892,1935,
34001,1955,5393,1685,513,644,921,3634,1502,8676,5257,783,4599,7149,6567,1343,844,1326,2874,6733,3410,26710,10845,3782,16851,510,106,8566,2170,3293,12036,4353,1746,4006,15690,421,2079,8915,59,82,905,3715,11867,1213,3739,4747,1535,1207,4710,1737,7055,2173,2261,10510,11921,2098,6698,12883,887,536,2729,869,4879,5786,794,10244,3434,483,19650,1141,401,898,1592,2650,7205,1338,1007,1471,10729,2565,1218,11544,1519,2522,1075,611,3049,16378,15818,1891,3203,4234,373,5363,1010,598,5794,1187,2358,9752,4490,901,
8639,15994,8821,830,4233,16956,2121,157,261,133,51,4818,3094,354,9414,2271,1746,9036,401,11165,14693,1175,4198,9202,8891,7081,906,145,2643,3571,1633,1173,30191,5135,4201,19172,437,15320,4420,102,2547,3655,3121,535,497,493,3994,5911,1498,143,7135,9386,3038,27732,4663,13104,1295,1899,2709,12022,23117,14947,627,5470,4340,5787,869,929,1575,11354,157,962,6809,9877,559,923,5923,1269,13261,2367,4801,371,4179,903,509,2082,2481,655,233,6782,8979,806,743,2678,1462,13515,6119,14899,8872,7517,709,1010,862,477,
2621,1429,6131,14911,603,133,19164,6842,603,1022,76,1987,2631,4965,747,5474,1227,371,11016,9862,6775,9580,23309,8884,20046,8318,1342,37,2022,101,2058,8226,101,2442,29389,1482,773,1957,2685,325,6044,11425,1007,3265,4095,907,2941,422,4641,1177,2333,32686,1473,2655,10225,6331,2323,793,14119,15186,6557,677,991,606,11522,339,717,2118,1702,5409,2968,2147,3555,897,8343,3057,5461,170,884,255,111,2145,13382,3657,4548,13812,6090,8519,9254,10108,3210,825,897,23,2726,10008,923,537,4318,1724,3484,585,8085,1747,
4078,167,16127,2649,16040,519,1295,1194,1353,982,1653,445,525,1501,1183,23072,894,15270,18425,2013,2863,2245,1017,217,3041,2813,7972,9059,11248,281,4274,387,1730,8271,1435,4631,10011,443,457,6991,1198,266,554,6550,3505,203,19580,1686,24714,3185,29,3394,6999,543,7750,118,3080,1661,1205,381,13694,157,1593,122,2641,7385,24921,9386,2371,762,171,4278,78,8697,787,1078,789,8794,1066,7898,171,9777,415,4623,1385,159,22692,167,1395,253,6791,957,4831,21354,3745,1113,1935,1033,1861,7304,7317,8609,402,2402,8974,
26162,8318,957,3076,2687,234,3449,869,2445,267,3605,787,771,2327,4058,1871,13607,265,2006,1626,917,6579,14284,13292,990,15425,12117,4411,5086,29949,67,1590,627,3769,3815,2939,42,1311,5855,2161,8900,5663,2434,9405,5305,8730,339,3289,934,190,2254,4565,17751,123,8771,982,5857,1009,2221,5934,1842,4985,7406,9149,8689,34907,4286,331,559,2623,774,1165,970,3479,34963,2793,906,18296,547,439,29,5199,117,645,546,13966,5634,2391,589,6619,2285,1021,1547,14562,3798,2695,2314,893,970,11437,55,1123,15025,9319,130,
333,12497,2089,10216,4311,4043,273,6423,2477,2591,1563,2211,1689,6558,1689,7411,6686,4459,24641,755,7035,251,929,1465,1749,595,4395,3971,6079,745,4943,9910,7731,699,299,521,2338,2753,9228,2091,2837,519,103,20129,1077,329,3689,3483,1993,10022,330,9251,4078,19689,9570,1061,1953,2710,1019,7571,7945,123,1399,2015,1266,3545,36069,85,4993,10192,4195,1047,1322,2565,2638,257,2999,22664,709,23,24060,491,5907,7529,55,8134,10110,5731,10379,3414,195,10983,28705,3445,3341,9242,8220,369,2142,8873,583,5713,2263,
341,6426,2917,133,7606,27717,8690,5107,5495,22065,377,6202,10001,15505,3742,3766,1913,2819,1825,4799,5377,7385,1533,2129,10118,8711,29298,1316,11677,54862,3514,2235,6815,8121,2293,2810,565,6410,1130,9130,489,3022,1671,4491,3790,1395,15396,1222,6070,452,9601,13623,6935,4118,862,2438,3854,1604,12305,2750,273,22,13840,127,9788,4054,3689,3286,935,6019,2518,2525,3245,14826,9275,14709,2525,14227,12697,2390,7149,1991,1565,8038,2893,1261,5947,181,6799,4973,3791,3478,5197,1531,559,142,23379,650,206,1355,
129,11545,10962,5726,8517,9162,5467,407,2931,5341,1197,5473,2290,1537,6024,133,2419,17301,73,82,806,2918,11242,12158,3301,182,7227,9626,37807,7674,3097,10453,31919,9515,297,11208,4826,2164,3417,11280,1122,8042,1173,595,847,993,8297,274,5887,1281,8320,5281,1438,569,107,719,26808,714,3915,2925,205,611,339,4917,14016,8115,682,7423,6747,7054,1821,21073,6905,1043,11643,3327,8373,1130,520,1642,1287,1382,2225,342,723,741,4385,9087,719,3810,1330,14038,302,6681,4102,433,1119,16577,2247,6009,4599,2821,2589,
2307,538,1418,2249,322,27193,4115,2516,20100,2807,20535,15819,2518,399,2030,182,24333,14385,3166,6619,8963,1902,1837,2615,822,773,526,4723,2726,373,5131,771,6095,2927,3241,943,367,1831,901,298,12393,3354,2374,3178,979,458,5953,2287,4407,1166,3751,114,9393,2251,863,7979,277,13168,6229,909,1986,1417,4259,12953,1570,5479,2567,7471,1787,782,851,611,3318,12418,4990,2066,5559,9621,13700,4969,863,1769,1179,235,1809,797,11136,3561,9594,505,1368,330,40770,1529,1782,4317,8371,5947,7619,411,4593,1391,8270,
6290,9685,4295,526,11247,36028,3471,731,7501,995,2253,12660,1621,6662,568,6054,577,4076,23138,3485,63,1270,9499,6019,365,871,11,11477,2206,1662,542,13541,527,2054,1037,3369,7314,7799,7403,307,807,3714,10753,823,697,239,1453,11305,2349,8890,7953,4971,23987,3823,3091,5555,15859,1689,697,3621,19982,8050,1491,22508,1462,603,4349,3903,1179,1283,1418,2267,3537,689,4962,1059,5890,2267,12155,4865,1365,26935,3195,191,40521,9127,27623,67,2902,8511,47,15914,8984,7613,3511,8678,439,1554,2526,73,2874,2471,8861,
117,1124,9084,6043,7698,1035,2366,5009,515,24582,2229,263,1713,5391,21108,46805,7343,1199,1342,813,1063,173,2243,945,1517,14286,321,7561,1683,9676,76,1202,8700,6605,4551,1847,53,322,4597,1023,715,15929,2643,11978,4383,4773,2825,57371,61,386,1548,2677,5291,2655,38702,9825,587,193,6359,1651,9860,8989,14872,7036,8038,3877,52242,1043,4583,6571,4914,18419,1962,17405,295,8411,855,779,498,9139,3685,14424,19,8131,29477,6331,681,1050,2041,9256,16083,4615,8365,2885,3014,975,1553,5759,29006,2623,9574,3859,
1769,3339,3330,4265,271,38434,39,5898,26227,8563,8734,9987,2967,770,8305,14856,1109,9109,553,5818,17466,2567,8540,6623,1227,4242,2739,1911,2105,881,796,12709,923,4709,6851,26911,895,5179,751,182,2305,11650,733,3442,9766,3203,8386,33,1385,2069,6731,9040,12617,3926,91,3519,3131,1441,1319,2097,83,1203,4780,2506,2287,5829,2179,3211,7382,3434,1045,3734,2990,4739,3158,63,1939,1314,2790,5827,7193,1207,2018,1079,1033,4105,1642,7371,7065,1098,1475,1674,2930,5427,655,7946,9651,2951,766,397,8643,4979,341,4925,
193,70,3589,9921,19,775,14671,3107,2909,1073,170,9447,13390,3803,8178,10608,2763,3150,809,501,1636,8811,2163,941,7045,7770,4721,1406,7862,3735,3573,1651,3293,822,373,8534,7869,505,5085,2569,2106,9380,4382,942,1409,6689,5697,2103,9774,7094,4459,399,8650,4527,666,2967,1019,2741,347,2494,9470,4011,5945,5370,3891,6207,7117,3073,2210,3809,10774,1379,14140,7123,1035,1149,238,747,6076,849,915,1810,161,20518,2266,3431,4089,6851,17348,1143,3077,281,17372,903,2107,2633,182,997,467,489,495,333,7678,1497,269,
12609,33,1443,1631,397,2554,499,4550,286,741,193,2967,2695,5362,395,3778,1329,10132,13834,6215,311,1778,4626,13933,62,465,12052,9853,4904,2373,274,937,253,461,59,2686,809,4594,9303,2838,446,14486,2917,1290,175,7907,285,1830,26995,267,1743,1231,219,3815,9634,110,6751,3519,1168,4957,2945,109,2001,5042,9565,4059,2072,4457,202,3441,4363,839,3615,9345,4031,2161,618,3035,889,5209,5474,3967,154,538,2181,7118,1941,3763,9393,453,149,12456,431,654,145,1274,226,65,5459,649,779,13811,9822,47855,3087,1505,342,
1341,2685,801,826,2171,32511,1059,10653,1135,345,16271,2538,593,9058,15071,3223,5238,1003,5701,17488,962,8803,7137,222,804,2609,15770,1029,5527,22081,586,1390,5445,3922,8273,1414,9341,923,433,28522,1247,5179,9045,503,4109,299,395,2658,2909,6078,3640,2549,1189,5919,17749,12627,843,755,374,799,25971,10758,2713,9154,148,747,342,2669,1678,7153,1162,2978,674,606,435,6027,1357,355,10008,3901,10330,5946,2410,8216,1966,20319,1683,6781,449,1011,39,3111,5457,3910,2709,8531,9421,7577,14622,322,12918,36524,
14046,321,2799,1917,1801,6190,3247,1590,3278,10549,6709,711,1317,927,10035,4067,1189,670,5245,7286,2139,6783,694,2423,9812,2301,9187,189,1469,409,83,9407,5927,1486,19888,127,5817,991,4534,355,10037,8431,4747,2491,547,3035,67,2035,14503,290,9555,411,3942,4574,3746,1699,3653,12261,5837,8398,107,1653,202,7094,1543,1318,4403,1557,899,5134,5898,4954,2861,370,10007,4438,8844,1322,1806,15540,651,985,10316,8791,5265,4358,11917,2234,1419,4619,1579,1479,5183,2597,22,5591,2925,2552,13786,1319,10397,5417,1287,
1463,218,4971,5157,8675,1923,1683,1529,2713,1510,683,3841,4043,711,62,8435,829,4775,693,3107,4997,339,1810,339,1437,2303,2422,93,249,10867,1689,609,1162,2697,394,6253,4465,1863,337,3914,19686,2294,2227,1637,613,10573,5094,15483,369,1635,6043,82,2030,6468,9803,1597,6758,3678,1798,491,12148,364,1381,789,6971,2105,95,350,193,13234,3180,5201,6044,285,1406,4643,741,8730,527,4091,15388,2692,15028,74,13039,1635,1853,2570,6139,1741,3747,4045,513,3679,286,2635,1950,1901,4954,3043,232,2469,8470,1575,1642,
3347,1145,1674,1194,9533,4781,446,859,385,4193,1562,73,660,501,582,2125,5691,13373,3372,5429,3775,2431,2269,13287,16654,3438,6238,911,4030,463,3110,15501,210,8399,6351,11563,2139,122,1478,15307,7661,11932,7553,3197,371,23131,5229,3315,1766,1506,10929,7162,3260,2466,4934,5685,4899,2981,29,2378,913,12329,373,185,1462,425,1767,9326,33,6063,22818,4217,7999,8355,20228,10667,49583,3447,1522,489,20148,5467,5866,1309,229,239,862,3101,431,3745,2894,3298,2449,8454,10880,1277,7015,2743,1229,1317,2451,2079,
4006,8210,1374,1179,1229,1717,2333,7389,395,5745,12014,4018,317,6447,8587,9440,1097,515,10104,1673,2607,181,10081,474,6927,10056,19017,2350,3609,27185,9227,207,4201,14624,357,3111,58,1055,9607,2949,1101,4771,30703,2630,79,3587,2342,11631,3582,1517,230,1165,6773,2077,35618,649,3522,1674,1654,1031,233,14118,1574,8877,3257,10400,6959,2785,7715,6649,2707,274,833,9277,624,3514,2979,247,5197,8459,889,18575,1851,8337,31,1703,1442,17048,7729,1969,1351,10114,1443,1165,807,1262,3867,24831,1390,13939,4021,
2442,8577,26737,5110,3614,257,2455,9174,2027,7903,1743,129,1209,12840,9274,8019,1902,11917,371,1703,1746,8897,8691,38333,1537,1202,1954,483,9852,7315,1978,103,2598,446,175,8149,2186,7787,155,5077,1546,5986,6349,6025,385,3679,8377,5899,693,3029,966,1047,11,13947,14610,10278,4586,226,3171,2379,6511,31,10692,9355,9250,9359,20510,487,998,6487,8936,3247,840,8678,89,4955,1859,2054,9528,1015,157,458,695,130,9122,19072,470,5157,7117,1286,3333,11906,9496,123,7438,6563,1599,6547,538,8399,449,463,8099,3014,
2226,15461,47,1281,7461,1115,6970,139,5646,1105,14044,2951,5369,8489,33253,4758,13469,309,2838,1859,2465,11666,1277,11864,4887,4111,3866,1409,1209,3857,2147,1139,3623,14799,7041,3411,438,3021,3023,9574,3577,723,2955,161,1951,415,682,4398,4971,419,17664,2639,1377,10449,103,4559,27115,28468,4417,333,14904,25458,2102,3642,3749,2329,7471,3559,2226,86,1206,8334,5271,9915,6105,587,26801,3914,7399,6322,1382,799,74,7013,10693,274,12410,7763,354,1063,183,1347,1349,3638,3326,15180,2174,1192,1446,3497,2395,
3117,6647,5111,595,1523,443,6054,2631,1886,6549,758,4709,1294,1123,5215,137,335,1334,1333,4810,4478,438,2387,4031,741,4401,2261,1241,411,403,2331,171,814,8354,1525,6051,1418,4607,12084,2103,4249,11626,3098,21244,4325,7061,2109,1735,213,833,473,126,117,5229,7151,39,1859,8443,2471,505,4754,970,1820,285,47682,1897,4869,4187,1113,13768,3735,935,1139,8229,3197,994,1302,2371,4333,6110,15553,1267,179,285,7429,667,6645,895,2811,14,140,2318,1386,117,11149,2318,1189,215,4781,12607,8236,4958,14578,8654,37,
1309,2289,1113,8538,105,127,5318,11727,8730,1439,321,6133,1395,367,2613,1877,6071,10521,1795,8359,1005,858,12404,2453,7490,2703,3889,2066,562,18950,15869,703,3466,5883,257,5393,10612,2402,2633,362,783,1786,8408,1473,813,27802,1706,9446,3063,2354,5995,3577,14446,4817,2629,2946,254,145,11371,5258,2979,14220,147,1915,8505,1699,402,10499,95,359,970,3731,4567,990,3495,7116,5419,1935,1086,6577,3277,185,9799,2551,15059,319,5918,7501,11205,2125,8309,10646,6881,3474,348,42908,970,277,5022,2129,377,9896,6943,
443,9066,1255,1318,1833,2119,18553,9716,10022,1094,850,8320,8855,3291,662,2202,1475,1762,5257,986,1603,2743,2941,2489,2549,6863,8133,8137,3117,3815,5799,779,766,8752,2351,13977,16855,9069,339,9230,9328,9203,6742,3335,4002,1459,2635,3189,1229,731,3107,1541,154,3545,13304,2931,4343,655,3507,16263,6987,533,15688,12163,10170,1269,1877,1293,274,5739,5521,2046,7775,24676,5727,2110,10528,85,13,14210,581,1510,2146,33,2202,1519,527,4566,902,2748,7114,11962,14540,22072,801,12324,1862,1101,5259,3679,1929,1333,
4989,34918,3023,449,4433,3795,2763,5457,10021,2943,1785,11181,3007,1543,6429,3363,5315,11071,12203,1361,370,2358,34741,782,2165,4307,3890,12440,2929,4013,1451,5279,4910,305,12146,4497,10214,14631,4479,1974,3963,2606,1333,1582,1640,1222,10582,2529,1233,3127,4821,4143,2310,1015,13400,9325,34259,3179,1594,3513,8826,4104,9417,8449,6017,10718,7563,1251,5756,5469,15173,1330,1554,1321,1302,3283,373,109,3810,4155,5849,3389,1971,8506,313,10045,617,743,357,366,7485,6342,2243,1859,8690,157,15486,1967,2010,
3451,27013,2866,485,7083,1282,4794,573,2162,518,16566,222,789,3109,4983,2722,527,391,962,4185,9480,6005,1413,2177,17510,3994,10121,399,4138,5794,251,12083,517,2075,2357,8111,339,7375,15860,8574,2243,14859,1407,1155,8262,1897,3062,3141,308,8384,9033,1337,2249,254,374,1998,11342,11044,23,413,3635,504,455,8494,95,6147,2395,247,3305,6891,669,12226,43544,181,9971,2533,8568,4697,230,1511,133,362,727,2869,1003,10855,253,402,4582,11298,10929,215,7209,10601,482,2085,3455,9761,233,14870,8417,1731,621,12763,
1134,13976,306,4354,6499,1694,2350,937,11305,991,11800,1299,5309,873,1357,5909,28187,181,1335,3858,325,9707,318,2493,7815,3279,3321,511,2138,5035,11146,397,819,1717,2557,14642,9775,4054,2024,8979,4967,13415,1105,8972,35,1446,9868,73,4383,6711,2651,2109,1046,1257,499,2247,6835,1755,7941,14130,21203,8469,6782,716,1747,7894,2241,2301,9206,5782,5430,5742,899,1347,602,11241,1621,74,863,311,1693,626,1794,5243,1223,1423,1763,8996,2067,12458,555,6965,5027,1987,5026,1719,953,1341,654,1988,815,11669,2947,
4055,1173,2525,783,6595,9856,3939,8645,813,3877,2174,6513,4214,537,301,6850,3485,7623,517,18742,1385,399,7826,897,33146,7021,6086,1174,3115,4278,8094,1289,1657,2578,2241,39,39974,582,9018,2993,3378,16527,3597,1349,9484,2202,585,498,8213,937,19053,554,637,4999,318,199,91,791,5794,1762,3205,10056,8886,2163,9902,1143,1667,5551,6703,8784,741,1378,11076,1528,766,7275,9554,6333,3571,5218,1687,6631,9539,473,10893,5802,342,3086,5474,14916,878,1213,3157,4025,9185,8399,7093,21371,2930,8555,21134,4923,1598,
9528,2117,8697,5813,1262,5351,14227,10024,16760,8309,5398,322,74,9981,11504,6095,1874,37638,2115,2346,326,13138,25275,1097,10315,12230,489,333,11167,739,3641,521,1621,11779,7235,731,677,9663,1255,12508,634,2950,16475,7138,1261,4559,7565,1357,4795,813,1051,2277,12431,4955,611,10575,26482,1043,12469,7439,6783,1498,5509,3195,7370,1281,1163,5322,16860,1861,2267,1833,1163,1806,3181,4579,2411,1821,3117,7659,1698,438,9745,533,9273,6177,2862,3011,1177,2866,5391,3406,455,10889,10236,37,1467,1603,1107,4933,
974,9404,115,2141,442,9723,2315,13,7718,971,481,4615,187,6898,3458,3961,1473,9222,1870,1485,8338,3930,522,14163,12509,1854,1241,7301,12462,8254,1949,319,1282,10822,31018,1045,4810,8198,145,13982,13441,7245,3414,2269,10344,9899,3757,1861,674,1467,2232,2005,2225,2241,153,2397,2573,61,1861,1285,8889,13781,8672,2819,13866,541,318,19373,319,3003,1061,9303,954,2493,1817,11212,2577,858,103,5003,571,1693,10243,2331,1097,9877,4177,9255,3050,8260,18974,5963,3189,2823,2967,1418,11386,6188,29154,7939,926,5549,
78,979,4338,259,2330,2666,5901,5187,803,8222,86,9079,1271,5391,4279,3798,7359,1385,487,4771,67,8175,941,259,11494,4110,5379,1206,522,1890,226,24149,4011,114,1707,261,2515,8358,1771,850,365,663,19574,2482,8724,9203,475,563,8925,830,4037,4973,7211,291,10869,15689,234,482,1093,13607,2363,3114,3465,93,818,1649,1974,1351,2402,381,681,1940,534,4699,3855,1233,397,8475,22934,15726,523,7653,3369,9177,1997,3014,3729,878,7231,2294,13194,31,2591,4718,213,4235,5257,8179,533,1405,1127,5673,1611,727,153,531,1053,
5354,4415,9494,1015,5933,1461,8264,3913,2737,2509,199,102,3193,287,470,9547,1543,1546,4745,4959,8644,4069,12063,2805,3130,834,246,3911,751,5963,28947,129,3615,43,37,4453,3,3826,1562,8399,1434,730,6961,4977,2473,15820,8329,1530,623,10531,6653,21731,731,1074,1041,1430,9721,2921,251,1115,222,8393,4211,163,4258,7385,2533,1499,99,2203,1407,1357,41,2509,6949,3145,4003,1446,7611,9415,5447,817,5207,4325,5717,427,2007,8159,87,1848,15378,9005,5170,398,103,9519,13692,4015,12125,3178,3286,8779,1687,563,9770,
278,2129,25951,24657,5030,682,3250,3743,5659,34,3623,1939,3185,11304,58,17944,1886,166,1002,5593,881,443,542,1714,5301,1689,1438,3694,12812,518,906,850,755,6251,2649,327,2653,403,8677,530,110,290,5645,2695,670,8318,3149,1958,4705,894,1503,12507,159,177,83,609,7035,2915,6483,6749,5694,139,3317,149,3515,1162,2577,11452,14934,1779,1510,1052,36540,9387,743,3879,1335,5123,8486,3767,183,7522,1449,33329,637,941,7581,449,6114,609,938,1594,5627,5361,1146,2253,9878,1273,2737,770,2007,4474,8346,373,3615,14282,
31,755,6034,5438,34365,1622,7157,4298,11043,554,15964,1677,1507,9169,1609,24594,3745,805,387,4457,5891,2158,10954,1699,2229,3695,5958,3255,351,1642,4079,8827,762,1062,3016,8501,12296,2874,8213,2549,19964,1306,4187,565,3886,3563,12387,214,13743,2795,11443,2067,1741,3753,234,691,351,7268,253,5287,6175,11736,2341,1071,1395,3525,8719,4095,1546,3585,1669,1171,3997,6958,12762,9308,664,1009,1775,4383,5635,1141,1313,2259,2635,9023,1537,491,1038,1373,2005,5274,4719,351,5597,241,2167,14539,3814,3666,2113,
399,109,1929,29930,2647,1142,1941,781,7114,2743,11414,1415,217,83,1839,5764,1535,21484,2461,1249,3463,4701,11678,1065,6953,8981,5141,5990,10440,874,7163,2053,9647,522,3565,919,8959,3321,13,93,9736,2883,313,860,4565,27869,1610,11508,1581,24114,10586,1162,849,274,142,16797,303,7073,8222,1315,89,2929,11501,4363,1790,671,2520,366,617,3349,1061,14962,15408,1127,4202,8493,1767,765,1353,2318,5579,1290,13840,191,797,1357,3559,2556,13705,2798,277,2725,2138,4227,4517,1731,35782,7298,7102,863,3449,8479,8531,
585,3003,2103,13725,254,2319,6935,11724,10846,341,13048,25829,1686,539,6523,1769,1466,1153,1866,203,24088,503,5403,2510,7045,37,14181,3629,751,1469,8400,4935,13354,2515,6118,2949,8357,185,2826,21389,1053,3885,606,3179,334,379,4595,66,2920,2687,8827,1705,2091,691,1930,4435,921,126,219,434,5950,12683,3305,7778,7283,9113,4038,122,17279,794,10620,1289,509,806,8848,11960,1045,11377,827,2442,26077,418,21117,745,9134,1895,13771,17855,753,9024,3451,743,3651,406,9335,5394,1034,642,1546,7713,6319,8194,73,
993,19069,638,337,4329,833,7335,926,7659,4431,1434,7913,8343,1373,2666,7413,1837,9301,8685,805,35769,3923,1338,12778,1175,12479,754,5270,8470,6629,1570,9193,28873,2251,9362,10675,201,965,185,39,9860,6935,12368,9949,2042,4617,9860,5543,1401,9161,1878,12106,23743,2426,366,194,10630,8183,6882,7969,33211,1563,3571,339,11217,7671,323,1006,185,859,10823,4977,7689,1465,1098,8895,2021,15633,1271,12530,6558,152,21710,1373,5803,5673,8923,826,5195,8125,475,1722,3049,6012,9371,10213,1878,10139,3818,11028,1358,
3225,21639,63,31238,1923,178,15822,3194,9818,3474,1535,3391,549,993,62,1049,2274,2982,8191,473,12698,2710,8198,198,3293,313,985,574,3432,1165,7910,25189,2429,3901,11373,1276,4385,173,1431,719,1733,1107,58693,1946,4507,3369,2902,2870,1086,2066,1630,4635,3601,291,749,2721,4393,2403,2015,7631,5574,1614,311,12052,247,1387,15612,3429,2279,8055,9127,70,1149,586,2687,61,399,4154,6019,4625,1623,499,7142,429,4645,20059,1099,777,369,1806,55,9257,3230,13910,1927,2731,1443,2686,5586,265,12276,2977,2143,481,
11223,598,12552,811,1095,762,1067,551,1103,4495,12450,25633,1535,714,258,2525,8162,2046,3464,237,13,2979,1179,1105,101,2194,178,2834,786,1939,259,6629,12329,1034,4206,11750,6570,6634,2713,1346,16681,11996,11214,4850,16490,11527,261,9615,16830,5179,6685,17351,1207,3638,13420,1783,301,483,539,3322,14839,2357,994,11433,109,5854,12331,7558,1133,7715,2877,8007,29,5167,6109,51,3713,3886,2241,8123,3687,283,5729,749,1329,4667,11070,2513,2619,4246,1895,1295,1311,235,3205,2063,7594,92,1263,2577,537,947,3959,
1739,1219,13572,2162,2386,11758,8349,102,46,6575,19440,3681,6986,778,2325,94,13322,231,2219,3030,4065,2717,255,3239,1226,82,2570,445,521,3449,7782,6361,617,3641,2158,234,3179,971,9821,2630,213,1357,11744,7218,14618,9085,5723,4921,8761,411,2266,31,8215,9098,6454,12298,4834,2882,2698,4143,2694,8241,383,1293,174,2077,15428,11321,2431,3311,3255,10483,5206,18205,1078,5514,930,3997,4461,1725,6194,2453,282,8567,8183,23128,7234,11685,1482,1589,24678,330,11755,35928,5194,4043,5477,2961,7958,2082,38693,10293,
10738,1042,861,1318,8762,235,3451,4866,6419,1751,313,177,8373,415,4187,30810,4413,286,10491,15662,9524,1357,4101,3955,3385,3286,7105,138,1915,9152,18705,2266,1525,93,9308,12606,1970,317,333,952,891,1667,222,11848,781,34257,183,477,855,32935,347,442,9003,4529,1233,9748,4632,35483,1030,5005,763,8531,9164,1222,5530,8238,3407,1207,1918,3746,542,5463,2412,7411,2602,1265,2837,13905,353,9441,407,1694,4206,137,859,155,847,10435,2923,85,18721,231,505,10994,16233,9906,325,1219,5835,947,331,17,2267,14496,6355,
927,5094,677,3478,12290,24969,9351,1214,2677,36327,195,1739,3115,6971,8054,1049,141,1034,967,4329,4101,427,4467,10061,11294,1270,9108,1187,353,487,16360,2001,535,263,1845,254,593,577,4659,423,4505,4833,1453,8669,1133,1642,7016,11332,9186,99,6065,367,513,3477,67,2518,101,623,1359,193,1443,3595,107,8126,6923,11862,1546,6570,7033,1158,4187,1190,635,1242,3709,3379,3230,7647,26233,3827,707,913,67,4707,1263,18347,1797,4593,117,194,4157,1529,8359,2403,9093,2123,197,2083,3322,2009,137,3829,1263,12055,1905,
2461,2381,653,199,9278,3018,3187,4978,472,65,25817,109,1754,5946,13835,2159,1202,5991,2193,860,1966,15881,36127,4651,21,5450,5322,3594,3007,5657,3441,9479,1087,9789,7582,5141,387,234,443,86,1453,11729,718,4166,9414,1757,971,10884,8946,783,725,8403,10891,4707,4298,14380,1867,61,2590,1630,2715,646,2903,326,1533,4302,951,609,2707,5164,3709,661,3170,670,4142,14086,11572,3639,11756,5771,881,667,4071,13385,9141,835,1342,3194,2267,8800,2517,10023,4026,211,4647,614,1279,6245,669,1013,14198,2565,8541,1117,
2117,27714,2236,4644,1593,57,239,711,11030,27875,2761,741,7910,11041,17570,29830,562,2321,4526,6113,482,997,2443,6934,1627,10643,1607,2847,804,5233,2855,4406,161,1846,2391,1011,4683,209,661,1538,3869,1702,15126,2438,4674,2766,4777,2021,1199,4526,2041,2490,9554,685,395,683,21711,21509,5977,1529,1873,99,2845,7493,534,8644,27397,8783,19343,9212,13377,3419,11585,9852,454,542,10056,502,11494,8523,2287,1171,1583,14926,3774,803,2915,29850,3469,5218,2631,32897,738,15454,2379,1307,11909,13919,8403,3407,1123,
11,9108,5059,11624,2082,4717,514,21778,257,4991,295,2305,1403,4197,326,14069,6290,8775,161,8319,199,197,1261,2273,4359,1262,3213,327,4937,2301,1607,2521,659,237,32386,1430,2735,11385,445,9992,971,8271,317,2286,3818,83,876,1954,614,10870,5946,541,9885,1349,5554,12635,1321,9852,7471,4407,4501,489,13175,1048,6909,2233,11852,3842,5467,4586,25179,3367,2073,1827,2757,3883,1826,3989,4081,1070,9441,417,3583,8631,8814,4419,5169,2249,10389,7831,969,3537,253,9048,6597,914,1218,3099,6513,13284,691,667,1771,
9396,1489,8965,817,4117,2854,1385,549,5465,8337,1453,13,1547,8019,4053,9184,4643,4271,8610,1338,747,789,4605,791,9188,11602,8387,286,21007,1018,2701,6014,82,7074,437,13432,6119,303,1237,149,34014,3806,55,18351,4697,8723,1495,1654,4979,3122,9799,3539,19723,5578,4098,741,189,1615,1627,3067,5378,2029,12828,11611,801,4162,3046,536,6653,247,7186,4619,109,6385,22852,1161,4502,178,353,511,1271,329,333,9194,723,9890,202,1159,987,23528,366,885,278,1214,1978,2201,14191,461,1203,2261,3391,166,419,32619,1449,
1613,183,23,17873,4971,19366,6815,1835,1466,207,8276,12774,2210,11613,97,8171,8280,639,1349,29552,9184,203,353,667,1573,4857,114,10089,1067,390,295,440,3801,3625,9350,6521,102,1913,1801,2379,2042,7193,922,2785,2493,1133,2150,629,2547,4501,2949,5073,9403,3589,333,8702,1719,2158,1061,4385,6917,23764,696,425,8760,2299,10706,5458,951,1730,10615,8335,1459,291,1469,8413,13188,71,994,139,2402,743,14043,877,1309,4581,466,4035,3858,551,2106,7265,1377,1067,2487,14933,1134,1198,1917,5742,215,1491,1930,8827,
1413,16793,305,5665,1911,817,15645,963,774,3115,8800,7402,1506,394,2319,4362,1382,9316,3250,109,14028,3714,9312,1257,3597,6849,1155,35901,5377,3695,481,2449,2495,3439,2997,2567,3082,6551,599,630,933,1847,1205,10562,43,8984,1127,4685,1196,3849,11071,3385,4711,5365,1134,9285,1719,239,185,201,7237,9199,3154,563,5405,1958,455,201,4063,663,10272,138,629,985,7550,494,15154,4679,14222,6329,69,1149,7817,5023,649,1534,8606,7067,24901,2865,2739,17569,1051,471,15756,3762,1700,1450,9968,11550,4019,11785,4823,
3977,11204,1219,175,26853,11613,8785,14340,6111,158,12594,7311,3115,1937,965,567,6062,1963,34747,4607,1519,8438,12579,1759,5878,1497,1037,690,583,1911,3617,10944,407,39,553,732,765,8623,337,3309,12455,158,1729,1006,1075,9761,12702,5989,759,3271,265,5103,5733,1946,4435,23113,3474,525,994,4627,5435,14428,23,10926,957,533,10964,8687,3094,5565,989,1138,1273,6429,24421,3897,1003,18936,6089,4043,15893,11195,2579,1442,6966,520,17582,10996,1645,129,6513,6221,8452,8697,10664,3347,4118,8066,291,551,1301,9749,
1863,271,2510,1767,410,2605,12413,3066,6983,461,407,2010,9108,4057,333,2214,521,39,581,934,8295,3406,2689,9172,485,12099,6823,3453,11032,3121,1419,277,761,929,1214,626,20282,1970,4364,1810,489,1202,4762,7474,506,9733,1319,1162,299,28650,4097,9107,6317,311,339,11793,13427,9010,874,9568,2719,2146,38,533,4274,3301,10134,74,1685,3391,2454,2550,5238,4878,13093,8878,4085,5685,4226,8887,1433,1777,986,803,11192,9400,461,6385,621,3094,157,1234,1610,325,1755,3317,55,1657,3179,262,2023,1446,8609,38923,8562,
473,12488,9200,1610,258,9010,1411,5685,1775,3237,3679,1495,20745,241,9495,1570,1047,5335,133,202,4740,4270,2994,5971,12360,3141,3031,10461,15909,4650,18960,713,10684,887,6365,95,7258,13463,7469,2534,649,3127,1387,3789,678,1034,873,591,259,971,10716,1313,4087,3322,763,6262,7501,466,44,909,5903,573,403,5890,14954,2274,8715,11968,7010,491,685,641,8330,607,5567,710,719,8546,583,7078,1366,1047,12233,442,2527,10467,14994,4039,2219,927,1795,12713,2870,5146,3891,2255,590,2947,26,641,1062,9131,813,11201,
1967,869,8317,654,191,1510,3853,15723,557,1411,4198,173,527,10994,1335,1398,3242,21465,1618,2709,929,3999,21022,4422,2245,9174,369,3682,1926,13789,35,7574,3367,4143,1434,3898,531,2375,9215,1194,925,7502,4486,309,52035,11213,4447,1337,3591,8910,353,919,4538,36686,9352,10808,77,3265,20242,1339,2427,3119,4988,1446,2857,1625,11506,12261,1926,1230,12498,4227,3453,1831,3431,187,1793,457,15834,6655,2041,473,8765,2278,9182,1513,32409,3229,2594,3565,32307,1995,2783,115,3257,17808,8458,959,5770,13365,6771,
67,2414,8858,2447,1322,3619,310,1229,5367,1371,33319,2037,3861,8487,6777,2595,119,1767,1754,3517,1679,118,21137,4282,2134,2306,2550,430,4069,1847,12552,1687,16639,6466,3829,9885,6095,2787,1205,4515,1221,171,3137,11388,557,8453,2828,293,8307,1951,32179,2206,73,13694,17540,149,4817,17271,3766,9260,27874,2378,3194,11078,927,1283,3989,4243,221,6182,20619,3758,865,3489,16486,4023,436,1611,978,187,8873,9285,7306,703,8992,199,12971,3270,1991,10021,266,391,835,1393,8870,27321,4105,2651,403,1266,8349,2578,
6147,4313,11667,7438,2173,6533,11817,8675,19495,1518,1978,943,7065,13299,2543,311,1437,5901,1819,1862,32323,5047,385,65,543,1802,4861,1383,1902,9700,451,705,1943,3909,1219,4534,1573,1621,6085,15544,3713,522,385,1270,11328,1685,11479,12606,13831,8928,15019,2023,1597,9531,2921,2239,570,1107,5326,386,16087,5582,111,6098,70,2837,11720,4067,3350,11065,618,13184,2763,1229,1419,19658,30146,9472,12700,1133,474,9130,1639,646,1425,3158,7254,1903,6823,9588,7705,3413,2630,6451,2453,929,12074,1658,3794,581,177,
1018,1469,818,2097,1178,2142,12607,2853,7229,539,897,20441,618,951,2342,2317,926,1929,599,1897,13327,1278,2995,5614,9756,403,3315,26191,5956,4213,22599,10279,1010,226,1988,609,8992,693,2898,14284,6033,4327,6753,2451,17639,489,5605,5105,2142,1705,13250,12015,829,2873,7779,702,211,1038,1539,9882,5298,495,7559,315,9483,2875,3093,5559,39349,734,194,3479,761,3503,2931,1179,40618,140,1559,2097,8609,340,247,40061,9475,195,4533,2411,183,9202,4486,131,8973,819,719,2003,1435,893,2627,61,419,3047,434,5214,
5089,763,2919,985,2449,1197,622,797,2481,5119,8502,3959,861,9035,4837,1374,6159,5009,881,1142,6305,9785,899,9258,13854,342,1171,8799,8549,8847,335,5411,211,5889,1263,5263,1771,14555,899,1226,33071,3269,8872,2415,3641,229,8388,5726,2278,3985,857,4227,3030,10879,6187,8619,2386,5639,10934,2358,4433,4939,833,91,222,7197,10821,2819,6493,981,3634,468,8470,3441,2747,3058,10208,115,1505,545,2259,3943,9555,517,8943,2302,9664,3109,899,2650,754,30565,13597,62,12733,11205,311,2826,5949,5179,3439,10219,17006,
665,3749,1207,5515,2519,3397,5277,8256,12441,999,3067,13321,11753,2877,4241,2431,12999,9164,21694,8203,2897,7002,9232,2145,4141,2939,953,7317,189,1655,261,906,31970,4550,38284,327,377,151,1282,778,7845,2293,3867,61,3809,1195,3090,2161,1186,4243,197,31115,3114,4051,1389,386,9608,227,8488,2734,4849,851,5612,2497,359,16856,1507,2766,17500,7695,2202,1647,2249,1873,221,2899,587,1063,2619,301,11975,11132,11989,8826,3839,311,571,711,35637,7023,5689,8813,4139,17493,3753,2253,3965,2246,1454,875,10421,806,
279,15554,1295,5525,2510,10653,671,10600,683,9446,11788,9291,4239,4479,1279,1795,269,2266,753,5733,3492,2861,1451,1745,1782,12176,1274,5315,12401,6033,219,11585,2877,417,505,778,2782,5169,11119,1943,1563,2722,703,6651,8363,8057,453,2455,711,4042,11509,2965,826,13133,13170,1489,1249,1091,1543,11279,14055,31323,5399,319,21263,3101,2338,670,531,257,145,2145,2134,8034,14369,13771,4463,3726,4330,1430,1475,10342,2507,9502,9616,1429,3823,193,6717,521,483,5638,16948,846,279,11340,3849,5001,4191,3945,1791,
569,533,3501,1021,923,22341,14006,5587,3001,1198,1961,3498,1739,9407,7291,3231,333,3043,583,2511,8508,2087,154,1962,10975,903,2221,482,229,4097,9254,3725,7799,4493,15180,743,10180,1974,1607,8358,12614,8683,10502,1234,32771,3699,91,631,5627,937,2997,12524,11404,378,10053,38675,11696,5277,1554,5425,4591,9374,6366,523,890,5898,183,6141,6749,979,1662,5603,2829,9253,239,1871,9576,10167,341,12252,5770,14841,1795,2753,7411,739,79,4243,13605,4841,14797,4219,8063,6156,1855,7347,1590,10808,85,213,3202,851,
28427,9457,8885,26285,190,943,2417,1619,8321,4454,59,3014,138,1235,1738,689,4534,1219,131,9912,1673,171,1105,833,6547,257,1155,553,1274,6393,2298,17456,9465,9626,2058,942,5110,2027,26437,37327,1726,5066,2903,8330,9328,1158,3009,14902,893,3243,419,4681,7451,5927,473,6311,1741,1554,2194,690,587,5326,222,8835,1777,531,654,1247,9420,6333,1550,8163,1160,347,717,549,8431,10492,943,637,8228,1577,10335,7851,970,14331,167,963,4433,3313,61,220,373,5934,6713,926,2141,663,11758,25155,13942,6985,1959,3875,3578,
1617,7373,1835,269,107,991,3911,910,1034,1741,293,4819,193,597,11559,6474,2146,16210,1811,573,16510,2591,7722,549,2834,1874,6581,2246,1295,463,15572,18665,354,319,5279,723,1449,7457,499,1883,7489,199,3882,3302,2275,1279,7865,1574,6835,466,2055,3927,602,8980,915,2101,463,2751,223,15083,1949,1405,5065,6831,897,3651,1742,7743,909,2118,18651,5233,11864,3198,2295,7326,769,2613,14455,8315,139,3258,8119,12148,850,6551,7343,8847,1033,21435,9241,835,4693,1422,185,1162,1173,8367,238,4645,494,7147,33,179,2074,
22,395,6907,799,9877,24701,3171,209,4155,2142,1507,4091,1817,753,235,10466,4790,1635,5582,14739,2389,2498,1931,11797,1045,1306,4706,18007,597,2195,3734,5886,1659,939,2783,7330,1254,639,4627,4137,2630,2869,411,12782,3235,4878,20995,3677,9116,12037,7433,25103,107,237,821,3090,2839,11347,978,7573,3673,1832,3670,1022,8994,1217,3134,5570,2157,4717,7391,3334,7055,7402,115,3983,300,221,318,907,8354,1094,761,401,1645,6874,1218,33051,295,5995,843,127,6109,4671,94,30514,2863,6873,11961,1045,1647,12889,4547,
8906,3868,2363,1923,5097,11199,2898,1487,667,6439,4439,878,1689,7181,2773,5267,4459,1893,12715,609,4269,3767,3102,29273,3854,17937,385,2290,1569,4953,6270,535,1437,2330,8559,2282,4691,730,8026,5523,7339,19277,3637,5170,8814,10535,983,2422,31423,73,109,1910,16379,435,1272,6727,3454,3659,1758,7617,12092,1822,850,6670,8741,4630,1171,10286,2955,1274,2053,10141,2891,4122,8098,1539,5167,386,9786,2631,2491,2442,5038,4629,7466,10181,843,11705,1753,435,4973,2774,267,4091,959,742,374,258,2974,2217,3942,1143,
786,267,4086,1810,8826,3207,3074,86,3469,123,8377,6342,15830,8824,5427,4043,399,2713,1053,2655,2771,3991,2949,2461,219,3999,609,1353,1007,7978,757,3013,6958,2546,671,2571,3153,2189,1731,4641,8444,3225,8054,2557,8638,28335,903,3149,3515,1275,659,2388,8912,3842,4695,6349,14610,5663,8966,5233,1047,8669,1407,493,34818,7507,6183,10270,3303,2353,146,2710,2058,681,38380,9298,478,1419,377,10467,1237,927,3223,946,1570,747,47,2231,785,1277,4697,577,667,581,114,281,3089,10880,4123,3091,2135,8292,12459,1337,
2018,8593,1693,2226,1903,379,6231,39748,8336,5330,1854,1309,94,6143,4171,1831,4550,491,4151,2421,2071,8773,2295,2858,2257,2322,1109,3358,32545,651,1553,113,314,13574,163,5031,8518,9357,3157,7593,834,3181,3627,2795,1407,15592,8354,5947,11567,251,771,14684,5066,177,2519,523,1294,5897,9646,1517,511,4309,64850,2413,1625,5489,3214,4582,3033,567,857,8933,706,11197,8314,2697,1061,5459,7599,2765,1133,6249,446,5577,5125,778,582,2267,1305,2657,1903,1591,325,981,2063,4801,3071,3166,79,3077,354,1063,385,8724,
3246,703,1061,4581,58,385,3101,13386,3747,333,5085,10739,1935,5335,3427,8400,22852,1757,77,8334,199,1015,8956,7262,7963,2605,393,222,3905,17976,8934,2019,2313,2607,2273,4215,319,16672,415,2550,5727,3567,14742,462,2733,831,5715,7159,2175,621,7815,25545,4022,863,2135,15157,2519,7243,5151,1205,11409,12115,1637,373,974,139,2927,217,14,488,2811,11180,298,15520,1673,24691,10854,4338,431,1153,8286,10135,831,4467,582,3157,5959,9402,842,673,4306,171,2042,397,1207,3930,5535,8874,502,391,149,9855,21,545,3759,
713,19830,3095,2674,3194,8861,23942,7413,10074,1781,11391,3503,20615,6267,7226,71,305,4914,1477,94,22012,4010,3385,4857,2473,2938,319,1359,1142,10037,5039,12645,766,350,3194,594,2286,10499,3197,5131,13639,35366,249,839,9232,8098,28263,1753,1099,454,4231,197,2759,13553,567,7317,1495,3690,5817,57,4442,8227,1781,9688,1635,3101,15325,271,411,1290,2720,2574,9782,7465,8404,621,1619,791,2767,9556,3971,74,5390,743,623,2857,4268,2521,1326,4151,171,7227,2497,6089,10076,6058,11036,10870,4250,18948,646,11109,
910,2179,11924,20986,19,4637,499,8469,3425,226,1615,4499,921,299,5330,1258,6671,2371,223,8014,4583,5003,918,827,11758,2402,8516,199,57,10613,9382,8940,5691,299,1221,1145,2113,3529,1246,3324,2711,29761,8517,51348,347,438,14313,2225,5861,37,1118,6769,35452,936,622,859,214,931,11008,30545,2286,1763,910,1385,551,1155,2239,5979,1831,19878,2614,303,4711,117,2463,39994,1943,3133,3030,317,633,11043,8682,18630,1988,114,313,339,650,3755,1783,4822,899,6907,3739,9331,8129,12933,2967,2003,6255,3011,1915,8877,
492,4682,7086,974,10743,306,13655,1397,373,1566,4721,13771,1913,707,811,9448,13180,35,629,9023,3309,646,8325,863,327,603,6614,53,393,8439,1055,451,9700,1867,5322,3669,13,8216,2578,226,1126,1973,3110,8212,1963,6997,6339,3879,2449,951,2108,6377,233,590,1629,95,5993,526,730,939,615,767,622,3103,11088,4937,2423,7346,1881,7541,9955,95,1277,6697,1318,787,5466,24557,29726,19263,1927,12989,8760,2417,865,2714,5773,3670,349,11670,11241,471,4071,2893,4227,679,1378,730,2750,471,7835,473,1117,37,10378,613,7027,
10567,6877,10298,24918,16518,2123,1542,3207,1074,1902,19348,327,2203,2397,1061,5537,2455,9511,219,2131,3715,15407,802,8290,9170,5131,127,6439,14725,26917,239,5615,3471,797,74,155,1159,705,2578,16566,8769,3211,106,2443,12704,907,15497,4501,1037,574,5123,8675,109,3293,1265,7374,4150,8429,27555,91,8599,417,533,3821,52443,619,2214,4413,1675,13537,2050,881,2698,567,755,24281,6266,11496,445,430,223,657,5257,2923,139,3151,7818,3159,3338,499,979,6546,1899,463,3005,3091,9921,5750,369,31,347,1279,1374,8694,
379,2673,1319,623,3505,641,3814,3110,2226,795,61,3997,2003,12244,23,9717,1827,1471,3035,8585,5403,4266,155,38,1841,10537,2569,473,1617,727,9935,2718,359,7093,683,386,2379,25013,2563,413,1773,6986,1274,3007,1966,4149,1099,12267,1569,1107,5814,2082,2514,569,15238,17613,3238,2006,9902,1498,17398,29281,2611,4397,141,373,6459,691,1039,4389,1135,5858,1053,3567,9654,3707,2395,13106,1186,769,223,913,8827,7294,3418,5141,1173,290,2788,107,770,2419,4009,327,14100,3695,1766,12925,443,2234,1869,2619,3965,278,
4430,8395,1685,7467,13234,3442,6206,2575,699,2046,1067,5305,359,8308,1506,3503,769,457,2724,257,2215,3073,811,14258,1430,1706,3109,933,1871,5543,1361,22895,3434,149,526,521,25911,1449,3507,3677,3865,11,8946,183,2326,10565,623,3359,85,11290,2014,2359,4037,7949,33916,11639,5253,6091,1355,8695,1101,4510,2513,8242,8807,3625,63,1247,3361,165,5489,8298,3234,12760,15322,9993,4821,3713,691,4993,1309,2354,1365,3459,541,8697,4417,833,549,4110,2651,8514,14666,871,22999,1191,2722,6910,8521,4731,2905,8038,3851,
5678,3818,4795,123,21052,214,9169,607,7489,1157,190,20594,1259,1663,2387,219,215,4341,7011,13393,2845,1203,209,694,1177,903,8757,2253,3477,19033,353,1735,7979,5249,1973,1251,1654,505,483,143,725,325,8369,991,1245,2815,11071,2854,10759,15426,183,3270,21723,419,631,803,759,8207,7734,407,1929,6299,3061,4438,693,5381,11421,2669,42688,564,4622,4226,2138,8511,1857,3961,2153,337,4271,209,1954,8390,671,10024,526,2747,2942,5748,4285,1926,7333,323,4918,57,2381,1061,9884,5006,817,3147,1226,4767,9497,701,1615,
1614,859,369,8330,5163,10834,2747,11715,1955,422,1055,2381,4562,9564,165,1463,2290,1298,2939,11956,1863,8501,3652,199,2854,3081,1807,543,2886,13930,1832,7,1078,21211,1495,1999,6739,1743,32510,627,8105,246,3251,2379,3423,14117,1177,367,11697,102,12632,16279,6293,11814,13366,1543,11556,22,10255,309,3049,2106,9265,933,123,393,15557,159,483,2223,949,16484,10589,5997,1191,4402,674,1592,554,9641,9889,11547,24548,8263,1335,381,15222,115,23229,14923,5067,390,1245,9561,922,1543,6689,717,3395,423,3784,3707,
6821,590,665,3938,2323,3017,26038,2770,201,9190,1729,8629,9065,4716,3365,1293,3795,1557,2089,1848,6602,4743,1953,969,687,8022,4059,2050,5510,8518,10274,939,1071,861,1619,553,6373,1547,1579,1283,11477,11658,1405,20153,2599,374,5878,12764,1026,1353,12673,24794,1067,8722,9613,2516,101,6550,1069,809,20469,1139,2262,1187,1954,4442,502,302,18284,1602,663,1051,927,2327,63,1387,115,4603,342,231,3911,193,678,658,2599,36738,1064,1393,2621,2382,4569,482,2507,1069,1234,934,469,637,15058,1957,7143,15990,6047,
1325,923,3327,2789,9374,259,12701,10977,505,3781,6874,7193,1949,5951,693,9064,986,549,1822,406,1751,6073,7159,4435,869,2655,13,10487,8725,15267,146,119,9016,1485,1509,9869,546,3793,278,2849,1902,12920,238,12712,21663,3363,6237,381,3521,741,6413,1155,4202,2283,1220,2410,2333,277,3942,8309,2714,8556,11658,8902,1901,1081,7177,1094,7611,9370,2330,131,1221,3855,2543,2067,3033,4682,9284,2487,4681,4150,26114,6762,12069,3429,282,895,3586,1013,1346,1279,467,11085,2729,787,1079,3406,11489,489,8693,13470,159,
1573,4100,786,107,13943,1459,4677,8937,2189,10152,5351,481,7171,2641,3111,171,10735,843,5431,274,18017,4497,4287,185,1485,2438,954,9029,2526,5333,2349,502,10171,10018,8221,1766,913,16499,1555,10509,9111,714,17345,266,3795,8439,16355,5854,1115,94,166,2713,4073,2790,14312,34111,8268,2173,3085,1209,7403,2631,1061,1829,2427,1567,119,8651,2851,1126,554,1467,5982,8723,6843,1846,3735,10948,3033,29321,16458,526,7715,1430,2571,2515,8695,1667,1751,3907,8415,530,714,902,2050,346,9440,17954,3517,8375,1027,1131,
2127,535,3305,2643,2506,2811,5239,3475,6985,1035,5714,731,4330,9588,31372,8854,5950,1661,20185,3293,146,18010,8629,9746,13169,6343,1727,65,103,1007,4878,14265,137,9490,5785,58,673,2555,1286,397,1705,36761,104,6149,9852,2383,161,619,10874,12903,56731,13423,4793,5369,10942,13745,707,83,21707,5943,2495,5737,645,4763,1300,2391,2833,4494,1211,3115,6951,635,3230,8832,303,2063,61,1450,2065,3085,426,1345,2473,1285,5797,9872,5530,227,2409,1415,26,8914,7515,730,10211,3483,6754,203,163,174,2518,27165,15711,
12886,4514,13810,1942,1389,471,9254,2753,1281,3226,9702,1607,1910,9337,8301,664,949,12623,1843,3411,3543,3619,1670,43,3373,4027,590,319,1041,8210,5246,15456,803,14536,21487,371,8821,2358,6423,7739,2497,2269,975,11092,1537,2713,3238,366,1785,861,637,1131,2211,1490,11652,2055,689,1065,2407,28010,2071,138,799,183,27759,1051,31,2921,9971,5491,981,8597,2322,670,1256,617,234,1819,12932,3851,8199,8387,63,499,2539,1494,1073,1983,387,485,1821,10421,793,573,2203,1266,545,4149,1189,1786,2417,12255,195,3298,
11968,1547,616,190,7358,28642,398,7561,335,2043,119,33,7061,1604,3053,13082,4291,5981,482,3541,1873,3494,15780,8429,263,403,10729,537,9225,7358,209,10539,3253,385,667,15674,755,1219,8226,4355,667,1081,141,541,8516,8114,3599,4199,787,5629,3229,181,383,6374,4850,583,13864,991,2333,478,4162,1827,757,1175,9534,2023,2106,9615,2117,1259,4435,1705,1987,411,13931,1013,8520,199,498,9790,2314,1561,3083,322,1401,8700,3769,13119,11458,10670,2085,3278,6911,2871,12023,5336,157,9477,2773,1547,9386,8606,2878,5626,
4417,3438,322,3786,1206,2849,2773,11429,3263,10017,5814,69,534,3525,1213,2846,1041,5187,147,583,5030,4637,1933,9142,7099,971,555,7559,1015,889,6659,599,1157,2177,6898,786,199,6375,1551,1979,16801,315,11519,4253,2553,5050,1635,2219,850,4058,11347,279,834,885,8926,262,3497,2438,2287,1409,6369,9909,28468,8915,425,8002,305,8913,1487,1423,2171,10114,6127,1601,3438,40134,15886,5843,2459,10131,7013,3517,21767,277,4349,403,28038,19301,131,1233,33925,4329,3878,1433,1399,2193,31,1359,4854,8387,2913,1263,4748,
31895,30669,4873,927,11268,5999,157,3175,91,1228,1027,127,8512,471,925,1195,8284,9319,3113,5006,214,2995,4037,32185,3587,23182,2573,309,1143,2539,15576,2361,7807,106,3473,8238,5814,3143,2221,998,2225,409,8539,9340,838,8932,957,22832,59,14862,2527,25938,2069,5218,1075,4961,3159,2595,2425,2265,6093,2010,10451,5862,133,1650,1917,620,5831,247,2119,10366,3827,1251,11192,9668,1737,199,1543,3205,13474,9411,1991,442,6378,3319,15449,335,2165,1494,3889,4789,1410,2938,8686,6401,51,7782,2321,1611,2465,10338,
8655,17946,1217,1529,2930,4239,142,1342,13711,1226,11451,1574,6231,1601,8305,4519,7247,886,4317,38430,17965,2937,3,9592,1921,1661,3788,1146,4169,2929,547,2402,1647,1107,2935,6247,1631,355,2373,2141,1169,2455,14720,369,5355,1053,10699,2123,4021,7361,922,39,4459,3505,2243,2267,3195,2334,3129,5379,790,12504,5089,7682,9985,29227,5651,902,9647,32794,2109,19794,233,253,2337,1585,601,1255,2018,4254,4009,1725,857,65,1165,4693,9262,25157,6225,2665,11374,1349,19697,1965,3491,7526,2842,30969,1422,967,34364,
637,3819,3210,4919,21444,33354,167,1450,2542,1530,14684,4005,214,3215,7421,2455,191,1057,8631,1377,575,2981,3027,5942,1485,11218,5998,7644,1318,2803,9428,9625,1267,1915,1702,1201,19335,1172,2861,1011,16163,524,747,5639,6195,1971,223,6469,11985,8510,259,1791,18567,9267,4843,1252,3994,1767,10614,8416,263,23459,10837,4173,1669,2379,4033,11705,6749,67,506,1989,5225,5074,74,2618,35,419,34,467,2305,5589,5777,2599,4957,29,37712,2462,5563,1535,2290,7386,1642,5821,990,6947,749,1087,110,635,16080,3003,4758,
3542,651,663,1997,1981,2855,4413,15183,435,591,285,2099,15978,9906,10479,2595,9196,11464,25723,4218,29515,10660,24195,6843,11805,733,255,838,2815,6717,9791,14339,4709,4702,4967,5075,9512,1346,7126,16150,5277,1327,4135,161,563,11,1259,2435,275,1686,186,3023,8819,10050,1863,4578,218,12108,4869,1031,7467,6337,861,2834,4775,5371,782,11457,1543,12694,705,9143,1871,1704,251,217,117,809,293,9077,3211,6837,2982,6199,4126,907,8033,15097,1894,5826,12785,241,3715,1583,2857,16459,657,1653,9582,1795,695,10461,
1314,4062,511,12379,3022,795,365,5719,1039,4497,4926,7013,5401,14153,2289,667,3268,9070,8906,5425,71,1820,8474,1102,2994,4849,7430,629,16702,1559,5387,790,6742,8634,1255,5279,1926,13868,1347,11504,2354,1219,1041,3447,1821,894,1238,697,3706,1135,9157,1038,5001,950,927,1122,2461,959,287,9209,11219,790,7709,266,183,10857,7306,34,185,303,6413,3153,285,2059,2707,10863,673,3187,6791,8766,9605,7506,1879,16066,371,15962,885,8528,359,8965,2153,178,695,7233,29898,11600,5910,20985,7341,9215,10928,3297,1341,
19247,3037,11758,610,1313,969,743,660,8441,12867,788,9012,175,883,577,522,3902,3357,7222,6279,18376,11043,453,266,3290,1335,1651,381,5449,2301,24103,2163,5767,2177,2847,1798,7978,182,1267,6582,1241,14002,3839,381,1465,53,6285,2867,4011,2375,175,13959,2995,9991,707,10411,1546,3614,851,4836,807,2417,154,1870,322,9944,1102,247,7363,1006,24838,974,10099,10034,634,12851,9298,2101,690,2065,8935,5377,4922,15273,2149,2595,16671,1854,1963,1193,4290,1599,11067,28,8114,1569,341,263,2698,10930,2149,3339,1911,
143,8337,29431,29,3945,1675,434,555,3413,8437,4997,233,833,539,8998,9057,995,163,4237,11454,663,1795,1722,3495,16491,8726,7727,1147,2503,2573,2930,5627,4105,7385,4634,402,8517,1578,10040,3003,725,111,8519,477,271,1235,4111,9612,1480,546,699,3842,1927,3321,201,9133,1033,1286,9715,13596,11580,2717,11271,533,8167,5306,8220,1338,4115,3517,6905,28417,2175,3757,3207,5791,5826,1023,797,2323,7595,2071,761,7636,7447,4799,1057,470,2329,2006,201,635,538,1679,371,5579,1140,1097,1299,6481,11547,12056,14638,25754,
1529,1146,11408,715,428,983,3053,2886,2527,6702,542,11825,7373,3133,638,1337,977,1053,9042,22742,1873,8707,309,1937,4137,253,2991,919,1203,1955,4754,6829,9052,351,801,2405,17945,3081,5098,8528,342,5995,17229,10371,1117,15692,5642,3405,6477,4893,7035,873,8316,3314,1077,1037,1081,1591,9941,4978,1191,635,11027,3955,2959,2466,16160,433,861,5982,1477,142,1407,14880,241,455,1410,3749,3005,366,847,10527,777,7715,9691,3541,46,2007,1793,1115,222,8421,4325,2227,133,569,4701,3349,1122,9930,8196,191,2314,2461,
655,2049,319,9320,634,1199,22,2049,8747,10096,1489,10390,3142,1593,1341,10033,1230,1791,1738,1522,10984,3194,14522,2927,1733,1093,13680,1469,907,990,4822,403,1743,620,9855,1953,5030,1327,8635,3065,2989,5157,2797,8637,4377,759,1174,517,5307,1626,7422,4195,14469,2086,6205,20548,5369,478,8792,63,2387,11896,689,1743,4897,15664,593,5794,70,1046,3277,998,6585,6402,7550,3377,13446,6297,3813,3063,4607,42559,701,1914,9970,3631,287,573,1597,1491,3873,1794,1271,5611,2150,153,5691,5039,325,1142,2253,62,2050,
1142,18879,95,4515,523,4967,3725,9098,18274,2290,2871,633,14582,269,7923,8149,3053,1982,2039,15796,2983,12818,498,2834,2665,2158,426,1938,6567,466,82,18667,909,15857,2035,291,241,654,31194,167,349,819,4809,12879,9479,386,1750,3283,141,2427,2873,1449,9942,154,987,4365,3967,11048,659,2827,6573,2121,22450,8014,5939,1107,1491,1069,1147,22,3334,5273,13168,5217,78,2889,8742,166,677,249,246,1240,3954,7174,12082,4307,12917,823,821,3686,593,28979,5650,9321,12380,997,950,3043,1813,4593,7292,1737,309,3399,
3955,11160,8434,6302,2918,31003,5181,5146,459,1633,1787,2339,3047,14577,36985,1983,4757,1699,24406,3819,2654,433,1015,12878,1302,10336,1741,3373,7170,5785,1053,13374,11913,533,941,599,5086,9586,2747,4458,1666,11487,5645,40577,7163,9925,279,2009,407,2693,9102,5273,10107,1422,115,12049,119,11801,341,9129,8514,1614,1043,2349,366,903,3702,3609,2705,2328,1414,2293,2727,2113,3127,19308,2329,1023,10175,925,8544,4115,2194,1359,9252,138,817,7897,918,323,8530,2829,1751,5581,25658,1517,445,2397,163,241,14777,
171,2782,3930,39974,1277,318,12735,12044,4934,2323,6697,698,6036,4694,5269,587,8445,2218,4630,1954,3981,8067,8187,4522,1041,10402,37117,1182,1543,11430,9242,834,3537,11522,33957,4995,491,2279,4122,637,8722,3317,6163,1349,455,843,995,681,42,1783,959,3095,1469,28598,9902,5909,1163,8572,1722,566,9001,2725,3226,7830,2051,1354,1151,8228,1735,19522,6446,178,1223,5197,51,1074,893,381,5286,8925,27691,681,1239,9390,1159,7178,598,10187,10721,12067,891,9191,783,2377,2889,137,5050,2581,394,5383,1110,6173,9401,
9059,1275,10477,2108,2211,510,8195,4039,13323,4398,4829,1828,5146,5553,9105,209,9466,201,941,8550,3713,2761,12454,3582,3270,15090,3446,8658,7411,2694,419,9631,8598,5405,911,2447,2630,5381,2755,4395,3385,8224,3201,10432,2437,4091,8499,2335,777,20517,643,2641,5426,2302,1266,386,1115,2494,165,2802,4617,695,1625,922,701,1757,8954,510,2072,4207,1966,7815,914,26081,2099,917,4625,6695,7186,6210,117,5026,1271,5254,1715,4462,5370,8965,1593,1116,2115,673,1307,62,3722,4870,1913,253,8632,550,876,205,943,20717,
1486,1779,3334,3878,1266,499,7989,15711,7187,4325,8247,658,5398,2698,3269,1706,5141,30431,1649,1094,13996,3847,4390,3694,15908,17573,20205,2870,14429,2837,383,2124,2607,5042,5186,1502,2006,8628,2963,2653,442,1579,1353,3183,893,3977,8346,11853,714,417,466,762,967,495,275,10096,3197,3982,1911,2821,21737,2510,9330,1671,6513,8563,6061,2022,2153,4302,8330,1195,5967,10342,543,766,519,2371,511,417,1446,14731,389,2687,431,319,117,561,638,2563,4293,1303,1289,5850,467,2709,1828,154,24097,419,178,2094,587,
653,9920,6697,3799,3747,323,1437,2619,1630,1379,924,3523,8811,1134,1299,991,1659,11179,3105,34590,8896,362,2595,1254,1393,379,6677,571,155,379,4726,501,1186,1025,447,10505,975,754,1386,2117,20220,5143,7843,2795,3011,713,389,1101,767,1116,447,1307,2059,4058,3598,610,2921,2353,166,4433,199,1477,4025,9850,301,5006,14630,693,10889,12815,151,2526,4595,1159,4087,1795,7707,30981,10557,10197,5281,6233,7310,1823,2131,4621,2283,3510,1310,1449,4606,2231,326,2062,9019,10527,3577,6812,4229,323,669,2413,3601,
4041,2151,8904,10171,5683,1349,2763,7486,1189,771,1139,3803,12249,431,13170,1091,37694,3891,9607,2221,5779,7870,11678,998,969,2133,8933,14255,11624,2465,15753,2838,4187,3367,1286,5679,2186,715,2143,998,342,751,12177,6259,1302,571,8110,7923,12334,8286,1113,1222,8769,371,876,7991,2706,4717,2705,541,394,1883,7507,643,835,357,929,8694,8122,1973,2503,357,130,937,8681,975,91,123,13290,4583,2078,3270,2682,8936,8249,1775,8519,1439,6295,4151,10817,8942,3263,18933,1293,183,4601,9155,5481,481,39192,358,16721,
419,2142,4135,16945,10998,6490,1175,8490,846,6409,12493,23326,2899,415,36491,13051,866,161,22,2150,849,4670,3849,489,833,549,5675,14716,8764,1685,161,4785,30101,1067,635,126,1387,5495,985,1942,166,453,2101,8602,1389,6802,193,4307,1302,6233,3138,1199,3111,10278,2852,8693,13152,10313,10246,66,554,1941,8070,5726,9988,1997,693,2405,549,350,11750,5111,6237,5873,4531,1289,1709,2238,831,202,2893,1726,47,23812,1553,1053,2927,635,5906,4088,1084,7304,7714,1363,3417,2427,3119,7934,4549,2271,1330,1483,650,223,
649,7917,3966,7318,411,8992,2455,1135,4953,919,2130,610,1844,137,403,3757,9114,1366,646,8467,7277,7171,2442,93,857,895,2134,38617,11100,12622,67,3883,5807,743,5005,731,10084,69,46255,865,6671,1071,3798,1855,1426,3361,571,2697,161,286,497,2207,2038,2015,12745,10440,313,1443,8753,11702,267,7226,27666,398,1309,170,10128,1398,1839,8276,3414,55,3107,3154,2405,1814,8903,4845,1417,1667,2427,549,818,475,1499,1169,767,537,1697,2386,2075,3673,1871,9379,1938,3229,1237,222,209,5473,5589,4397,9112,5233,12487,
1835,149,7860,5289,10055,4185,53,1394,2265,2610,6079,8316,13263,3475,9011,22,159,6061,82,2361,349,25061,10631,2375,10013,9529,10695,17510,231,9334,1798,1263,4233,1565,23669,930,1958,4258,1674,6817,33844,2653,5899,749,1627,639,4551,2575,11826,2063,8731,1695,514,339,233,3975,1795,7759,1851,1463,3431,1310,4105,4382,3243,9196,4133,14732,1146,4154,3782,1494,1370,2986,5165,12898,2091,143,11820,22253,874,3347,14570,13432,29171,581,12929,770,9338,3913,2067,466,1442,1065,10328,3273,197,4811,8897,5417,1769,
206,119,3406,6754,309,3909,646,3677,3514,8797,1435,301,9215,3730,7282,165,3381,303,6369,2627,115,761,3695,1233,1297,1803,1130,122,511,58,6426,1389,1433,9102,943,2795,13162,9905,899,9740,8719,517,5115,4977,1486,8259,7163,13549,7561,305,119,973,7698,5619,5326,5772,32611,8846,1709,4641,782,2503,3905,4051,4923,165,16132,16827,1913,6590,322,741,522,7666,1655,5087,468,8541,1169,4455,22993,2801,6623,446,1979,10923,3503,32053,523,5721,9608,955,11766,49919,1982,66,3347,9508,439,145,5697,1026,1569
        };
        constexpr static itype::u64 bases2 = 15ull | (135ull << 8) | (13ull << 16) | (60ull << 24) | (15ull << 32) | (117ull << 40) | (65ull << 48) | (29ull << 56);
        template<class Modint = internal::MontgomeryModint64Impl> constexpr static bool calc(const itype::u64 x) {
            if (x % 2 == 0 || x % 3 == 0 || x % 5 == 0 || x % 7 == 0 || x % 11 == 0 || x % 13 == 0 || x % 17 == 0 || x % 19 == 0) return false;
            Modint mint;
            mint.set(x);
            const itype::u32 S = std::countr_zero(x - 1);
            const itype::u64 D = (x - 1) >> S;
            const auto one = mint.one(), mone = mint.neg(one);
            auto test2 = [&](itype::u32 base1, itype::u32 base2) GSH_INTERNAL_INLINE {
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
                    a = mint.mul(a, a), b = mint.mul(b, b), c = mint.mul(c, c);
                    res1 |= mint.same(a, mone), res2 |= mint.same(b, mone);
                }
                return res1 && res2;
            };
            auto test3 = [&](itype::u32 base1, itype::u32 base2, itype::u32 base3) GSH_INTERNAL_INLINE {
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

}  


template<bool Prob = true> constexpr bool isPrime(const itype::u64 x) {
    if (x <= 0xffffffff) {
        if (x < 65536u) return internal::isPrime16<0>::calc(x);
        else if (x < 2147483648u) return internal::isPrime32<0>::calc(x);
        else return internal::isPrime32<0>::calc<internal::MontgomeryModint64Impl>(x);
    } else return internal::isPrime64<Prob, 0>::calc(x);
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



namespace internal {
    template<itype::u8 A, itype::u8 B, itype::u8 C, itype::u8 D> __attribute__((always_inline)) constexpr void TrialDiv(itype::u32& n, itype::u64*& res) {
        const bool a = n % A == 0, b = n % B == 0, c = n % C == 0, d = n % D == 0;
        if (a) [[unlikely]] {
            do {
                n /= A;
                *(res++) = A;
            } while (n % A == 0);
        }
        if (b) [[unlikely]] {
            do {
                n /= B;
                *(res++) = B;
            } while (n % B == 0);
        }
        if (c) [[unlikely]] {
            do {
                n /= C;
                *(res++) = C;
            } while (n % C == 0);
        }
        if (d) [[unlikely]] {
            do {
                n /= D;
                *(res++) = D;
            } while (n % D == 0);
        }
    }
    constexpr void FactorizeSub32(itype::u32 n, itype::u64*& res) {
        if (n < 256 || isPrime(n)) {
            *(res++) = n;
            return;
        }
        const itype::u32 copy = n;
        
#define GSH_INTERNAL_TDIV(a, b, c, d) TrialDiv<a, b, c, d>(n, res); if(d != 251 && n < (d + 2) * (d + 2)) { *(res++) = n; return; }
        GSH_INTERNAL_TDIV(17,19,23,29)GSH_INTERNAL_TDIV(31,37,41,43)GSH_INTERNAL_TDIV(47,53,59,61)GSH_INTERNAL_TDIV(67,71,73,79)
        GSH_INTERNAL_TDIV(83,89,97,101)GSH_INTERNAL_TDIV(103,107,109,113)GSH_INTERNAL_TDIV(127,131,137,139)GSH_INTERNAL_TDIV(149,151,157,163)
        GSH_INTERNAL_TDIV(167,173,179,181)GSH_INTERNAL_TDIV(191,193,197,199)GSH_INTERNAL_TDIV(211,223,227,229)GSH_INTERNAL_TDIV(233,239,241,251)
#undef GSH_INTERNAL_TDIV
          
          if (n < 65536 || (copy != n && isPrime(n))) {
            *(res++) = n;
            return;
        }
        for (itype::u32 x = 240; x != 65520; x += 30) {
            const itype::u32 a = x + 17, b = x + 19, c = x + 23, d = x + 29, e = x + 31, f = x + 37, g = x + 41, h = x + 43;
            const bool i = n % a == 0, j = n % b == 0, k = n % c == 0, l = n % d == 0, m = n % e == 0, o = n % f == 0, p = n % g == 0, q = n % h == 0;
            auto div = [&](itype::u32 val, bool flag) __attribute__((always_inline)) {
                if (flag) [[unlikely]] {
                    do {
                        n /= val;
                        *(res++) = val;
                    } while (n % val == 0);
                }
            };
            div(a, i), div(b, j), div(c, k), div(d, l), div(e, m), div(f, o), div(g, p), div(h, q);
            if (n <= h * h) [[unlikely]]
                break;
        }
        if (n != 1) *(res++) = n;
    }
    constexpr itype::u64 FindFactor64(itype::u64 n) {
        return n;
    }
    constexpr void FactorizeSub64(itype::u64 n, itype::u64*& res) {
        if (n <= 0xffffffff) {
            FactorizeSub32(n, res);
            return;
        }
        if (isPrime(n)) {
            *(res++) = n;
            return;
        }
        const itype::u64 m = FindFactor64(n);
        FactorizeSub64(n / m, res);
        FactorizeSub64(m, res);
    }
}  
constexpr Arr<itype::u64> Factorize(itype::u64 n) {
    if (n <= 1) [[unlikely]]
        return {};
    itype::u64 res[64];
    itype::u64* p = res;
    {
        itype::u32 rz = std::countr_zero(n);
        n >>= rz;
        for (itype::u32 i = 0; i != rz; ++i) *(p++) = 2;
    }
    {
        const bool a = n % 3 == 0, b = n % 5 == 0, c = n % 7 == 0, d = n % 11 == 0, e = n % 13 == 0;
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
    }
    if (n >= 256) [[likely]]
        internal::FactorizeSub64(n, p);
    else *(p++) = n;
    return {};
}

}  

#include <ctime>   
#include <limits>  
#include <cstdio>  

namespace gsh {

class ClockTimer {
    std::clock_t start_time;
public:
    ClockTimer() { start_time = std::clock(); }
    void restart() { start_time = std::clock(); }
    std::clock_t elapsed() const { return (std::clock() - start_time) * 1000 / CLOCKS_PER_SEC; }
    void print() const { std::printf("gsh::ClockTimer::print / %llu ms\n", static_cast<unsigned long long>(elapsed())); }
};

}  


#include <cassert>

#if false
#include <fcntl.h>
gsh::BasicReader r(open("in.txt", O_RDONLY));
gsh::BasicWriter w(open("out.txt", O_WRONLY | O_TRUNC));
#else
gsh::BasicReader r;
gsh::BasicWriter w;
#endif
void Main() {
    using namespace std;
    using namespace gsh;
    using namespace gsh::itype;
    using namespace gsh::ftype;
    using namespace gsh::ctype;
    internal::DynamicModint32Impl mint;
    mint.set(998244353);
    
    
    auto a = mint.build(2u), b = mint.build(3u), c = mint.build(5u);
    ClockTimer t;
    [&]() __attribute__((noinline)) {
        for (u32 i = 0; i != 100000000; ++i) {
            auto d = mint.neg(mint.inc(mint.sub(mint.dec(mint.add(mint.mul(a, a), b)), c)));
            auto e = mint.neg(mint.inc(mint.sub(mint.dec(mint.add(mint.mul(b, b), c)), a)));
            auto f = mint.neg(mint.inc(mint.sub(mint.dec(mint.add(mint.mul(c, c), a)), b)));
            a = d, b = e, c = f;
        }
        
    }
    ();
    t.print();
    Formatter<u64>{}(w, mint.val(a));
    Formatter<c8>{}(w, '\n');
    Formatter<u64>{}(w, mint.val(b));
    Formatter<c8>{}(w, '\n');
    Formatter<u64>{}(w, mint.val(c));
    Formatter<c8>{}(w, '\n');
}
int main() {
#ifdef ONLINE_JUDGE
    Main();
    w.reload();
#else
    try {
        Main();
        w.reload();
    } catch (gsh::Exception& e) {
        gsh::Formatter<const gsh::ctype::c8*>{}(w, "gsh::Exception was throwed: ");
        gsh::Formatter<const gsh::ctype::c8*>{}(w, e.what());
        gsh::Formatter<gsh::ctype::c8>{}(w, '\n');
        w.reload();
    }
#endif
}

