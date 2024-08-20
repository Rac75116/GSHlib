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
}  // namespace itype

namespace ftype {

    using f32 = float;
    using f64 = double;
#ifdef __SIZEOF_FLOAT128__
    using f128 = __float128;
#endif
    using flong = long double;
}  // namespace ftype

namespace ctype {
    using c8 = char;
    using wc = wchar_t;
    using utf8 = char8_t;
    using utf16 = char16_t;
    using utf32 = char32_t;
}  // namespace ctype

namespace internal {
    template<class T, class U> constexpr bool IsSame = false;
    template<class T> constexpr bool IsSame<T, T> = true;
    template<class T, class U, class... V> constexpr bool IsSameAny = IsSame<T, U> || IsSameAny<T, V...>;
    template<class T, class U> constexpr bool IsSameAny<T, U> = IsSame<T, U>;
}  // namespace internal

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

}  // namespace simd

}  // namespace gsh


#define GSH_INTERNAL_SELECT1(a, ...)                      a
#define GSH_INTERNAL_SELECT2(a, b, ...)                   b
#define GSH_INTERNAL_SELECT3(a, b, c, ...)                c
#define GSH_INTERNAL_SELECT4(a, b, c, d, ...)             d
#define GSH_INTERNAL_SELECT5(a, b, c, d, e, ...)          e
#define GSH_INTERNAL_SELECT6(a, b, c, d, e, f, ...)       f
#define GSH_INTERNAL_SELECT7(a, b, c, d, e, f, g, ...)    g
#define GSH_INTERNAL_SELECT8(a, b, c, d, e, f, g, h, ...) h

#define GSH_INTERNAL_STR(s)       #s
#define GSH_INTERNAL_CONCAT(a, b) a##b
#define GSH_INTERNAL_VA_SIZE(...) GSH_INTERNAL_SELECT8(__VA_ARGS__, 7, 6, 5, 4, 3, 2, 1, 0)
#if defined __clang__ || defined __INTEL_COMPILER
#define GSH_INTERNAL_UNROLL(n) _Pragma(GSH_INTERNAL_STR(unroll n))
#elif defined __GNUC__
#define GSH_INTERNAL_UNROLL(n) _Pragma(GSH_INTERNAL_STR(GCC unroll n))
#else
#define GSH_INTERNAL_UNROLL(n)
#endif
#ifdef __GNUC__
#define GSH_INTERNAL_INLINE   [[gnu::always_inline]]
#define GSH_INTERNAL_NOINLINE [[gnu::noinline]]
#elif defined _MSC_VER
#define GSH_INTERNAL_INLINE   [[msvc::forceinline]]
#define GSH_INTERNAL_NOINLINE [[msvc::noinline]]
#else
#define GSH_INTERNAL_INLINE
#define GSH_INTERNAL_NOINLINE
#endif
#ifdef __clang__
#define GSH_INTERNAL_PUSH_ATTRIBUTE(apply, ...) _Pragma(GSH_INTERNAL_STR(clang attribute push(__attribute__((__VA_ARGS__)), apply_to = apply)))
#define GSH_INTERNAL_POP_ATTRIBUTE              _Pragma("clang attribute pop")
#elif defined __GNUC__
#define GSH_INTERNAL_PUSH_ATTRIBUTE(apply, ...) _Pragma("GCC push_options") _Pragma(GSH_INTERNAL_STR(GCC __VA_ARGS__))
#define GSH_INTERNAL_POP_ATTRIBUTE              _Pragma("GCC pop_options")
#else
#define GSH_INTERNAL_PUSH_ATTRIBUTE(apply, ...)
#define GSH_INTERNAL_POP_ATTRIBUTE
#endif


namespace gsh {

namespace itype {
    struct i4dig {};
    struct u4dig {};
    struct i8dig {};
    struct u8dig {};
    struct i16dig {};
    struct u16dig {};
}  // namespace itype

template<class T> class Parser;

namespace internal {
    template<class Stream> constexpr itype::u8 Parseu8(Stream& stream) {
        itype::u32 v;
        std::memcpy(&v, stream.current(), 4);
        v ^= 0x30303030;
        itype::i32 tmp = std::countr_zero(v & 0xf0f0f0f0) >> 3;
        v <<= (32 - (tmp << 3));
        v = (v * 10 + (v >> 8)) & 0x00ff00ff;
        v = (v * 100 + (v >> 16)) & 0x0000ffff;
        stream.skip(tmp + 1);
        return v;
    }
    template<class Stream> constexpr itype::u16 Parseu16(Stream& stream) {
        itype::u64 v;
        std::memcpy(&v, stream.current(), 8);
        v ^= 0x3030303030303030;
        itype::i32 tmp = std::countr_zero(v & 0xf0f0f0f0f0f0f0f0) >> 3;
        v <<= (64 - (tmp << 3));
        v = (v * 10 + (v >> 8)) & 0x00ff00ff00ff00ff;
        v = (v * 100 + (v >> 16)) & 0x0000ffff0000ffff;
        v = (v * 10000 + (v >> 32)) & 0x00000000ffffffff;
        stream.skip(tmp + 1);
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
        GSH_INTERNAL_UNROLL(4)
        for (itype::u32 i = 0; i != 4; ++i) {
            itype::u64 v;
            std::memcpy(&v, stream.current(), 8);
            if (((v ^= 0x3030303030303030) & 0xf0f0f0f0f0f0f0f0) != 0) break;
            v = (v * 10 + (v >> 8)) & 0x00ff00ff00ff00ff;
            v = (v * 100 + (v >> 16)) & 0x0000ffff0000ffff;
            v = (v * 10000 + (v >> 32)) & 0x00000000ffffffff;
            if (i == 0) res = v;
            else res = res * 100000000 + v;
            stream.skip(8);
        }
        itype::u64 buf;
        std::memcpy(&buf, stream.current(), 8);
        itype::u64 res2 = 0, pw = 1;
        {
            itype::u32 v = buf;
            if (!((v ^= 0x30303030) & 0xf0f0f0f0)) {
                buf >>= 32;
                v = (v * 10 + (v >> 8)) & 0x00ff00ff;
                v = (v * 100 + (v >> 16)) & 0x0000ffff;
                res2 = v;
                pw = 10000;
                stream.skip(4);
            }
        }
        {
            itype::u16 v = buf;
            if (!((v ^= 0x3030) & 0xf0f0)) {
                buf >>= 16;
                v = (v * 10 + (v >> 8)) & 0x00ff;
                res2 = res2 * 100 + v;
                pw *= 100;
                stream.skip(2);
            }
        }
        {
            const ctype::c8 v = ctype::c8(buf) ^ 0x30;
            const bool f = (v & 0xf0) == 0;
            const volatile auto tmp1 = pw * 10, tmp2 = res2 * 10 + v;
            const auto tmp3 = tmp1, tmp4 = tmp2;
            pw = f ? tmp3 : pw;
            res2 = f ? tmp4 : res2;
            stream.skip(f + 1);
        }
        return res * pw + res2;
    }
    template<class Stream> constexpr itype::u16 Parseu4dig(Stream& stream) {
        itype::u32 v;
        std::memcpy(&v, stream.current(), 4);
        v ^= 0x30303030;
        itype::i32 tmp = std::countr_zero(v & 0xf0f0f0f0) >> 3;
        v <<= (32 - (tmp << 3));
        v = (v * 10 + (v >> 8)) & 0x00ff00ff;
        v = (v * 100 + (v >> 16)) & 0x0000ffff;
        stream.skip(tmp + 1);
        return v;
    }
    template<class Stream> constexpr itype::u32 Parseu8dig(Stream& stream) {
        itype::u64 v;
        std::memcpy(&v, stream.current(), 8);
        v ^= 0x3030303030303030;
        const itype::u64 msk = v & 0xf0f0f0f0f0f0f0f0;
        itype::i32 tmp = std::countr_zero(msk) >> 3;
        v <<= (64 - (tmp << 3));
        v = (v * 10 + (v >> 8)) & 0x00ff00ff00ff00ff;
        v = (v * 100 + (v >> 16)) & 0x0000ffff0000ffff;
        v = (v * 10000 + (v >> 32)) & 0x00000000ffffffff;
        stream.skip(tmp + 1);
        return v;
    }
}  // namespace internal

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
        stream.skip(neg);
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
        stream.skip(neg);
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
        stream.skip(neg);
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
        stream.skip(neg);
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
        stream.skip(neg);
        itype::i128 tmp = internal::Parseu128(stream);
        if (neg) tmp = -tmp;
        return tmp;
    }
};
template<> class Parser<itype::u4dig> {
public:
    template<class Stream> constexpr itype::u16 operator()(Stream& stream) const {
        stream.reload(8);
        return internal::Parseu4dig(stream);
    }
};
template<> class Parser<itype::i4dig> {
public:
    template<class Stream> constexpr itype::i16 operator()(Stream& stream) const {
        stream.reload(8);
        bool neg = *stream.current() == '-';
        stream.skip(neg);
        itype::i16 tmp = internal::Parseu4dig(stream);
        if (neg) tmp = -tmp;
        return tmp;
    }
};
template<> class Parser<itype::u8dig> {
public:
    template<class Stream> constexpr itype::u32 operator()(Stream& stream) const {
        stream.reload(16);
        return internal::Parseu8dig(stream);
    }
};
template<> class Parser<itype::i8dig> {
public:
    template<class Stream> constexpr itype::i32 operator()(Stream& stream) const {
        stream.reload(16);
        bool neg = *stream.current() == '-';
        stream.skip(neg);
        itype::i32 tmp = internal::Parseu8dig(stream);
        if (neg) tmp = -tmp;
        return tmp;
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
template<> class Parser<ctype::c8*> {
    ctype::c8* s;
    itype::u32 n;
public:
    constexpr Parser(ctype::c8* s_, itype::u32 n_ = 0xffffffff) noexcept : s(s_), n(n_) {}
    template<class Stream> constexpr void operator()(Stream& stream) const {
        if (n == 0xffffffff) {
            stream.reload(16);
            ctype::c8* c = s;
            while (true) {
                const ctype::c8* e = stream.current();
                while (*e >= '!') ++e;
                const itype::u32 len = e - stream.current();
                std::memcpy(c, stream.current(), len);
                stream.skip(len);
                c += len;
                if (stream.avail() == 0) stream.reload();
                else break;
            }
            stream.skip(1);
            *c = '\0';
        } else {
            itype::u32 rem = n;
            ctype::c8* c = s;
            itype::u32 avail = stream.avail();
            while (avail <= rem) {
                std::memcpy(c, stream.current(), avail);
                c += avail;
                rem -= avail;
                stream.skip(avail);
                stream.reload();
                avail = stream.avail();
            }
            std::memcpy(c, stream.current(), rem);
            c += rem;
            stream.skip(rem + 1);
            *c = '\0';
        }
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
        auto copy1 = [&](itype::u16 x) {
            itype::u32 off = (x < 10) + (x < 100) + (x < 1000);
            std::memcpy(stream.current(), InttoStr<0>.table + (4 * x + off), 4);
            stream.skip(4 - off);
        };
        auto copy2 = [&](itype::u16 x) {
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
        auto div_1e16 = [&](itype::u64& rem) -> itype::u64 {
            if (std::is_constant_evaluated()) {
                rem = n % 10000000000000000;
                return n / 10000000000000000;
            }
            itype::u64 res;
            __asm__("divq %[v]" : "=a"(res), "=d"(rem) : [v] "r"(10000000000000000), "a"(static_cast<itype::u64>(n)), "d"(static_cast<itype::u64>(n >> 64)));
            return res;
        };
        constexpr itype::u128 t = static_cast<itype::u128>(10000000000000000) * 10000000000000000;
        if (n >= t) {
            const itype::u32 dv = n / t;
            n -= dv * t;
            if (dv >= 10000) {
                copy1(dv / 10000);
                copy2(dv % 10000);
            } else copy1(dv);
            itype::u64 a, b = 0;
            a = div_1e16(b);
            const itype::u32 c = a / 100000000, d = a % 100000000, e = b / 100000000, f = b % 100000000;
            copy2(c / 10000), copy2(c % 10000);
            copy2(d / 10000), copy2(d % 10000);
            copy2(e / 10000), copy2(e % 10000);
            copy2(f / 10000), copy2(f % 10000);
        } else {
            itype::u64 a, b = 0;
            a = div_1e16(b);
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
    template<class Stream> constexpr void Formatu8dig(Stream& stream, itype::u32 x) {
        const itype::u32 n = x;
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
    template<class Stream> constexpr void Formatu16dig(Stream& stream, itype::u64 x) {
        const itype::u64 n = x;
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
}  // namespace internal

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
    template<class Stream> constexpr void operator()(Stream& stream, itype::u32 n) const {
        stream.reload(8);
        internal::Formatu8dig(stream, n);
    }
};
template<> class Formatter<itype::i8dig> {
public:
    template<class Stream> constexpr void operator()(Stream& stream, itype::i32 n) const {
        stream.reload(9);
        *stream.current() = '-';
        stream.skip(n < 0);
        internal::Formatu8dig(stream, static_cast<itype::u32>(n < 0 ? -n : n));
    }
};
template<> class Formatter<itype::u16dig> {
public:
    template<class Stream> constexpr void operator()(Stream& stream, itype::u64 n) const {
        stream.reload(16);
        internal::Formatu16dig(stream, n);
    }
};
template<> class Formatter<itype::i16dig> {
public:
    template<class Stream> constexpr void operator()(Stream& stream, itype::i64 n) const {
        stream.reload(17);
        *stream.current() = '-';
        stream.skip(n < 0);
        internal::Formatu16dig(stream, static_cast<itype::u64>(n < 0 ? -n : n));
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

template<itype::u32 Bufsize = (1 << 18)> class BasicReader {
    itype::i32 fd = 0;
    ctype::c8 buf[Bufsize + 1] = {};
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

template<itype::u32 Bufsize = (1 << 18)> class BasicWriter {
    itype::i32 fd = 1;
    ctype::c8 buf[Bufsize + 1] = {};
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

}  // namespace gsh

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

}  // namespace gsh

#include <bit>
#include <cstring>
#include <type_traits>

#include <type_traits>


namespace gsh {

[[noreturn]] void Unreachable() {
#if defined __GNUC__ || defined __clang__
    __builtin_unreachable();
#elif _MSC_VER
    __assume(false);
#else
    [[maybe_unused]] itype::u32 n = 1 / 0;
#endif
};
GSH_INTERNAL_INLINE constexpr void Assume(const bool f) {
    if (std::is_constant_evaluated()) return;
#if defined __clang__
    __builtin_assume(f);
#elif defined __GNUC__
    if (!f) __builtin_unreachable();
#elif _MSC_VER
    __assume(f);
#else
    if (!f) Unreachable();
#endif
}
template<bool Likely = true> GSH_INTERNAL_INLINE constexpr bool Expect(const bool f) {
    if (std::is_constant_evaluated()) return f;
#if defined __GNUC__ || defined __clang__
    return __builtin_expect(f, Likely);
#else
    if constexpr (Likely) {
        if (f) [[likely]]
            return true;
        else return false;
    } else {
        if (f) [[unlikely]]
            return false;
        else return true;
    }
#endif
}
GSH_INTERNAL_INLINE constexpr bool Unpredictable(const bool f) {
    if (std::is_constant_evaluated()) return f;
#if defined __clang__
    return __builtin_unpredictable(f);
#elif defined __GNUC__
    return __builtin_expect_with_probability(f, 1, 0.5);
#else
    return f;
#endif
}
template<class T>
    requires std::is_scalar_v<T>
GSH_INTERNAL_INLINE constexpr void ForceCalc([[maybe_unused]] const T& v) {
#if defined __GNUC__ || defined __INTEL_COMPILER
    if (!std::is_constant_evaluated()) __asm__ volatile("" ::"r"(v));
#endif
}
template<class T, class... Args> GSH_INTERNAL_INLINE constexpr void ForceCalc(const T& v, const Args&... args) {
    ForceCalc(v);
    ForceCalc(args...);
}

class InPlaceTag {};
constexpr InPlaceTag InPlace;


}  // namespace gsh

#include <immintrin.h>


namespace gsh {

template<itype::u32 Size>
    requires(Size <= (1u << 24))
class BitTree24 {
    constexpr static itype::u32 s1 = ((Size + 262143) / 262144 + 7) / 8 * 8, s2 = ((Size + 4095) / 4096 + 7) / 8 * 8, s3 = ((Size + 63) / 64 + 7) / 8 * 8;
    alignas(32) itype::u64 v0, v1[s1], v2[s2], v3[s3];
    constexpr void build() noexcept {
        if (std::is_constant_evaluated()) {
            v0 = 0;
            for (itype::u32 i = 0; i != s1; ++i) v1[i] = 0;
            for (itype::u32 i = 0; i != s2; ++i) v2[i] = 0;
            for (itype::u32 i = 0; i != s3; ++i) v2[i / 64] |= (static_cast<itype::u64>(v3[i] != 0) << (i % 64));
            for (itype::u32 i = 0; i != s2; ++i) v1[i / 64] |= (static_cast<itype::u64>(v2[i] != 0) << (i % 64));
            for (itype::u32 i = 0; i != s1; ++i) v0 |= (static_cast<itype::u64>(v1[i] != 0) << (i % 64));
            return;
        }
        for (itype::u32 x = 0; x < s3; x += 64) {
            auto get = [&](itype::u32 n) -> itype::u32 {
                return _mm256_movemask_pd(_mm256_castsi256_pd(_mm256_cmpeq_epi64(_mm256_load_si256(reinterpret_cast<const __m256i*>(&v3[x + n])), _mm256_setzero_si256())));
            };
            const itype::u64 a = get(0), b = get(4), c = get(8), d = get(12), e = get(16), f = get(20), g = get(24), h = get(28);
            const itype::u64 i = get(32), j = get(36), k = get(40), l = get(44), m = get(48), n = get(52), o = get(56), p = get(60);
            v2[x / 64] = ~(a | b << 4 | c << 8 | d << 12 | e << 16 | f << 20 | g << 24 | h << 28 | i << 32 | j << 36 | k << 40 | l << 44 | m << 48 | n << 52 | o << 56 | p << 60);
        }
        for (itype::u32 i = s3 / 64; i != s2; ++i) v2[i] = 0;
        for (itype::u32 i = s3 / 64 * 64; i != s3; ++i) v2[i / 64] |= (static_cast<itype::u64>(v3[i] != 0) << (i % 64));
        for (itype::u32 x = 0; x < s2; x += 64) {
            auto get = [&](itype::u32 n) -> itype::u32 {
                return _mm256_movemask_pd(_mm256_castsi256_pd(_mm256_cmpeq_epi64(_mm256_load_si256(reinterpret_cast<const __m256i*>(&v2[x + n])), _mm256_setzero_si256())));
            };
            const itype::u64 a = get(0), b = get(4), c = get(8), d = get(12), e = get(16), f = get(20), g = get(24), h = get(28);
            const itype::u64 i = get(32), j = get(36), k = get(40), l = get(44), m = get(48), n = get(52), o = get(56), p = get(60);
            v1[x / 64] = ~(a | b << 4 | c << 8 | d << 12 | e << 16 | f << 20 | g << 24 | h << 28 | i << 32 | j << 36 | k << 40 | l << 44 | m << 48 | n << 52 | o << 56 | p << 60);
        }
        for (itype::u32 i = s2 / 64; i != s1; ++i) v1[i] = 0;
        for (itype::u32 i = s2 / 64 * 64; i != s2; ++i) v1[i / 64] |= (static_cast<itype::u64>(v2[i] != 0) << (i % 64));
        v0 = 0;
        for (itype::u32 i = 0; i != s1; ++i) v0 |= (static_cast<itype::u64>(v1[i] != 0) << (i % 64));
    }
public:
    constexpr BitTree24() noexcept : v0{}, v1{}, v2{}, v3{} {}
    constexpr BitTree24(itype::u64 val) noexcept : v0{}, v1{}, v2{}, v3{} {
        if (val != 0) {
            v0 = v1[0] = v2[0] = 1ull;
            v3[0] = val;
        }
    }
    constexpr BitTree24(const ctype::c8* p) { assign(p); }
    constexpr BitTree24(const ctype::c8* p, itype::u32 sz) { assign(p, sz); }
    constexpr ~BitTree24() noexcept = default;
    constexpr BitTree24& operator=(const BitTree24&) noexcept = default;
    constexpr void assign(const ctype::c8* p) { assign(p, std::strlen(p)); }
    constexpr void assign(const ctype::c8* p, itype::u32 sz) {
        sz = sz < Size ? sz : Size;
        if (std::is_constant_evaluated()) {
            for (itype::u32 i = 0; i != s3; ++i) v3[i] = 0;
            for (itype::u32 i = 0; i < sz; ++i) v3[i / 64] |= static_cast<itype::u64>(p[i] - '0') << (i % 64);
            build();
            return;
        }
        for (itype::u32 i = 0; i < sz; i += 64) {
            auto get = [&](itype::u32 n) -> itype::u32 {
                return _mm256_movemask_epi8(_mm256_cmpeq_epi8(_mm256_loadu_si256(reinterpret_cast<const __m256i_u*>(p + i + n)), _mm256_set1_epi8('1')));
            };
            const itype::u64 a = get(0), b = get(8), c = get(16), d = get(24), e = get(32), f = get(40), g = get(48), h = get(56);
            v3[i / 64] = a | b << 8 | c << 16 | d << 24 | e << 32 | f << 40 | g << 48 | h << 56;
        }
        for (itype::u32 i = sz / 64; i != s3; ++i) v3[i] = 0;
        for (itype::u32 i = sz / 64 * 64; i < sz; i += 4) {
            const itype::u32 n = _mm256_movemask_epi8(_mm256_cmpeq_epi8(_mm256_loadu_si256(reinterpret_cast<const __m256i_u*>(p + i)), _mm256_set1_epi8('1')));
            v3[i / 64] |= static_cast<itype::u64>(n) << (i % 64);
        }
        const itype::u32 b = sz / 4 * 4;
        switch (sz % 4) {
        case 3 : v3[(b + 2) / 64] |= static_cast<itype::u64>(p[b + 2] - '0') << ((b + 2) % 64); [[fallthrough]];
        case 2 : v3[(b + 1) / 64] |= static_cast<itype::u64>(p[b + 1] - '0') << ((b + 1) % 64); [[fallthrough]];
        case 1 : v3[(b + 0) / 64] |= static_cast<itype::u64>(p[b + 0] - '0') << ((b + 0) % 64); [[fallthrough]];
        default : break;
        }
        build();
    }
    constexpr BitTree24& operator&=(const BitTree24& rhs) noexcept {
        for (itype::u32 i = 0; i != s3; ++i) v3[i] &= rhs.v3[i];
        build();
        return *this;
    }
    constexpr BitTree24& operator|=(const BitTree24& rhs) noexcept {
        v0 |= rhs.v0;
        for (itype::u32 i = 0; i != s1; ++i) v1[i] |= rhs.v1[i];
        for (itype::u32 i = 0; i != s2; ++i) v2[i] |= rhs.v2[i];
        for (itype::u32 i = 0; i != s3; ++i) v3[i] |= rhs.v3[i];
        return *this;
    }
    constexpr BitTree24& operator^=(const BitTree24& rhs) noexcept {
        for (itype::u32 i = 0; i != s3; ++i) v3[i] ^= rhs.v3[i];
        build();
        return *this;
    }
    constexpr BitTree24& operator<<=(itype::u32 pos) noexcept;
    constexpr BitTree24& operator>>=(itype::u32 pos) noexcept;
    constexpr BitTree24& set() noexcept {
        for (itype::u32 i = 0; i != (Size + 4095) / 262144; ++i) v1[i] = 0xffffffffffffffff;
        for (itype::u32 i = 0; i != (Size + 63) / 4096; ++i) v2[i] = 0xffffffffffffffff;
        for (itype::u32 i = 0; i != Size / 64; ++i) v3[i] = 0xffffffffffffffff;
        if constexpr (Size + 262143 >= (1ull << 24)) v0 = 0xffffffffffffffff;
        else v0 = (1ull << ((Size + 262143) / 262144)) - 1;
        if constexpr (constexpr itype::u32 x = (Size + 4095) / 4096; x % 64 != 0) v1[x / 64] = (1ull << (x % 64)) - 1;
        if constexpr (constexpr itype::u32 x = (Size + 63) / 64; x % 64 != 0) v2[x / 64] = (1ull << (x % 64)) - 1;
        if constexpr (Size % 64 != 0) v3[Size / 64] = (1ull << (Size % 64)) - 1;
        return *this;
    }
    constexpr BitTree24& set(itype::u32 pos) {
        v0 |= (1ull << (pos / 262144));
        v1[pos / 262144] |= (1ull << (pos / 4096) % 64);
        v2[pos / 4096] |= (1ull << ((pos / 64) % 64));
        v3[pos / 64] |= (1ull << (pos % 64));
        return *this;
    }
    constexpr BitTree24& set(itype::u32 pos, bool val) {
        if (val) return set(pos);
        else return reset(pos);
    }
    constexpr BitTree24& reset() noexcept {
        std::memset(this, 0, sizeof(BitTree24));
        return *this;
    }
    constexpr BitTree24& reset(itype::u32 pos) {
        const itype::u64 m1 = (1ull << ((pos / 4096) % 64)), m2 = (1ull << ((pos / 64) % 64)), m3 = (1ull << (pos % 64));
        const bool f1 = v1[pos / 262144] == m1, f2 = v2[pos / 4096] == m2, f3 = v3[pos / 64] == m3;
        v3[pos / 64] &= ~m3;
        v2[pos / 4096] &= (f3 ? ~m2 : 0xffffffffffffffff);
        v1[pos / 262144] &= (f2 && f3 ? ~m1 : 0xffffffffffffffff);
        v0 &= (f1 && f2 && f3 ? ~(1ull << (pos / 262144)) : 0xffffffffffffffff);
        return *this;
    }
    constexpr BitTree24& flip() noexcept {
        for (itype::u32 i = 0; i != s3; ++i) v3[i] = ~v3[i];
        build();
    }
    constexpr BitTree24& flip(itype::u32 pos) noexcept { return set(pos, !test(pos)); }
    class reference {
        friend class BitTree24;
        BitTree24& ref;
        itype::u32 idx;
        constexpr reference(BitTree24& ref_, itype::u32 idx_) noexcept : ref(ref_), idx(idx_) {}
    public:
        constexpr ~reference() noexcept {}
        constexpr reference& operator=(bool x) noexcept {
            ref.set(idx, x);
            return *this;
        }
        constexpr reference& operator=(const reference& x) {
            ref.set(idx, x);
            return *this;
        }
        constexpr bool operator~() const noexcept { return ~ref.test(idx); }
        constexpr operator bool() const noexcept { return ref.test(idx); }
        constexpr reference& flip() const noexcept {
            ref.flip(idx);
            return *this;
        }
    };
    constexpr bool operator[](itype::u32 pos) const { return test(pos); }
    constexpr reference operator[](itype::u32 pos) { return reference(*this, pos); }
    constexpr itype::u32 count() const noexcept {
        itype::u32 res = 0;
        for (itype::u32 i = 0; i != s3; ++i) res += std::popcount(v3[i]);
        return res;
    }
    constexpr itype::u32 size() const noexcept { return Size; }
    constexpr bool test(itype::u32 pos) const { return v3[pos / 64] >> (pos % 64) & 1; }
    constexpr bool all() const noexcept;
    constexpr bool any() const noexcept { return v0 != 0; }
    constexpr bool none() const noexcept { return v0 == 0; }
    constexpr itype::u64 to_u64() const {
        if (v0 > 1 || v1[0] > 1 || v2[0] > 1) throw Exception("gsh::BitTree::to_u64 / Result overflowed.");
        return v3[0];
    }
    constexpr unsigned long to_ulong() const { return to_u64(); }
    constexpr unsigned long long to_ullong() const { return to_u64(); }
    constexpr bool operator==(const BitTree24& rhs) const noexcept {
        if (v0 != rhs.v0) return false;
        if (std::is_constant_evaluated()) {
            for (itype::u32 i = 0; i != s1; ++i)
                if (v1[i] != rhs.v1[i]) return false;
            for (itype::u32 i = 0; i != s2; ++i)
                if (v2[i] != rhs.v2[i]) return false;
            for (itype::u32 i = 0; i != s3; ++i)
                if (v3[i] != rhs.v3[i]) return false;
            return true;
        }
        for (itype::u32 i = 0; i != s1; i += 4) {
            const itype::u32 t = _mm256_movemask_epi8(_mm256_cmpeq_epi8(_mm256_load_si256((const __m256i*) &v1[i]), _mm256_load_si256((const __m256i*) &rhs.v1[i])));
            if (t != 0xffffffff) return false;
        }
        for (itype::u32 i = 0; i != s2; i += 4) {
            const itype::u32 t = _mm256_movemask_epi8(_mm256_cmpeq_epi8(_mm256_load_si256((const __m256i*) &v2[i]), _mm256_load_si256((const __m256i*) &rhs.v2[i])));
            if (t != 0xffffffff) return false;
        }
        for (itype::u32 i = 0; i != s3; i += 4) {
            const itype::u32 t = _mm256_movemask_epi8(_mm256_cmpeq_epi8(_mm256_load_si256((const __m256i*) &v3[i]), _mm256_load_si256((const __m256i*) &rhs.v3[i])));
            if (t != 0xffffffff) return false;
        }
        return true;
    }
    constexpr bool operator!=(const BitTree24& rhs) const noexcept { return !operator==(rhs); }
    constexpr BitTree24 operator<<(itype::u32 pos) const noexcept { return BitTree24(*this) <<= pos; }
    constexpr BitTree24 operator>>(itype::u32 pos) const noexcept { return BitTree24(*this) >>= pos; }
    friend constexpr BitTree24 operator&(const BitTree24& lhs, const BitTree24& rhs) noexcept { return BitTree24(lhs) &= rhs; }
    friend constexpr BitTree24 operator|(const BitTree24& lhs, const BitTree24& rhs) noexcept { return BitTree24(lhs) |= rhs; }
    friend constexpr BitTree24 operator^(const BitTree24& lhs, const BitTree24& rhs) noexcept { return BitTree24(lhs) ^= rhs; }
    constexpr static itype::u32 npos = -1;
    constexpr itype::u32 find_next(itype::u32 pos) const {
        if (const itype::u64 tmp = v3[pos / 64] & -(1ull << (pos % 64)); tmp != 0) return pos / 64 * 64 + std::countr_zero(tmp);
        if (const itype::u64 tmp = v2[pos / 4096] & -(2ull << (pos / 64 % 64)); tmp != 0) {
            const itype::u64 a = pos / 4096 * 64 + std::countr_zero(tmp), b = v3[a];
            Assume(b != 0);
            return a * 64 + std::countr_zero(b);
        }
        if (const itype::u64 tmp = v1[pos / 262144] & -(2ull << (pos / 4096 % 64)); tmp != 0) {
            const itype::u64 a = pos / 262144 * 64 + std::countr_zero(tmp), b = v2[a];
            Assume(b != 0);
            const itype::u64 c = a * 64 + std::countr_zero(b), d = v3[c];
            Assume(d != 0);
            return c * 64 + std::countr_zero(d);
        }
        if (const itype::u64 tmp = v0 & -(2ull << (pos / 262144 % 64)); tmp != 0) {
            const itype::u64 a = std::countr_zero(tmp), b = v1[a];
            Assume(b != 0);
            const itype::u64 c = a * 64 + std::countr_zero(b), d = v2[c];
            Assume(d != 0);
            const itype::u64 e = c * 64 + std::countr_zero(d), f = v3[e];
            Assume(f != 0);
            return e * 64 + std::countr_zero(f);
        }
        return npos;
    }
    constexpr itype::u32 find_first() const noexcept { return find_next(0); }
    constexpr itype::u32 find_prev(itype::u32 pos) const {
        if (const itype::u64 tmp = v3[pos / 64] & ((2ull << (pos % 64)) - 1); tmp != 0) return pos / 64 * 64 + std::bit_width(tmp) - 1;
        if (const itype::u64 tmp = v2[pos / 4096] & ((1ull << (pos / 64 % 64)) - 1); tmp != 0) {
            const itype::u64 a = pos / 4096 * 64 + std::bit_width(tmp) - 1, b = v3[a];
            Assume(b != 0);
            return a * 64 + std::bit_width(b) - 1;
        }
        if (const itype::u64 tmp = v1[pos / 262144] & ((1ull << (pos / 4096 % 64)) - 1); tmp != 0) {
            const itype::u64 a = pos / 262144 * 64 + std::bit_width(tmp) - 1, b = v2[a];
            Assume(b != 0);
            const itype::u64 c = a * 64 + std::bit_width(b) - 1, d = v3[c];
            Assume(d != 0);
            return c * 64 + std::bit_width(d) - 1;
        }
        if (const itype::u64 tmp = v0 & ((1ull << (pos / 262144 % 64)) - 1); tmp != 0) {
            const itype::u64 a = std::bit_width(tmp) - 1, b = v1[a];
            Assume(b != 0);
            const itype::u64 c = a * 64 + std::bit_width(b) - 1, d = v2[c];
            Assume(d != 0);
            const itype::u64 e = c * 64 + std::bit_width(d) - 1, f = v3[e];
            Assume(f != 0);
            return e * 64 + std::bit_width(f) - 1;
        }
        return npos;
    }
    constexpr itype::u32 find_last() const noexcept { return find_prev(Size - 1); }
};

}  // namespace gsh

#if 0 && !defined ONLINE_JUDGE
#include <fcntl.h>
gsh::BasicReader r(open("in.txt", O_RDONLY));
gsh::BasicWriter w(open("out.txt", O_WRONLY | O_TRUNC));
#else
gsh::MmapReader r;
gsh::BasicWriter w;
#endif
void Main() {
    using namespace std;
    using namespace gsh;
    using namespace gsh::itype;
    using namespace gsh::ftype;
    using namespace gsh::ctype;
    u32 Q = Parser<u32>{}(r);
    static BitTree24<(1 << 20)> A;
    static u64 B[1 << 20];
    A.set();
    while (Q--) {
        c8 t = Parser<c8>{}(r);
        u64 x = Parser<u64>{}(r);
        if (t == '1') {
            u32 y = A.find_next(x % (1 << 20));
            if (y == A.npos) y = A.find_first();
            A.reset(y);
            B[y] = x;
        } else {
            if (!A.test(x % (1 << 20))) {
                Formatter<u64>{}(w, B[x % (1 << 20)]);
                Formatter<c8>{}(w, '\n');
            } else {
                Formatter<c8>{}(w, '-');
                Formatter<c8>{}(w, '1');
                Formatter<c8>{}(w, '\n');
            }
        }
    }
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
