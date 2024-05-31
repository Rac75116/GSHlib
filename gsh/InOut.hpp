#pragma once
#include <cstring>   // std::memset, std::memcpy
#include <unistd.h>  // read
#include <gsh/TypeDef.hpp>

namespace gsh {

template<class T> class Parser;

class BufReader {
public:
    using char_type = ctype::c8;
    using pos_type = itype::u32;
    using off_type = itype::i32;
private:
    constexpr static off_type Bufsize = 1 << 18;
    char_type buf[Bufsize + 1];
    char_type *cur = buf, *eof = buf;
public:
    BufReader() { cur = buf, eof = buf; }
    BufReader(const BufReader& rhs) {
        std::memcpy(buf, rhs.buf, rhs.eof - rhs.cur);
        cur = buf + (rhs.cur - rhs.buf);
        eof = buf + (rhs.cur - rhs.eof);
    }
    BufReader& operator=(const BufReader& rhs) {
        std::memcpy(buf, rhs.buf, rhs.eof - rhs.cur);
        cur = buf + (rhs.cur - rhs.buf);
        eof = buf + (rhs.cur - rhs.eof);
    }
    off_type in_avail() { return eof - cur; }
    void uflow() {
        off_type rem = eof - cur;
        std::memmove(buf, cur, rem);
        *(eof = buf + rem + read(0, buf + rem, Bufsize - rem)) = '\0';
        cur = buf;
    }
    char_type* eback() { return buf; }
    char_type* gptr() { return cur; }
    char_type* egptr() { return eof; }
    void gbump(off_type n) { cur += n; }
};


}  // namespace gsh