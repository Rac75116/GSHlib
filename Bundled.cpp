
#pragma GCC optimize("Ofast")
#pragma GCC optimize("unroll-loops")

#include <bit>       
#include <cstdlib>   
#include <cstring>   
#include <unistd.h>  
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
}  

namespace ftype {
    using f32 = float;
    using f64 = double;
    using flong = long double;
}  

namespace ctype {
    using c8 = char;
    using utf8 = char8_t;
    using utf16 = char16_t;
    using utf32 = char32_t;
}  

namespace simd {

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

}  

template<class T, class U> constexpr T SimdCast(U x) {
    return __builtin_convertvector(x, T);
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

namespace gsh {

template<class T> class Parser;

namespace internal {
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
        stream.reload(9);
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
template<> class Parser<itype::u8dig> {
public:
    template<class Stream> constexpr itype::u8dig operator()(Stream& stream) const {
        stream.reload(9);
        return internal::Parseu8dig(stream);
    }
};
template<> class Parser<itype::i8dig> {
public:
    template<class Stream> constexpr itype::i8dig operator()(Stream& stream) const {
        stream.reload(9);
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
    template<int> constexpr auto InttoStr = [] {
        struct {
            ctype::c8 table[40004];
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
        auto copy1 = [&](itype::u64 x) {
            itype::u32 off = (x < 10) + (x < 100) + (x < 1000);
            std::memcpy(stream.current(), InttoStr<0>.table + (4 * x + off), 4);
            stream.skip(4 - off);
        };
        auto copy2 = [&](itype::u64 x) {
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
        internal::Formatu8dig(stream, itype::u8dig{ n.val < 0 ? -n.val : n.val });
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
        internal::Formatu16dig(stream, itype::u16dig{ n.val < 0 ? -n.val : n.val });
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
        if (eof == buf + Bufsize) [[likely]] {
            itype::u32 rem = eof - cur;
            std::memmove(buf, cur, rem);
            *(eof = buf + rem + read(fd, buf + rem, Bufsize - rem)) = '\0';
            cur = buf;
        } else if (eof == cur) {
            *(eof = buf + read(fd, buf, Bufsize)) = '\0';
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
    itype::i32 fd = 0;
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
    ~BasicWriter() { reload(); }
    BasicWriter& operator=(const BasicWriter& rhs) {
        fd = rhs.fd;
        std::memcpy(buf, rhs.buf, rhs.cur - rhs.buf);
        cur = buf + (rhs.cur - rhs.buf);
        return *this;
    }
    void reload() {
        [[maybe_unused]] itype::i32 tmp = write(1, buf, cur - buf);
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

#include <bit>              
#include <ctime>            
  

namespace gsh {

namespace internal {
    constexpr itype::u64 Splitmix(itype::u64 x) {
        itype::u64 z = (x + 0x9e3779b97f4a7c15);
        z = (z ^ (z >> 30)) * 0xbf58476d1ce4e5b9;
        z = (z ^ (z >> 27)) * 0x94d049bb133111eb;
        return z ^ (z >> 31);
    }
}  


class Rand64 {
    itype::u64 s0, s1;
public:
    using result_type = itype::u64;
    static constexpr itype::usize word_size = sizeof(result_type) * 8;
    static constexpr result_type default_seed = 0xcafef00dd15ea5e5;
    constexpr Rand64() : Rand64(default_seed) {}
    constexpr explicit Rand64(result_type value) : s0(value), s1(internal::Splitmix(value)) {}
    constexpr result_type operator()() {
        itype::u64 t0 = s0, t1 = s1;
        const itype::u64 res = t0 + t1;
        t1 ^= t0;
        s0 = std::rotr(t0, 9) ^ t1 ^ (t1 << 14);
        s1 = std::rotr(t1, 28);
        return res;
    };
    constexpr void discard(itype::u64 z) {
        for (itype::u64 i = 0; i < z; ++i) operator()();
    }
    static constexpr result_type max() { return 18446744073709551615u; }
    static constexpr result_type min() { return 0; }
    constexpr void seed(result_type value = default_seed) { s0 = value, s1 = internal::Splitmix(value); }
    friend constexpr bool operator==(Rand64 x, Rand64 y) { return x.s0 == y.s0 && x.s1 == y.s1; }
};


class Rand32 {
    itype::u64 val;
public:
    using result_type = itype::u32;
    static constexpr itype::usize word_size = sizeof(result_type) * 8;
    static constexpr result_type default_seed = 0xcafef00d;
    constexpr Rand32() : Rand32(default_seed) {}
    constexpr explicit Rand32(result_type value) : val(internal::Splitmix((itype::u64) value << 32 | value)) {}
    constexpr result_type operator()() {
        itype::u64 x = val;
        const itype::i32 count = x >> 61;
        val = x * 0xcafef00dd15ea5e5;
        x ^= x >> 22;
        return x >> (22 + count);
    };
    constexpr void discard(itype::u64 z) {
        itype::u64 pow = 0xcafef00dd15ea5e5;
        while (z != 0) {
            if (z & 1) val *= pow;
            z >>= 1;
            pow *= pow;
        }
    }
    static constexpr result_type max() { return 4294967295u; }
    static constexpr result_type min() { return 0; }
    constexpr void seed(result_type value = default_seed) { val = internal::Splitmix((itype::u64) value << 32 | value); }
    friend constexpr bool operator==(Rand32 x, Rand32 y) { return x.val == y.val; }
};


class RandomDevice {
    Rand64 engine{ static_cast<itype::u64>(std::time(nullptr)) };
public:
    using result_type = itype::u32;
    RandomDevice() {}
    RandomDevice(const RandomDevice&) = delete;
    ~RandomDevice() = default;
    void operator=(const RandomDevice&) = delete;
    ftype::f64 entropy() const noexcept { return 0.0; }
    static constexpr result_type max() { return 4294967295u; }
    static constexpr result_type min() { return 0; }
    result_type operator()() {
        itype::u64 a = internal::Splitmix(static_cast<itype::u64>(std::time(nullptr)));
        itype::u64 b = internal::Splitmix(static_cast<itype::u64>(std::clock()));
        return static_cast<result_type>(engine() ^ a ^ b);
    }
};


template<class URBG> constexpr itype::u32 Uniform32(URBG& g, itype::u32 max) {
    return (static_cast<itype::u64>(g() & 4294967295u) * max) >> 32;
}

template<class URBG> constexpr itype::u32 Uniform32(URBG& g, itype::u32 min, itype::u32 max) {
    return static_cast<itype::u32>((static_cast<itype::u64>(g() & 4294967295u) * (max - min)) >> 32) + min;
}

template<class URBG> constexpr itype::u64 Uniform64(URBG& g, itype::u64 max) {
    return (static_cast<itype::u128>(g()) * max) >> 64;
}

template<class URBG> constexpr itype::u64 Uniform64(URBG& g, itype::u64 min, itype::u64 max) {
    return static_cast<itype::u64>((static_cast<itype::u128>(g()) * (max - min)) >> 64) + min;
}

template<class URBG> constexpr itype::u32 UnbiasedUniform32(URBG& g, itype::u32 max) {
    itype::u32 mask = ~0u;
    --max;
    mask >>= std::countl_zero(max | 1);
    itype::u32 x;
    do {
        x = g() & mask;
    } while (x > max);
    return x;
}
template<class URBG> constexpr itype::u32 UnbiasedUniform32(URBG& g, itype::u32 min, itype::u32 max) {
    return min + UnbiasedUniform32(g, max - min);
}
template<class URBG> constexpr itype::u64 UnbiasedUniform64(URBG& g, itype::u64 max) {
    itype::u64 mask = ~0ull;
    --max;
    mask >>= std::countl_zero(max | 1);
    itype::u64 x;
    do {
        x = g() & mask;
    } while (x > max);
    return x;
}
template<class URBG> constexpr itype::u32 UnbiasedUniform64(URBG& g, itype::u64 min, itype::u64 max) {
    return min + UnbiasedUniform64(g, max - min);
}



template<class URBG> constexpr ftype::f32 Canocicaled32(URBG& g) {
    return std::bit_cast<ftype::f32>((127u << 23) | (static_cast<itype::u32>(g()) & 0x7fffff)) - 1.0f;
}
template<class URBG> constexpr ftype::f32 Uniformf32(URBG& g, ftype::f32 max) {
    return canocicaled32(g) * max;
}
template<class URBG> constexpr ftype::f32 Uniformf32(URBG& g, ftype::f32 min, ftype::f32 max) {
    return canocicaled32(g) * (max - min) + min;
}
template<class URBG> constexpr ftype::f64 Canocicaled64(URBG& g) {
    return std::bit_cast<ftype::f64>((1023ull << 52) | (g() & 0xfffffffffffffull)) - 1.0;
}
template<class URBG> constexpr ftype::f64 Uniformf64(URBG& g, ftype::f64 max) {
    return canocicaled64(g) * max;
}
template<class URBG> constexpr ftype::f64 Uniformf64(URBG& g, ftype::f64 min, ftype::f64 max) {
    return canocicaled64(g) * (max - min) + min;
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

#include <type_traits>
#include <cmath>
#include <type_traits>        
#include <limits>             
#include <bit>                
    

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
#include <utility>            
#include <initializer_list>   
#include <compare>            
#include <tuple>              
#include <type_traits>  
#include <utility>      
#include <iterator>     
#include <compare>      

namespace gsh {

class InPlaceTag {};
constexpr InPlaceTag InPlace;

namespace internal {

    template<class T> struct DiffType {
        using type = void;
    };
    template<class T>
        requires requires(T x) { x - x; }
    struct DiffType<T> {
        T x;
        using type = decltype(x - x);
    };
    template<class T> class StepIterator {
        T data;
    public:
        using value_type = T;
        using difference_type = DiffType<T>;
        using pointer = const T*;
        using reference = const T&;
        using iterator_category = std::random_access_iterator_tag;
        using iterator_concept = std::random_access_iterator_tag;
        constexpr StepIterator() : data() {}
        constexpr StepIterator(const value_type& x) : data(x) {}
        constexpr StepIterator(value_type&& x) : data(std::move(x)) {}
        template<class U> constexpr StepIterator(const StepIterator<U>& x) : data(static_cast<value_type>(*x)) {}
        template<class U> constexpr StepIterator(StepIterator<U>&& x) : data(static_cast<value_type>(std::move(*x))) {}
        constexpr StepIterator& operator=(const StepIterator& u) = default;
        template<class U> constexpr StepIterator& operator=(const StepIterator<U>& u) {
            data = static_cast<value_type>(u.data);
            return *this;
        }
        template<class U> constexpr StepIterator& operator=(StepIterator<U>&& u) {
            data = static_cast<value_type>(std::move(u.data));
            return *this;
        }
        constexpr reference operator*() { return data; }
        constexpr reference operator*() const { return data; }
        constexpr pointer operator->() const { return &data; }
        constexpr StepIterator& operator++() {
            ++data;
            return *this;
        }
        constexpr StepIterator operator++(int) {
            StepIterator copy = *this;
            ++data;
            return copy;
        }
        constexpr StepIterator& operator--() {
            --data;
            return *this;
        }
        constexpr StepIterator operator--(int) {
            StepIterator copy = *this;
            --data;
            return copy;
        }
        constexpr StepIterator& operator+=(const difference_type& n) {
            data += n;
            return *this;
        }
        constexpr StepIterator& operator-=(const difference_type& n) {
            data -= n;
            return *this;
        }
        constexpr StepIterator operator+(const difference_type& n) const { return StepIterator(*this) += n; }
        constexpr StepIterator operator-(const difference_type& n) const { return StepIterator(*this) -= n; }
        constexpr reference operator[](const difference_type& n) const { return **this + n; }
        friend constexpr StepIterator operator+(const difference_type& n, const StepIterator& x) { return StepIterator(x) += n; }
        friend constexpr difference_type operator-(const StepIterator& x, const StepIterator& y) { return x.data - y.data; }
    };
    template<class T, class U> constexpr bool operator==(const StepIterator<T>& x, const StepIterator<U>& y) {
        return *x == *y;
    }
    template<class T, std::three_way_comparable_with<T> U> constexpr std::compare_three_way_result_t<T, U> operator<=>(const StepIterator<T>& x, const StepIterator<U>& y) {
        return *x <=> *y;
    }

    template<class T> class StepSentinel {
        T data;
    public:
        using value_type = T;
        using difference_type = DiffType<T>;
        using pointer = const T*;
        using reference = const T&;
        constexpr StepSentinel() : data() {}
        constexpr StepSentinel(const value_type& x) : data(x) {}
        constexpr StepSentinel(value_type&& x) : data(std::move(x)) {}
        template<class U> constexpr StepSentinel(const StepSentinel<U>& x) : data(static_cast<value_type>(*x)) {}
        template<class U> constexpr StepSentinel(StepSentinel<U>&& x) : data(static_cast<value_type>(std::move(*x))) {}
        constexpr StepSentinel& operator=(const StepSentinel& u) = default;
        template<class U> constexpr StepSentinel& operator=(const StepSentinel<U>& u) {
            data = static_cast<value_type>(u.data);
            return *this;
        }
        template<class U> constexpr StepSentinel& operator=(StepSentinel<U>&& u) {
            data = static_cast<value_type>(std::move(u.data));
            return *this;
        }
        constexpr reference operator*() { return data; }
        constexpr reference operator*() const { return data; }
        constexpr pointer operator->() const { return &data; }
    };
    template<class T, class U> constexpr bool operator==(const StepSentinel<T>& x, const StepSentinel<U>& y) {
        return *x == *y;
    }
    template<class T, class U> constexpr bool operator==(const StepIterator<T>& x, const StepSentinel<U>& y) {
        if constexpr (requires(T x, U y) { x < y; }) return !(*x < *y);
        else return *x == *y;
    }
    template<class T, class U> constexpr bool operator==(const StepSentinel<T>& x, const StepIterator<U>& y) {
        return operator==(y, x);
    }

    template<class T> class StepResult1 {
        T ed;
    public:
        using iterator = StepIterator<T>;
        using const_iterator = iterator;
        using sentinel = StepSentinel<T>;
        template<class U> constexpr StepResult1(U&& e) : ed(std::forward<U>(e)) {}
        constexpr iterator begin() const { return iterator(); }
        constexpr const_iterator cbegin() const { return const_iterator(); }
        constexpr sentinel end() const { return sentinel(ed); }
        constexpr sentinel cend() const { return sentinel(ed); }
    };
    template<class T> class StepResult2 {
        T bg, ed;
    public:
        using iterator = StepIterator<T>;
        using const_iterator = iterator;
        using sentinel = StepSentinel<T>;
        template<class U, class V> constexpr StepResult2(U&& b, V&& e) : bg(std::forward<U>(b)), ed(std::forward<V>(e)) {}
        constexpr iterator begin() const { return iterator(bg); }
        constexpr const_iterator cbegin() const { return const_iterator(ed); }
        constexpr sentinel end() const { return sentinel(ed); }
        constexpr sentinel cend() const { return sentinel(ed); }
    };

}  

template<class T> constexpr auto Step(T&& ed) {
    return internal::StepResult1<std::decay_t<T>>(std::forward<T>(ed));
}
template<class T, class U> constexpr auto Step(T&& bg, U&& ed) {
    using loop_var_type = std::common_type_t<T, U>;
    return internal::StepResult2<loop_var_type>(std::forward<loop_var_type>(bg), std::forward<loop_var_type>(ed));
}

}  
  

namespace gsh {

class NullOpt {};
constexpr NullOpt Null;
template<class T> class Option {
    template<class U> friend class Option;
public:
    using value_type = T;
private:
    alignas(value_type) Byte buffer[sizeof(value_type)];
    bool has = false;
    template<class... Args> constexpr void construct(Args&&... args) { new (buffer) value_type(std::forward<Args>(args)...); }
    constexpr void destroy() {
        if constexpr (!std::is_trivially_destructible_v<value_type>) delete reinterpret_cast<value_type*>(buffer);
    }
    constexpr value_type& ref() { return *reinterpret_cast<value_type*>(buffer); }
    constexpr const value_type& ref() const { return *reinterpret_cast<const value_type*>(buffer); }
    template<class U> constexpr static bool is_explicit = !std::is_convertible_v<U, T>;
    constexpr static bool noexcept_swapable = []() {
        using std::swap;
        return noexcept(swap(std::declval<value_type&>(), std::declval<value_type&>()));
    }();
public:
    constexpr Option() noexcept {}
    constexpr ~Option() noexcept {
        if (has) destroy();
    }
    constexpr Option(NullOpt) noexcept {}
    constexpr Option(const Option& rhs) {
        if (rhs.has) {
            has = true;
            construct(rhs.ref());
        } else {
            has = false;
        }
    }
    constexpr Option(Option&& rhs) noexcept {
        if (rhs.has) {
            has = true;
            construct(std::move(rhs.ref()));
        } else {
            has = false;
        }
    }
    template<class... Args> constexpr explicit Option(InPlaceTag, Args&&... args) {
        has = true;
        construct(std::forward<Args>(args)...);
    }
    template<class U, class... Args> constexpr explicit Option(InPlaceTag, std::initializer_list<U> il, Args&&... args) {
        has = true;
        construct(il, std::forward<Args>(args)...);
    }
    template<class U = T>
        requires requires(U&& rhs) { static_cast<value_type>(std::forward<U>(rhs)); }
    explicit(is_explicit<U>) constexpr Option(U&& rhs) {
        has = true;
        construct(std::forward<U>(rhs));
    }
    template<class U> explicit(is_explicit<U>) constexpr Option(const Option<U>& rhs) {
        if (rhs.has) {
            has = true;
            construct(static_cast<value_type>(rhs.ref()));
        } else {
            has = false;
        }
    }
    template<class U> explicit(is_explicit<U>) constexpr Option(Option<U>&& rhs) {
        if (rhs.has) {
            has = true;
            construct(static_cast<value_type>(std::move(rhs.ref())));
        } else {
            has = false;
        }
    }
    constexpr Option& operator=(NullOpt) noexcept {
        if (has) {
            destroy();
            has = false;
        }
    }
    constexpr Option& operator=(const Option& rhs) {
        if (has) destroy();
        if (rhs.has) {
            has = true;
            construct(rhs.ref());
        } else {
            has = false;
        }
    }
    template<class... Args> constexpr T& emplace(Args&&... args) {
        if (has) destroy();
        has = true;
        construct(std::forward<Args>(args)...);
        return ref();
    }
    template<class U, class... Args> constexpr T& emplace(std::initializer_list<U> il, Args&&... args) {
        if (has) destroy();
        has = true;
        construct(il, std::forward<Args>(args)...);
        return ref();
    }
    constexpr void swap(Option& rhs) noexcept(noexcept_swapable) {
        using std::swap;
        swap(has, rhs.has);
        swap(ref(), rhs.ref());
    }
    constexpr void reset() noexcept {
        if (has) {
            destroy();
            has = false;
        }
    }
    constexpr T& operator*() & { return ref(); }
    constexpr T&& operator*() && { return std::move(ref()); }
    constexpr const T& operator*() const& { return ref(); }
    constexpr const T&& operator*() const&& { return std::move(ref()); }
    constexpr const T* operator->() const { return reinterpret_cast<const T*>(buffer); }
    constexpr T* operator->() { return reinterpret_cast<T*>(buffer); }
    constexpr explicit operator bool() const noexcept { return has; }
    constexpr bool has_value() const noexcept { return has; }
    constexpr const T& value() const& {
        if (!has) throw gsh::Exception("gsh::Option::value / The object doesn't have value.");
        return ref();
    }
    constexpr T& value() & {
        if (!has) throw gsh::Exception("gsh::Option::value / The object doesn't have value.");
        return ref();
    }
    constexpr T&& value() && {
        if (!has) throw gsh::Exception("gsh::Option::value / The object doesn't have value.");
        return std::move(ref());
    }
    constexpr const T&& value() const&& {
        if (!has) throw gsh::Exception("gsh::Option::value / The object doesn't have value.");
        return std::move(ref());
    }
    template<class U> constexpr T value_or(U&& v) const& { return has ? ref() : static_cast<T>(std::forward<U>(v)); }
    template<class U> constexpr T value_or(U&& v) && { return has ? static_cast<T>(std::move(ref())) : static_cast<T>(std::forward<U>(v)); }
};
template<class T> Option(T) -> Option<T>;

template<class T, class U> constexpr bool operator==(const Option<T>& x, const Option<U>& y) {
    if (x.has_value() && y.has_value()) return *x == *y;
    else return x.has_value() ^ y.has_value();
}
template<class T> constexpr bool operator==(const Option<T>& x, NullOpt) noexcept {
    return !x.has_value();
}
template<class T, class U> constexpr bool operator==(const Option<T>& x, const U& y) {
    return x.has_value() && (*x == y);
}
template<class T, class U> constexpr bool operator==(const T& x, const Option<U>& y) {
    return y.has_value() && (x == *y);
}
template<class T, std::three_way_comparable_with<T> U> constexpr std::compare_three_way_result_t<T, U> operator<=>(const Option<T>& x, const Option<U>& y) {
    return x.has_value() && y.has_value() ? *x <=> *y : x.has_value() <=> y.has_value();
}
template<class T> constexpr std::strong_ordering operator<=>(const Option<T>& x, NullOpt) noexcept {
    return x.has_value <=> false;
}
template<class T, std::three_way_comparable_with<T> U> constexpr std::compare_three_way_result_t<T, U> operator<=>(const Option<T>& x, const U& y) {
    return x.has_value() ? *x <=> y : std::strong_ordering::less;
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
        if (x.has_value()) return *x;
        else return T();
    } else return x.has_value();
}
template<std::size_t N, class T> auto get(Option<T>& x) {
    if constexpr (N == 0) {
        if (x.has_value()) return *x;
        else return T();
    } else return x.has_value();
}
template<std::size_t N, class T> auto get(Option<T>&& x) {
    if constexpr (N == 0) {
        if (x.has_value()) return std::move(*x);
        else return T();
    } else return x.has_value();
}
}  

namespace gsh {

namespace internal {

    
    template<class T> constexpr T calc_gcd(T x, T y) {
        if (x == 0 || y == 0) [[unlikely]]
            return x | y;
        const itype::i32 n = std::countr_zero(x);
        const itype::i32 m = std::countr_zero(y);
        const itype::i32 l = n < m ? n : m;
        x >>= n;
        y >>= m;
        while (x != y) {
            const T s = y < x ? x - y : y - x;
            const itype::i32 t = std::countr_zero(s);
            y = y < x ? y : x;
            x = s >> t;
        }
        return x << l;
    }

    template<class T> class ModintImpl : public T {
        using base_type = T;
    public:
        using value_type = std::decay_t<decltype(base_type::mod())>;
        using modint_type = ModintImpl;
        constexpr static bool is_staticmod = !requires { base_type::set_mod(0); };
        constexpr ModintImpl() noexcept : T() {}
        template<class U> constexpr ModintImpl(U x) noexcept { operator=(x); }
        constexpr explicit operator value_type() const noexcept { return val(); }
        constexpr static void set_mod(value_type x) {
            static_assert(!is_staticmod, "gsh::internal::ModintImpl::set_mod / Mod must be dynamic.");
            if (x <= 1) throw Exception("gsh::internal::ModintImpl::set_mod / Mod must be at least 2.");
            if (x == mod()) return;
            base_type::set_mod(x);
        }
        constexpr value_type val() const noexcept { return base_type::val(); }
        constexpr static value_type mod() noexcept { return base_type::mod(); }
        template<class U> constexpr modint_type& operator=(U x) noexcept {
            static_assert(std::is_integral_v<U>, "gsh::internal::ModintImpl::operator= / Only integer types can be assigned.");
            if constexpr (std::is_unsigned_v<U>) {
                if constexpr (std::is_same_v<U, unsigned long long> || std::is_same_v<U, unsigned long>) base_type::assign(static_cast<itype::u64>(x));
                else base_type::assign(static_cast<itype::u32>(x));
            } else {
                if (x < 0) {
                    if constexpr (std::is_same_v<U, long long> || std::is_same_v<U, long>) base_type::assign(static_cast<itype::u64>(-x));
                    else base_type::assign(static_cast<itype::u32>(-x));
                    base_type::neg();
                } else {
                    if constexpr (std::is_same_v<U, long long> || std::is_same_v<U, long>) base_type::assign(static_cast<itype::u64>(x));
                    else base_type::assign(static_cast<itype::u32>(x));
                }
            }
            return *this;
        }
        constexpr static modint_type raw(value_type x) noexcept {
            modint_type res;
            res.rawassign(x);
            return res;
        }
        template<class Istream> friend Istream& operator>>(Istream& ist, modint_type& x) {
            value_type n;
            ist >> n;
            x = n;
            return ist;
        }
        template<class Ostream> friend Ostream& operator<<(Ostream& ost, modint_type x) { return ost << x.val(); }
        constexpr Option<modint_type> inv() const noexcept {
            value_type a = 1, b = 0, x = val(), y = mod();
            if (x == 0) return Null;
            while (true) {
                if (x <= 1) {
                    if (x == 0) [[unlikely]]
                        break;
                    else return modint_type(a);
                }
                b += a * (y / x);
                y %= x;
                if (y <= 1) {
                    if (y == 0) [[unlikely]]
                        break;
                    else return modint_type(mod() - b);
                }
                a += b * (x / y);
                x %= y;
            }
            return Null;
        }
        constexpr modint_type pow(itype::u64 e) const noexcept {
            modint_type res = modint_type::raw(1), pow = *this;
            while (e) {
                modint_type tmp = pow * pow;
                if (e & 1) res *= pow;
                pow = tmp;
                e >>= 1;
            }
            return res;
        }
        constexpr modint_type operator+() const noexcept { return *this; }
        constexpr modint_type operator-() const noexcept {
            modint_type res = *this;
            res.neg();
            return res;
        }
        constexpr modint_type& operator++() noexcept {
            base_type::inc();
            return *this;
        }
        constexpr modint_type& operator--() noexcept {
            base_type::dec();
            return *this;
        }
        constexpr modint_type operator++(int) noexcept {
            modint_type copy = *this;
            operator++();
            return copy;
        }
        constexpr modint_type operator--(int) noexcept {
            modint_type copy = *this;
            operator--();
            return copy;
        }
        constexpr modint_type& operator+=(modint_type x) noexcept {
            base_type::add(x);
            return *this;
        }
        constexpr modint_type& operator-=(modint_type x) noexcept {
            base_type::sub(x);
            return *this;
        }
        constexpr modint_type& operator*=(modint_type x) noexcept {
            base_type::mul(x);
            return *this;
        }
        constexpr modint_type& operator/=(modint_type x) {
            auto iv = x.inv();
            if (!iv) throw gsh::Exception("gsh::internal::ModintImpl::operator/= / Cannot calculate inverse.");
            operator*=(*iv);
            return *this;
        }
        friend constexpr modint_type operator+(modint_type l, modint_type r) noexcept { return modint_type(l) += r; }
        friend constexpr modint_type operator-(modint_type l, modint_type r) noexcept { return modint_type(l) -= r; }
        friend constexpr modint_type operator*(modint_type l, modint_type r) noexcept { return modint_type(l) *= r; }
        friend constexpr modint_type operator/(modint_type l, modint_type r) { return modint_type(l) /= r; }
        friend constexpr bool operator==(modint_type l, modint_type r) noexcept { return l.val() == r.val(); }
        friend constexpr bool operator!=(modint_type l, modint_type r) noexcept { return l.val() != r.val(); }
        constexpr int legendre() const noexcept {
            value_type res = pow((mod() - 1) >> 1).val();
            return (res <= 1 ? static_cast<int>(res) : -1);
        }
        constexpr int jacobi(bool skip_calc_gcd = false) const noexcept {
            value_type a = val(), n = mod();
            if (a == 1) return 1;
            if (!skip_calc_gcd && calc_gcd(a, n) != 1) return 0;
            int res = 1;
            while (a != 0) {
                while (!(a & 1) && a != 0) {
                    a >>= 1;
                    res = ((n & 0b111) == 3 || (n & 0b111) == 5) ? -res : res;
                }
                res = ((a & 0b11) == 3 || (n & 0b11) == 3) ? -res : res;
                value_type tmp = n;
                n = a;
                a = tmp;
                a %= n;
            }
            return n == 1 ? res : 0;
        }
        constexpr Option<modint_type> sqrt() const noexcept {
            const value_type vl = val(), md = mod();
            if (vl <= 1) return *this;
            auto get_min = [](modint_type x) {
                return x.val() > (mod() >> 1) ? -x : x;
            };
            if ((md & 0b11) == 3) {
                modint_type res = pow((md + 1) >> 2);
                if (res * res != *this) return Null;
                else return get_min(res);
            } else if ((md & 0b111) == 5) {
                modint_type res = pow((md + 3) >> 3);
                if constexpr (is_staticmod) {
                    constexpr modint_type p = modint_type::raw(2).pow((md - 1) >> 2);
                    res *= p;
                } else if (res * res != *this) res *= modint_type::raw(2).pow((md - 1) >> 2);
                if (res * res != *this) return Null;
                else return get_min(res);
            } else {
                const itype::u32 S = std::countr_zero(md - 1);
                const value_type Q = (md - 1) >> S;
                const itype::u32 W = std::bit_width(md);
                if (S * S <= 12 * W) {
                    const modint_type tmp = pow(Q / 2);
                    modint_type R = tmp * (*this), t = R * tmp;
                    if (t.val() == 1) return R;
                    modint_type u = t;
                    for (itype::u32 i = 0; i != S - 1; ++i) u *= u;
                    if (u.val() != 1) return Null;
                    const modint_type z = [&]() {
                        if (md % 3 == 2) return modint_type::raw(3);
                        if (auto x = md % 5; x == 2 || x == 3) return modint_type::raw(5);
                        if (auto x = md % 7; x == 3 || x == 5 || x == 6) return modint_type::raw(7);
                        if (auto x = md % 11; x == 2 || x == 6 || x == 7 || x == 8 || x == 10) return modint_type(11);
                        if (auto x = md % 13; x == 2 || x == 5 || x == 6 || x == 7 || x == 8 || x == 11) return modint_type(13);
                        for (const itype::u32 x : { 17, 19, 23, 29, 31, 37, 41, 43, 47, 53, 59, 61, 67, 71, 73, 79, 83, 89, 97 }) {
                            if (modint_type(x).legendre() == -1) return modint_type(x);
                        }
                        modint_type z = 101;
                        while (z.legendre() != -1) z += 2;
                        return z;
                    }();
                    itype::u32 M = S;
                    modint_type c = z.pow(Q);
                    do {
                        modint_type U = t * t;
                        itype::u32 i = 1;
                        while (U.val() != 1) U = U * U, ++i;
                        modint_type b = c;
                        for (itype::u32 j = 0, k = M - i - 1; j < k; ++j) b *= b;
                        M = i, c = b * b, t *= c, R *= b;
                    } while (t.val() != 1);
                    return get_min(R);
                } else {
                    if (legendre() != 1) return Null;
                    modint_type a = 2;
                    while ((a * a - *this).legendre() != -1) ++a;
                    modint_type res1 = modint_type::raw(1), res2, pow1 = a, pow2 = modint_type::raw(1), w = a * a - *this;
                    value_type e = (md + 1) / 2;
                    while (true) {
                        const modint_type tmp2 = pow2 * w;
                        if (e & 1) {
                            const modint_type tmp = res1;
                            res1 = res1 * pow1 + res2 * tmp2;
                            res2 = tmp * pow2 + res2 * pow1;
                        }
                        e >>= 1;
                        if (e == 0) return get_min(res1);
                        const modint_type tmp = pow1;
                        pow1 = pow1 * pow1 + pow2 * tmp2;
                        pow2 *= tmp + tmp;
                    }
                }
            }
        }
    };

    template<itype::u32 mod_> class StaticModint32_impl {
        using value_type = itype::u32;
        using modint_type = StaticModint32_impl;
        value_type val_ = 0;
    protected:
        constexpr StaticModint32_impl() noexcept {}
        constexpr value_type val() const noexcept { return val_; }
        static constexpr value_type mod() noexcept { return mod_; }
        constexpr void assign(itype::u32 x) noexcept { val_ = x % mod_; }
        constexpr void assign(itype::u64 x) noexcept { val_ = x % mod_; }
        constexpr void rawassign(value_type x) noexcept { val_ = x; }
        constexpr void neg() noexcept { val_ = (val_ == 0 ? 0 : mod_ - val_); }
        constexpr void inc() noexcept { val_ = (val_ == mod_ - 1 ? 0 : val_ + 1); }
        constexpr void dec() noexcept { val_ = (val_ == 0 ? mod_ - 1 : val_ - 1); }
        constexpr void add(modint_type x) noexcept {
            if (mod_ - val_ > x.val_) val_ += x.val_;
            else val_ = x.val_ - (mod_ - val_);
        }
        constexpr void sub(modint_type x) noexcept {
            if (val_ >= x.val_) val_ -= x.val_;
            else val_ = mod_ - (x.val_ - val_);
        }
        constexpr void mul(modint_type x) noexcept { val_ = static_cast<itype::u64>(val_) * x.val_ % mod_; }
    };

    template<itype::u64 mod_> class StaticModint64_impl {
        using value_type = itype::u64;
        using modint_type = StaticModint64_impl;
        value_type val_ = 0;
    protected:
        constexpr StaticModint64_impl() noexcept {}
        constexpr value_type val() const noexcept { return val_; }
        static constexpr value_type mod() noexcept { return mod_; }
        constexpr void assign(itype::u32 x) noexcept {
            if constexpr (mod_ < (1ull << 32)) val_ = x % mod_;
            else val_ = x;
        }
        constexpr void assign(itype::u64 x) noexcept { val_ = x % mod_; }
        constexpr void rawassign(value_type x) noexcept { val_ = x; }
        constexpr void neg() noexcept { val_ = (val_ == 0 ? 0 : mod_ - val_); }
        constexpr void inc() noexcept { val_ = (val_ == mod_ - 1 ? 0 : val_ + 1); }
        constexpr void dec() noexcept { val_ = (val_ == 0 ? mod_ - 1 : val_ - 1); }
        constexpr void add(modint_type x) noexcept {
            if (mod_ - val_ > x.val_) val_ += x.val_;
            else val_ = x.val_ - (mod_ - val_);
        }
        constexpr void sub(modint_type x) noexcept {
            if (val_ >= x.val_) val_ -= x.val_;
            else val_ = mod_ - (x.val_ - val_);
        }
        constexpr void mul(modint_type x) noexcept {
            if constexpr (mod_ < (1ull << 63)) {
                constexpr itype::u128 M_ = std::numeric_limits<itype::u128>::max() / mod_ + std::has_single_bit(mod_);
                const value_type a = (((M_ * val_) >> 64) * x.val_) >> 64;
                const value_type b = val_ * x.val_;
                const value_type c = a * mod_;
                const value_type d = b - c;
                const bool e = d < mod_;
                const value_type f = d - mod_;
                val_ = e ? d : f;
            } else {
                constexpr itype::u128 M_ = std::numeric_limits<itype::u128>::max() / mod_ + std::has_single_bit(mod_);
                const value_type a = (((M_ * val_) >> 64) * x.val_) >> 64;
                const itype::u128 b = (itype::u128) val_ * x.val_;
                const itype::u128 c = (itype::u128) a * mod_;
                const itype::u128 d = b - c;
                const bool e = d < mod_;
                const itype::u128 f = d - mod_;
                val_ = e ? d : f;
            }
        }
    };

    template<int id> class DynamicModint32_impl {
        using value_type = itype::u32;
        using modint_type = DynamicModint32_impl;
        static inline value_type mod_ = 0;
        static inline itype::u64 M_ = 0;
        value_type val_ = 0;
    protected:
        DynamicModint32_impl() noexcept {}
        static void set_mod(value_type newmod) noexcept {
            mod_ = newmod;
            M_ = std::numeric_limits<itype::u64>::max() / mod_ + std::has_single_bit(mod_);
        }
        value_type val() const noexcept { return val_; }
        static value_type mod() noexcept { return mod_; }
        void assign(itype::u32 x) noexcept { val_ = x % mod_; }
        void assign(itype::u64 x) noexcept { val_ = x % mod_; }
        void rawassign(value_type x) noexcept { val_ = x; }
        void neg() noexcept { val_ = (val_ == 0 ? 0 : mod_ - val_); }
        void inc() noexcept { val_ = (val_ == mod_ - 1 ? 0 : val_ + 1); }
        void dec() noexcept { val_ = (val_ == 0 ? mod_ - 1 : val_ - 1); }
        void add(modint_type x) noexcept {
            const auto tmp = val_ + x.val_;
            const auto tmp2 = mod_ - val_;
            val_ = tmp2 > x.val_ ? tmp : x.val_ - tmp2;
        }
        void sub(modint_type x) noexcept {
            if (val_ >= x.val_) val_ -= x.val_;
            else val_ = mod_ - (x.val_ - val_);
        }
        void mul(modint_type x) noexcept {
            const itype::u64 a = (itype::u64) val_ * x.val_;
            const itype::u64 b = ((itype::u128) M_ * a) >> 64;
            const itype::u64 c = a - b * mod_;
            val_ = c < mod_ ? c : c - mod_;
            
        }
    };

    template<int id> class DynamicModint64_impl {
        using value_type = itype::u64;
        using modint_type = DynamicModint64_impl;
        static inline value_type mod_ = 0;
        static inline itype::u128 M_ = 0;
        value_type val_ = 0;
    protected:
        DynamicModint64_impl() noexcept {}
        static void set_mod(value_type newmod) noexcept {
            mod_ = newmod;
            M_ = std::numeric_limits<itype::u128>::max() / mod_ + std::has_single_bit(mod_);
        }
        value_type val() const noexcept { return val_; }
        static value_type mod() noexcept { return mod_; }
        void assign(itype::u32 x) noexcept { val_ = x % mod_; }
        void assign(itype::u64 x) noexcept { val_ = x % mod_; }
        void rawassign(value_type x) noexcept { val_ = x; }
        void neg() noexcept { val_ = (val_ == 0 ? 0 : mod_ - val_); }
        void inc() noexcept { val_ = (val_ == mod_ - 1 ? 0 : val_ + 1); }
        void dec() noexcept { val_ = (val_ == 0 ? mod_ - 1 : val_ - 1); }
        void add(modint_type x) noexcept {
            const auto tmp = val_ + x.val_;
            const auto tmp2 = mod_ - val_;
            val_ = tmp2 > x.val_ ? tmp : x.val_ - tmp2;
        }
        void sub(modint_type x) noexcept {
            if (val_ >= x.val_) val_ -= x.val_;
            else val_ = mod_ - (x.val_ - val_);
        }
        void mul(modint_type x) noexcept {
            const value_type a = (((M_ * val_) >> 64) * x.val_) >> 64;
            const value_type b = val_ * x.val_;
            const value_type c = a * mod_;
            const value_type d = b - c;
            const bool e = d < mod_;
            const value_type f = d - mod_;
            val_ = e ? d : f;
        }
    };

}  

template<itype::u32 mod_ = 998244353> using StaticModint32 = internal::ModintImpl<internal::StaticModint32_impl<mod_>>;
template<itype::u64 mod_ = 998244353> using StaticModint64 = internal::ModintImpl<internal::StaticModint64_impl<mod_>>;
template<itype::u64 mod_ = 998244353> using StaticModint = std::conditional_t<(mod_ < (1ull << 32)), StaticModint32<mod_>, StaticModint64<mod_>>;
template<int id = 0> using DynamicModint32 = internal::ModintImpl<internal::DynamicModint32_impl<id>>;
template<int id = 0> using DynamicModint64 = internal::ModintImpl<internal::DynamicModint64_impl<id>>;
template<int id = 0> using DynamicModint = DynamicModint64<id>;

}  



namespace gsh {


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
        
        const static itype::u64 table[147] = {0x2001002010213u,0x200001000020001u,0x20100010000u,0x10000200000010u,0x200000001u,0x20000000010u,0x200000000010000u,0x1200000000u,0x20000u,0x2000002000201u,0x1000000201u,0x20002100000u,0x10000000010000u,0x1000000000200u,0x2000000000010u,0x2010002u,0x100001u,0x20002u,0x210u,0x1000200000200u,0x110000u,0x2000000u,0x201001100000000u,0x2000100000000u,0x2000002000000u,0x201u,
        0x20002u,0x10001000000002u,0x200000000000000u,0x2100000u,0x10012u,0x200020100000000u,0x20100000000u,0x2000000000010u,0x1000200100200u,0u,0x10001000000003u,0x1200000000u,0x10000000000000u,0x2000002000010u,0x21000000001u,0x20100000000u,0x10000000010000u,0x200000200000000u,0u,0x2001000010200u,0x1000020000u,0x20000u,0x12000000000000u,0x1000200000201u,0x2020000100000u,0x10000002010000u,0x1001000000000u,0x20000u,
        0x2000000u,0x1u,0x10000000130000u,0x2u,0x201000300000200u,0x2000000100010u,0x2000010u,0x200001000000001u,0x100000002u,0x2000000000000u,0x1000000000201u,0x2010000u,0x10000000000002u,0x200020100000000u,0x100020010u,0x10u,0x200u,0x20100100000u,0x1000010000u,0x201000200020200u,0x2000000u,0x2000000000002u,0x21000000000u,0x20000000000u,0x13000000000010u,0x1u,0x20000000002u,0x10000002010001u,0x200000200020000u,
        0x100020000u,0x2000200000000u,0x1000000000u,0x120000u,0x211000000000000u,0x1000200000200u,0x100000u,0x2010201u,0x1000020001u,0x10020000020000u,0u,0x200000001u,0x100010u,0x200000000000002u,0x201001200000000u,0x100020000u,0x2000210u,0x1000000201u,0x10000100100000u,0x200000002u,0x1000000000200u,0x2000000000010u,0x2000000000012u,0x200000000000000u,0x20100020000u,0x10000000000010u,0x1000000000200u,0x20000110000u,
        0x10000u,0x201000200000000u,0x2000100000000u,0x3000000000000u,0x1000100000u,0x20000000000u,0x10001000010002u,0x200000000020000u,0x2000000u,0x2010010u,0x200000000000001u,0x20100020000u,0x203000000000000u,0x200100000u,0x100000u,0x10001002000001u,0x1001200000000u,0u,0x2000000u,0x1000000201u,0x20000020000u,0x200000000010002u,0x200000000u,0x100000u,0x212u,0x200001000000000u,0x100030000u,0x200000010u,0x1000000000201u,
        0x2000000100000u,0x2000002u,0x1000000000000u,0x20000u,0x2000000000011u,0u,0u};
        
        return (table[x / 64] >> (x % 64)) & 1;
    }
}  
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

}  


#ifdef EVAL
gsh::MmapReader r;
#else
gsh::BasicReader r;
#endif
gsh::BasicWriter w;
int main() {
    [[maybe_unused]] gsh::Rand32 engine;
    {
        using namespace std;
        using namespace gsh;
        using namespace gsh::itype;
        using namespace gsh::ftype;
        using namespace gsh::ctype;
        u64 N = Parser<u64>{}(r);
        QuotientsList q(N);
        Formatter<u32>{}(w, q.size());
        Formatter<c8>{}(w, '\n');
        for (u32 i = 0; i != q.iota_limit(); ++i) {
            Formatter<u8dig>{}(w, u8dig{ i + 1 });
            Formatter<c8>{}(w, ' ');
        }
        for (u32 i = q.div_limit(); i != 0; --i) {
            Formatter<u16dig>{}(w, u16dig{ N / i });
            Formatter<c8>{}(w, ' ');
        }
        
        
    }
}

