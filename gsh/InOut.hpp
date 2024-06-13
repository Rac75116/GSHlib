#pragma once
#include <bit>
#include <cstring>          // std::memset, std::memcpy, std::memmove
#include <unistd.h>         // read
#include <gsh/TypeDef.hpp>  // gsh::itype, gsh::ctype
#include <iostream>
#include <cstdint>

namespace gsh {

template<class T> class Parser;

template<> class Parser<itype::u32> {
public:
    template<class Stream> constexpr itype::u32 operator()(Stream& stream) const {
        if (!stream.reload(16)) return 0;
        itype::u64 x2 = 0;
        {
            itype::u64 v;
            memcpy(&v, stream.current(), 8);
            if (!((v -= 0x3030303030303030) & 0x8080808080808080)) {
                v = (v * 10 + (v >> 8)) & 0xff00ff00ff00ff;
                v = (v * 100 + (v >> 16)) & 0xffff0000ffff;
                v = (v * 10000 + (v >> 32)) & 0xffffffff;
                x2 = v;
                stream.bump(8);
            }
        }
        {
            itype::u32 v;
            memcpy(&v, stream.current(), 4);
            if (!((v -= 0x30303030) & 0x80808080)) {
                v = (v * 10 + (v >> 8)) & 0xff00ff;
                v = (v * 100 + (v >> 16)) & 0xffff;
                x2 = 10000 * x2 + v;
                stream.bump(4);
            }
        }
        {
            itype::u16 v;
            memcpy(&v, stream.current(), 2);
            if (!((v -= 0x3030) & 0x8080)) {
                v = (v * 10 + (v >> 8)) & 0xff;
                x2 = 100 * x2 + v;
                stream.bump(2);
            }
        }
        if (' ' < *stream.current()) {
            x2 = x2 * 10 + *stream.current() - '0';
            stream.bump(1);
        }
        stream.bump(1);
        return x2;
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