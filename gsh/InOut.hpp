#pragma once
#include <cstring>   // std::memset, std::memcpy
#include <unistd.h>  // read
#include <gsh/TypeDef.hpp>

namespace gsh {

template<class T> class Parser;

class IstreamBuf {
public:
    using char_type = ctype::c8;
    using pos_type = itype::u32;
    using off_type = itype::u32;
private:
    char_type buf[1 << 18];
    char_type *cur = buf, *eof = buf;
public:
    IstreamBuf() { cur = buf, eof = buf; }
    IstreamBuf(const IstreamBuf& rhs) {
        std::memcpy(buf, rhs.buf, rhs.eof - rhs.cur);
        cur = buf + (rhs.cur - rhs.buf);
        eof = buf + (rhs.cur - rhs.eof);
    }
    IstreamBuf& operator=(const IstreamBuf& rhs) {
        std::memcpy(buf, rhs.buf, rhs.eof - rhs.cur);
        cur = buf + (rhs.cur - rhs.buf);
        eof = buf + (rhs.cur - rhs.eof);
    }
    void swap(IstreamBuf& rhs) {
        IstreamBuf tmp = rhs;
        rhs = *this;
        *this = tmp;
    }
    friend void swap(IstreamBuf& lhs, IstreamBuf& rhs) { lhs.swap(rhs); }
};


}  // namespace gsh