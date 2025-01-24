#pragma once
#include "TypeDef.hpp"
#include "Memory.hpp"
#include <string>

namespace gsh {

namespace internal {

    template<class CharT> using StrImpl = std::basic_string<CharT>;

    template<class CharT> using StrViewImpl = std::basic_string_view<CharT>;

}  // namespace internal

using Str = internal::StrImpl<ctype::c8>;
using Str8 = internal::StrImpl<ctype::utf8>;
using Str16 = internal::StrImpl<ctype::utf16>;
using Str32 = internal::StrImpl<ctype::utf32>;
using Strw = internal::StrImpl<ctype::wc>;

template<class T> class Parser;
template<> class Parser<Str> {
public:
    template<class Stream> constexpr Str operator()(Stream&& stream) const {
        stream.reload(16);
        Str res;
        while (true) {
            const ctype::c8* e = stream.current();
            while (*e >= '!') ++e;
            const itype::u32 len = e - stream.current();
            const itype::u32 curlen = res.size();
            res.resize(curlen + len);
            MemoryCopy(res.data() + curlen, stream.current(), len);
            stream.skip(len);
            if (stream.avail() == 0) stream.reload();
            else break;
        }
        stream.skip(1);
        return res;
    }
    template<class Stream> constexpr Str operator()(Stream&& stream, itype::u32 n) const {
        itype::u32 rem = n;
        Str res;
        itype::u32 avail = stream.avail();
        while (avail <= rem) {
            const itype::u32 curlen = res.size();
            res.resize(curlen + avail);
            MemoryCopy(res.data() + curlen, stream.current(), avail);
            rem -= avail;
            stream.skip(avail);
            if (rem == 0) return res;
            stream.reload();
            avail = stream.avail();
        }
        const itype::u32 curlen = res.size();
        res.resize(curlen + rem);
        MemoryCopy(res.data() + curlen, stream.current(), rem);
        stream.skip(rem + 1);
        return res;
    }
};

template<class T> class Formatter;
template<> class Formatter<Str> {
public:
    template<class Stream> constexpr void operator()(Stream&& stream, const Str& str) const {
        const ctype::c8* s = str.data();
        itype::u32 len = str.size();
        itype::u32 avail = stream.avail();
        if (avail >= len) [[likely]] {
            MemoryCopy(stream.current(), s, len);
            stream.skip(len);
        } else {
            MemoryCopy(stream.current(), s, avail);
            len -= avail;
            s += avail;
            stream.skip(avail);
            while (len != 0) {
                stream.reload();
                avail = stream.avail();
                const itype::u32 tmp = len < avail ? len : avail;
                MemoryCopy(stream.current(), s, tmp);
                len -= tmp;
                s += tmp;
                stream.skip(tmp);
            }
        }
    }
};

}  // namespace gsh
