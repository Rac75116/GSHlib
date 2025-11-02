#pragma once
#include "TypeDef.hpp"
#include "Memory.hpp"
#include "Range.hpp"
#include "Arr.hpp"
#include <string>
#include <string_view>
#include <charconv>

namespace gsh {

namespace internal {

    template<class CharT, class Alloc = Allocator<CharT>> class StrImpl : public std::basic_string<CharT, std::char_traits<CharT>, Alloc>, public ViewInterface<StrImpl<CharT, Alloc>, CharT> {
        using base = std::basic_string<CharT, std::char_traits<CharT>, Alloc>;
        using view = ViewInterface<StrImpl<CharT, Alloc>, CharT>;
    public:
        constexpr StrImpl() noexcept(noexcept(Alloc())) : StrImpl(Alloc()) {}
        constexpr explicit StrImpl(const Alloc& a) noexcept : base(a) {}
        constexpr StrImpl(const StrImpl&) = default;
        constexpr StrImpl(StrImpl&&) noexcept = default;
        constexpr StrImpl(const StrImpl& str, itype::u32 pos, itype::u32 n, const Alloc& a = Alloc()) : base(str, pos, n, a) {}
        constexpr StrImpl(const StrImpl& str, itype::u32 pos, const Alloc& a = Alloc()) : base(str, pos, a) {}
        constexpr StrImpl(StrImpl&& str, itype::u32 pos, itype::u32 n, const Alloc& a = Alloc()) : base(std::move(str), pos, n, a) {}
        constexpr StrImpl(const CharT* s, itype::u32 n, const Alloc& a = Alloc()) : base(s, n, a) {}
        constexpr StrImpl(const CharT* s, const Alloc& a = Alloc()) : base(s, a) {}
        StrImpl(std::nullptr_t) = delete;
        constexpr StrImpl(itype::u32 n, CharT c, const Alloc& a = Alloc()) : base(n, c, a) {}
        template<class InputIterator> constexpr StrImpl(InputIterator begin, InputIterator end, const Alloc& a = Alloc()) : base(begin, end, a) {}
        constexpr StrImpl(std::initializer_list<CharT> init, const Alloc& a = Alloc()) : base(std::move(init), a) {}
        constexpr StrImpl(const StrImpl& str, const Alloc& a) : base(str, a) {}
        constexpr StrImpl(StrImpl&& str, const Alloc& a) : base(str, a) {}
        template<class T> constexpr explicit StrImpl(const T& t, const Alloc& a = Alloc()) : base(t, a) {}
        template<class T> constexpr StrImpl(const T& t, itype::u32 pos, itype::u32 n, const Alloc& a = Alloc()) : base(t, pos, n, a) {}
        constexpr itype::u32 size() const noexcept { return base::size(); }
        constexpr itype::u32 length() const noexcept { return base::length(); }
        template<class... Args> constexpr auto find(Args&&... args) const { return view::find(std::forward<Args>(args)...); }
        template<class... Args> constexpr auto rfind(Args&&... args) const { return view::rfind(std::forward<Args>(args)...); }
        template<class... Args> constexpr auto copy(Args&&... args) const { return view::copy(std::forward<Args>(args)...); }
        template<class... Args> constexpr auto contains(Args&&... args) const { return base::contains(std::forward<Args>(args)...); }
    };

    template<class CharT> class StrViewImpl : public std::basic_string_view<CharT>, public ViewInterface<StrViewImpl<CharT>, CharT> {
        using base = std::basic_string_view<CharT>;
        using view = ViewInterface<StrViewImpl<CharT>, CharT>;
    public:
        constexpr StrViewImpl() noexcept : base() {}
        constexpr StrViewImpl(const StrViewImpl&) noexcept = default;
        constexpr StrViewImpl(const CharT* str) : base(str) {}
        StrViewImpl(std::nullptr_t) = delete;
        constexpr StrViewImpl(const CharT* str, itype::u32 len) : base(str, len) {}
        template<class It, class End> constexpr StrViewImpl(It begin, End end) : base(begin, end) {}
        template<class R> constexpr explicit StrViewImpl(R&& r) : base(std::forward<R>(r)) {}
        constexpr itype::u32 size() const noexcept { return base::size(); }
        constexpr itype::u32 length() const noexcept { return base::length(); }
        template<class... Args> constexpr auto find(Args&&... args) const { return view::find(std::forward<Args>(args)...); }
        template<class... Args> constexpr auto rfind(Args&&... args) const { return view::rfind(std::forward<Args>(args)...); }
        template<class... Args> constexpr auto copy(Args&&... args) const { return view::copy(std::forward<Args>(args)...); }
        template<class... Args> constexpr auto contains(Args&&... args) const { return base::contains(std::forward<Args>(args)...); }
    };

    template<class CharT> constexpr void swap(StrImpl<CharT>& x, StrImpl<CharT>& y) noexcept(noexcept(x.swap(y))) {
        x.swap(y);
    }
    template<class CharT> constexpr void swap(StrViewImpl<CharT>& x, StrViewImpl<CharT>& y) noexcept(noexcept(x.swap(y))) {
        x.swap(y);
    }

    template<class CharT, class Alloc> StrImpl<CharT, Alloc> operator+(const StrImpl<CharT, Alloc>& lhs, const StrImpl<CharT, Alloc>& rhs) {
        StrImpl<CharT, Alloc> res = lhs;
        res.append(rhs);
        return res;
    }
    template<class CharT, class Alloc> StrImpl<CharT, Alloc> operator+(const StrImpl<CharT, Alloc>& lhs, const CharT* rhs) {
        StrImpl<CharT, Alloc> res = lhs;
        res.append(rhs);
        return res;
    }
    template<class CharT, class Alloc> StrImpl<CharT, Alloc> operator+(const CharT* lhs, const StrImpl<CharT, Alloc>& rhs) {
        StrImpl<CharT, Alloc> res = lhs;
        res.append(rhs);
        return res;
    }
    template<class CharT, class Alloc> StrImpl<CharT, Alloc> operator+(const StrImpl<CharT, Alloc>& lhs, CharT rhs) {
        StrImpl<CharT, Alloc> res = lhs;
        res.push_back(rhs);
        return res;
    }
    template<class CharT, class Alloc> StrImpl<CharT, Alloc> operator+(CharT lhs, const StrImpl<CharT, Alloc>& rhs) {
        StrImpl<CharT, Alloc> res(1, lhs);
        res.append(rhs);
        return res;
    }

}  // namespace internal

using Str = internal::StrImpl<ctype::c8>;
using Str8 = internal::StrImpl<ctype::utf8>;
using Str16 = internal::StrImpl<ctype::utf16>;
using Str32 = internal::StrImpl<ctype::utf32>;
using Strw = internal::StrImpl<ctype::wc>;
using StrView = internal::StrViewImpl<ctype::c8>;
using StrView8 = internal::StrViewImpl<ctype::utf8>;
using StrView16 = internal::StrViewImpl<ctype::utf16>;
using StrView32 = internal::StrViewImpl<ctype::utf32>;
using StrVieww = internal::StrViewImpl<ctype::wc>;

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
template<> class Formatter<StrView> {
public:
    template<class Stream> constexpr void operator()(Stream&& stream, const StrView& str) const {
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
template<> class Formatter<Str> {
public:
    template<class Stream> constexpr void operator()(Stream&& stream, const Str& str) const { Formatter<StrView>{}(stream, StrView(str.data(), str.size())); }
};

template<class... Args>
    requires((std::same_as<Args, ctype::c8> && ...) && sizeof...(Args) >= 1)
constexpr Str UniteChars(Args... c) {
    return Str{ c... };
}

template<std::integral T> constexpr Str ToStr(const T& val, const itype::i32 base = 10) {
    char buf[sizeof(T) * 8];
    char* last = std::to_chars(buf, buf + sizeof(T) * 8, val, base).ptr;
    return Str(buf, last);
}
template<std::floating_point T> constexpr Str ToStr(const T& val, const itype::i32 precision = 10) {
    char buf[64];
    auto result = std::to_chars(buf, buf + 64, val, std::chars_format::fixed, precision);
    if (result.ec != std::errc()) {
        throw Exception("gsh::ToStr / Conversion error in floating point to string.");
    }
    return Str(buf, result.ptr);
}
template<class T> constexpr T FromStr(const StrView& str, const itype::i32 base = 10) {
    T res{};
    auto result = std::from_chars(str.data(), str.data() + str.size(), res, base);
    if (result.ec != std::errc() || result.ptr != str.data() + str.size()) {
        throw Exception("gsh::FromStr / Conversion error in string to number.");
    }
    return res;
}

}  // namespace gsh
