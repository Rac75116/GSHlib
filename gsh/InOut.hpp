#pragma once
#include <bit>
#include <cstring>          // std::memset, std::memcpy, std::memmove
#include <unistd.h>         // read
#include <gsh/TypeDef.hpp>  // gsh::itype, gsh::ctype
#include <iostream>
#include <cstdint>

namespace gsh {

template<class T> class Parser;

namespace internal {
    template<class Stream> constexpr itype::u16 Parseu16(Stream& stream) {
        itype::u64 v;
        std::memcpy(&v, stream.current(), 8);
        v ^= 0x3030303030303030;
        itype::i32 tmp = std::countr_zero(v & 0xf0f0f0f0f0f0f0f0) >> 3;
        v <<= (64 - (tmp << 3));
        stream.bump(tmp + 1);
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
                stream.bump(8);
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
                stream.bump(4);
            }
        }
        {
            itype::u16 v = buf;
            if (!((v ^= 0x3030) & 0xf0f0)) {
                buf >>= 16;
                v = (v * 10 + (v >> 8)) & 0x00ff;
                res = 100 * res + v;
                stream.bump(2);
            }
        }
        {
            const ctype::c8 v = ctype::c8(buf) ^ 0x30;
            const bool f = !(v & 0xf0);
            res = f ? 10 * res + v : res;
            stream.bump(f + 1);
        }
        return res;
    };
}  // namespace internal

template<> class Parser<itype::u16> {
public:
    template<class Stream> constexpr itype::u16 operator()(Stream& stream) const {
        if (!stream.reload(8)) return 0;
        return internal::Parseu16(stream);
    }
};
template<> class Parser<itype::i16> {
public:
    template<class Stream> constexpr itype::i16 operator()(Stream& stream) const {
        if (!stream.reload(9)) return 0;
        const bool neg = *stream.current() == '-';
        stream.bump(neg);
        itype::i16 tmp = internal::Parseu16(stream);
        return neg ? -tmp : tmp;
    }
};
template<> class Parser<itype::u32> {
public:
    template<class Stream> constexpr itype::u32 operator()(Stream& stream) const {
        if (!stream.reload(16)) return 0;
        return internal::Parseu32(stream);
    }
};
template<> class Parser<itype::i32> {
public:
    template<class Stream> constexpr itype::i32 operator()(Stream& stream) const {
        if (!stream.reload(16)) return 0;
        const bool neg = *stream.current() == '-';
        stream.bump(neg);
        const itype::u32 tmp = internal::Parseu32(stream);
        return neg ? -tmp : tmp;
    }
};

class BasicReader {
private:
    constexpr static itype::u32 Bufsize = 1 << 18;
    ctype::c8 buf[Bufsize + 1];
    ctype::c8 *cur = buf, *eof = buf;
public:
    BasicReader() { cur = buf, eof = buf; }
    BasicReader(const BasicReader& rhs) {
        std::memcpy(buf, rhs.buf, rhs.eof - rhs.cur);
        cur = buf + (rhs.cur - rhs.buf);
        eof = buf + (rhs.cur - rhs.eof);
    }
    BasicReader& operator=(const BasicReader& rhs) {
        std::memcpy(buf, rhs.buf, rhs.eof - rhs.cur);
        cur = buf + (rhs.cur - rhs.buf);
        eof = buf + (rhs.cur - rhs.eof);
        return *this;
    }
    bool reload() {
        itype::u32 rem = eof - cur;
        std::memmove(buf, cur, rem);
        *(eof = buf + rem + read(0, buf + rem, Bufsize - rem)) = '\0';
        cur = buf;
        return true;
    }
    bool reload(itype::u32 len) {
        if (avail() < len) [[unlikely]] {
            reload();
            return avail() >= len;
        } else return true;
    }
    itype::u32 avail() const { return eof - cur; }
    const ctype::c8* current() const { return cur; }
    void bump(itype::u32 n) { cur += n; }
};

class StringReader {
private:
    const ctype::c8 *cur = nullptr, *eof = nullptr;
public:
    constexpr StringReader() {}
    constexpr StringReader(const char* c, itype::u32 n) : cur(c), eof(c + n) {}
    constexpr StringReader(const StringReader&) = default;
    constexpr StringReader& operator=(const StringReader&) = default;
    constexpr bool reload() { return true; }
    constexpr bool reload(itype::u32) { return true; }
    constexpr itype::u32 avail() const { return eof - cur; }
    constexpr const ctype::c8* current() const { return cur; }
    void bump(itype::u32 n) { cur += n; }
};

}  // namespace gsh