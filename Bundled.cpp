#define NDEBUG
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
    template<class Stream> constexpr void Formatu32(Stream& stream, itype::u32 n) {
        auto copy1 = [&](itype::u32 x) {
            itype::u32 off = 1 + (x >= 10) + (x >= 100) + (x >= 1000);
            std::memcpy(stream.current(), InttoStr<0>.table + (4 * x + (4 - off)), off);
            stream.skip(off);
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
}  

template<> class Formatter<itype::u32> {
public:
    template<class Stream> constexpr void operator()(Stream& stream, itype::u32 n) const {
        stream.reload(16);
        internal::Formatu32(stream, n);
    }
};
template<> class Formatter<itype::u64> {
public:
    template<class Stream> constexpr void operator()(Stream& stream, itype::u64 n) const {
        stream.reload(32);
        internal::Formatu64(stream, n);
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
    constexpr itype::u64 splitmix(itype::u64 x) {
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
    constexpr explicit Rand64(result_type value) : s0(value), s1(internal::splitmix(value)) {}
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
    constexpr void seed(result_type value = default_seed) { s0 = value, s1 = internal::splitmix(value); }
    friend constexpr bool operator==(Rand64 x, Rand64 y) { return x.s0 == y.s0 && x.s1 == y.s1; }
};


class Rand32 {
    itype::u64 val;
public:
    using result_type = itype::u32;
    static constexpr itype::usize word_size = sizeof(result_type) * 8;
    static constexpr result_type default_seed = 0xcafef00d;
    constexpr Rand32() : Rand32(default_seed) {}
    constexpr explicit Rand32(result_type value) : val(internal::splitmix((itype::u64) value << 32 | value)) {}
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
    constexpr void seed(result_type value = default_seed) { val = internal::splitmix((itype::u64) value << 32 | value); }
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
        itype::u64 a = internal::splitmix(static_cast<itype::u64>(std::time(nullptr)));
        itype::u64 b = internal::splitmix(static_cast<itype::u64>(std::clock()));
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
    using value_type = std::clock_t;
    ClockTimer() { start_time = std::clock(); }
    void restart() { start_time = std::clock(); }
    value_type elapsed() const { return (std::clock() - start_time) * 1000 / CLOCKS_PER_SEC; }
    void print() const { std::printf("gsh::ClockTimer::print / %llu ms\n", static_cast<unsigned long long>(elapsed())); }
};

}  

#include <iterator>           
#include <cstdlib>            
#include <tuple>              
#include <ranges>             

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
       

#define ALL(V)       std::begin(V), std::end(V)
#define RALL(V)      std::rbegin(V), std::rend(V)
#define ALLMID(V, n) std::begin(V), std::next(std::begin(V), n), std::end(V)
#define VALUE(...)   (([&]() __VA_ARGS__)())


#define RET_WITH(...) { __VA_ARGS__; return; } []{}
#define RETV_WITH(val, ...) { __VA_ARGS__; return val; } []{}
#define BRK_WITH(...) { __VA_ARGS__; break; } []{}
#define CTN_WITH(...) { __VA_ARGS__; continue; } []{}
#define EXT_WITH(...) { __VA_ARGS__; std::exit(0); } []{}
#define ARGS(...) __VA_ARGS__
#define EXPR(args, ...) [&](auto&&... GSH_EXPR_ARGS){ auto [args] = std::forward_as_tuple(GSH_EXPR_ARGS...); return (__VA_ARGS__); }


#define UNWRAP(var_name, ...) if (auto [var_name, GSH_UNWRAP_FLAG_##var_name] = (__VA_ARGS__); GSH_UNWRAP_FLAG_##var_name)
#define REP(var_name, ...)    for ([[maybe_unused]] auto var_name : gsh::Step(__VA_ARGS__))
#define RREP(var_name, ...)   for (auto var_name : gsh::Step(__VA_ARGS__) | std::ranges::reverse)
#include <memory>             
#include <iterator>           
#include <algorithm>          
#include <initializer_list>   
#include <type_traits>        
#include <cstring>            
#include <utility>            
    
  
#include <type_traits>      
#include <concepts>         
#include <utility>          
#include <iterator>         
#include <ranges>           
  

namespace gsh {

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
    constexpr derived_type& reverse() {
        std::ranges::reverse(get_ref());
        return get_ref();
    }
    constexpr auto reversed() { return SlicedRange{ get_rbegin(), get_rend() }; }
    constexpr auto reversed() const { return SlicedRange{ get_rbegin(), get_rend() }; }
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

}  

namespace gsh {

template<class T, class Allocator = std::allocator<T>>
    requires std::is_same_v<T, typename std::allocator_traits<Allocator>::value_type> && (!std::is_const_v<T>)
class Vec : public ViewInterface<Vec<T, Allocator>, T> {
    using traits = std::allocator_traits<Allocator>;
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
        if (std::is_trivially_copy_constructible_v<value_type> && !std::is_constant_evaluated()) {
            std::memcpy(ptr, x.ptr, sizeof(value_type) * len);
        } else {
            for (size_type i = 0; i != len; ++i) traits::construct(alloc, ptr + i, *(x.ptr + i));
        }
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
            if constexpr (std::is_trivially_move_constructible_v<value_type> && !std::is_constant_evaluated()) {
                std::memcpy(ptr, x.ptr, sizeof(value_type) * len);
            } else {
                for (size_type i = 0; i != len; ++i) traits::construct(alloc, ptr + i, std::move(*(x.ptr + i)));
            }
            traits::deallocate(x.alloc, x.ptr, x.cap);
            x.ptr = nullptr, x.len = 0, x.cap = 0;
        }
    }
    constexpr Vec(std::initializer_list<value_type> il, const allocator_type& a = Allocator()) : Vec(il.begin(), il.end(), a) {}
    constexpr ~Vec() {
        if (cap != 0) {
            if constexpr (!std::is_trivially_destructible_v<value_type>)
                for (size_type i = 0; i != len; ++i) traits::destroy(alloc, ptr + i);
            traits::deallocate(alloc, ptr, cap);
        }
    }
    constexpr Vec& operator=(const Vec& x) {
        if constexpr (!std::is_trivially_destructible_v<value_type>)
            for (size_type i = 0; i != len; ++i) traits::destroy(alloc, ptr + i);
        if (traits::propagate_on_container_copy_assignment || cap < x.len) {
            if (cap != 0) traits::deallocate(alloc, ptr, cap);
            if constexpr (traits::propagate_on_container_copy_assignment) alloc = x.alloc;
            cap = x.len;
            ptr = traits::allocate(alloc, cap);
        }
        len = x.len;
        if (std::is_trivially_copy_assignable_v<value_type> && !std::is_constant_evaluated()) {
            std::memcpy(ptr, x.ptr, sizeof(value_type) * len);
        } else {
            for (size_type i = 0; i != len; ++i) *(ptr + i) = *(x.ptr + i);
        }
        return *this;
    }
    constexpr Vec& operator=(Vec&& x) noexcept(traits::propagate_on_container_move_assignment::value || traits::is_always_equal::value) {
        if (cap != 0) {
            if constexpr (!std::is_trivially_destructible_v<value_type>)
                for (size_type i = 0; i != len; ++i) traits::destroy(alloc, ptr + i);
            traits::deallocate(alloc, ptr, cap);
        }
        if constexpr (traits::propagate_on_container_move_assignment) alloc = std::move(x.alloc);
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
                if (std::is_trivially_move_constructible_v<value_type> && !std::is_constant_evaluated()) {
                    std::memcpy(new_ptr, ptr, sizeof(value_type) * len);
                } else {
                    for (size_type i = 0; i != len; ++i) traits::construct(alloc, new_ptr + i, std::move(*(ptr + i)));
                }
                if constexpr (!std::is_trivially_destructible_v<value_type>)
                    for (size_type i = 0; i != len; ++i) traits::destroy(alloc, ptr + i);
                traits::deallocate(alloc, ptr, cap);
            }
            ptr = new_ptr;
            if constexpr (!std::is_trivially_default_constructible_v<value_type>)
                for (size_type i = len; i != sz; ++i) traits::construct(alloc, *(ptr + i));
            else std::memset(ptr + len, 0, sizeof(value_type) * (sz - len));
            len = sz, cap = sz;
        } else if (len < sz) {
            if constexpr (!std::is_trivially_default_constructible_v<value_type>)
                for (size_type i = len; i != sz; ++i) traits::construct(alloc, *(ptr + i));
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
                if (std::is_trivially_move_constructible_v<value_type> && !std::is_constant_evaluated()) {
                    std::memcpy(new_ptr, ptr, sizeof(value_type) * len);
                } else {
                    for (size_type i = 0; i != len; ++i) traits::construct(alloc, new_ptr + i, std::move(*(ptr + i)));
                }
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
                if (std::is_trivially_move_constructible_v<value_type> && !std::is_constant_evaluated()) {
                    std::memcpy(new_ptr, ptr, sizeof(value_type) * len);
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
            if (std::is_trivially_move_constructible_v<value_type> && !std::is_constant_evaluated()) {
                std::memcpy(new_ptr, ptr, sizeof(value_type) * len);
            } else {
                for (size_type i = 0; i != len; ++i) traits::construct(alloc, new_ptr + i, std::move(*(ptr + i)));
            }
            if constexpr (!std::is_trivially_destructible_v<value_type>)
                for (size_type i = 0; i != len; ++i) traits::destroy(alloc, ptr + i);
            traits::deallocate(alloc, ptr, cap);
            ptr = new_ptr, cap = len;
        }
    }
    constexpr reference operator[](const size_type n) {
#ifndef NDEBUG
        if (n >= len) [[unlikely]]
            throw gsh::Exception("gsh::Vec::operator[] / The index is out of range. ( n=", n, ", size=", len, " )");
#endif
        return *(ptr + n);
    }
    constexpr const_reference operator[](const size_type n) const {
#ifndef NDEBUG
        if (n >= len) [[unlikely]]
            throw gsh::Exception("gsh::Vec::operator[] / The index is out of range. ( n=", n, ", size=", len, " )");
#endif
        return *(ptr + n);
    }
    constexpr reference at(const size_type n) {
        if (n >= len) [[unlikely]]
            throw gsh::Exception("gsh::Vec::at / The index is out of range. ( n=", n, ", size=", len, " )");
        return *(ptr + n);
    }
    constexpr const_reference at(const size_type n) const {
        if (n >= len) [[unlikely]]
            throw gsh::Exception("gsh::Vec::at / The index is out of range. ( n=", n, ", size=", len, " )");
        return *(ptr + n);
    }
    constexpr reference at_unchecked(const size_type n) { return *(ptr + n); }
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
                if (std::is_trivially_move_constructible_v<value_type> && !std::is_constant_evaluated()) {
#pragma GCC diagnostic ignored "-Wclass-memaccess"
                    std::memcpy(new_ptr, ptr, sizeof(value_type) * len);
#pragma GCC diagnostic warning "-Wclass-memaccess"
                } else {
                    for (size_type i = 0; i != len; ++i) traits::construct(alloc, new_ptr + i, std::move(*(ptr + i)));
                }
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
    constexpr iterator insert(const const_iterator position, const value_type& x) {
        
    }
    constexpr iterator insert(const const_iterator position, value_type&& x) {
        
    }
    constexpr iterator insert(const const_iterator position, const size_type n, const value_type& x) {
        
    }
    template<class InputIter> constexpr iterator insert(const const_iterator position, const InputIter first, const InputIter last) {
        
    }
    constexpr iterator insert(const const_iterator position, const std::initializer_list<value_type> il) {
        
    }
    template<class... Args> constexpr iterator emplace(const_iterator position, Args&&... args) {
        
    }
    constexpr iterator erase(const_iterator position) {
        
    }
    constexpr iterator erase(const_iterator first, const_iterator last) {
        
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
template<std::input_iterator InputIter, class Allocator = std::allocator<typename std::iterator_traits<InputIter>::value_type>> Vec(InputIter, InputIter, Allocator = Allocator()) -> Vec<typename std::iterator_traits<InputIter>::value_type, Allocator>;

}  
#include <iostream>
#include <cassert>
#include <sstream>

gsh::MmapReader r;
gsh::BasicWriter w;
int main() {
    try {
        [[maybe_unused]] gsh::Rand64 engine;
        {
            using namespace std;
            using namespace gsh;
            using namespace gsh::itype;
            using namespace gsh::ftype;
            using namespace gsh::ctype;
            u32 N = Parser<u32>{}(r);
            for (u32 i = 0; i != N; ++i) {
                u64 A = Parser<u64>{}(r), B = Parser<u64>{}(r);
                Formatter<u64>{}(w, A + B);
                *w.current() = '\n';
                w.skip(1);
            }
            
        }
    } catch (gsh::Exception& e) {
        puts(e.what());
    }
}

