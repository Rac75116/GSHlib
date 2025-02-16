#pragma once
#include <tuple>          // std::tuple_size, std::tuple_element
#include <utility>        // std::integer_sequence, std::make_index_sequence
#include <ranges>         // std::ranges::forward_range
#include <charconv>       // std::to_chars, std::chars_format, std::errc
#include "TypeDef.hpp"    // gsh::itype, gsh::ctype
#include "Util.hpp"       // gsh::MemoryCopy, gsh::StrLen
#include "Exception.hpp"  // gsh::Exception
#include "Int128.hpp"     // gsh::itype::u128, gsh::itype::i128

namespace gsh {

namespace itype {
    struct i4dig;
    struct u4dig;
    struct i8dig;
    struct u8dig;
    struct i16dig;
    struct u16dig;
}  // namespace itype

template<class T> class Formatter;

namespace internal {
#ifndef GSH_USE_COMPILE_TIME_CALCULATION
    struct InttoStrT {
#define GSH_INTERNAL_INCLUDE_INTTOSTR "internal/InttoStr.txt"
        const ctype::c8* table =
#include GSH_INTERNAL_INCLUDE_INTTOSTR
    };
    template<itype::u32> constexpr InttoStrT InttoStr;
#else
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
#endif
    template<class Stream> constexpr void Formatu16(Stream&& stream, itype::u16 n) {
        auto copy1 = [&](itype::u16 x) {
            itype::u32 off = (x < 10) + (x < 100) + (x < 1000);
            MemoryCopy(stream.current(), InttoStr<0>.table + (4 * x + off), 4);
            stream.skip(4 - off);
        };
        auto copy2 = [&](itype::u16 x) {
            MemoryCopy(stream.current(), InttoStr<0>.table + 4 * x, 4);
            stream.skip(4);
        };
        if (n < 10000) copy1(n);
        else {
            copy1(n / 10000);
            copy2(n % 10000);
        }
    }
    template<class Stream> constexpr void Formatu32(Stream&& stream, itype::u32 n) {
        auto copy1 = [&](itype::u32 x) {
            itype::u32 off = (x < 10) + (x < 100) + (x < 1000);
            MemoryCopy(stream.current(), InttoStr<0>.table + (4 * x + off), 4);
            stream.skip(4 - off);
        };
        auto copy2 = [&](itype::u32 x) {
            MemoryCopy(stream.current(), InttoStr<0>.table + 4 * x, 4);
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
    template<class Stream> constexpr void Formatu64(Stream&& stream, itype::u64 n) {
        auto copy1 = [&](itype::u32 x) {
            itype::u32 off = (x < 10) + (x < 100) + (x < 1000);
            MemoryCopy(stream.current(), InttoStr<0>.table + (4 * x + off), 4);
            stream.skip(4 - off);
        };
        auto copy2 = [&](itype::u32 x) {
            MemoryCopy(stream.current(), InttoStr<0>.table + 4 * x, 4);
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
    template<class Stream> constexpr void Formatu128(Stream&& stream, itype::u128 n) {
        auto copy1 = [&](itype::u32 x) {
            itype::u32 off = (x < 10) + (x < 100) + (x < 1000);
            MemoryCopy(stream.current(), InttoStr<0>.table + (4 * x + off), 4);
            stream.skip(4 - off);
        };
        auto copy2 = [&](itype::u32 x) {
            MemoryCopy(stream.current(), InttoStr<0>.table + 4 * x, 4);
            stream.skip(4);
        };
        constexpr itype::u128 t = static_cast<itype::u128>(10000000000000000) * 10000000000000000;
        if (n >= t) {
            const itype::u32 dv = n / t;
            n -= dv * t;
            if (dv >= 10000) {
                copy1(dv / 10000);
                copy2(dv % 10000);
            } else copy1(dv);
            auto [a, b] = Divu128(n >> 64, n, 10000000000000000);
            const itype::u32 c = a / 100000000, d = a % 100000000, e = b / 100000000, f = b % 100000000;
            copy2(c / 10000), copy2(c % 10000);
            copy2(d / 10000), copy2(d % 10000);
            copy2(e / 10000), copy2(e % 10000);
            copy2(f / 10000), copy2(f % 10000);
        } else {
            auto [a, b] = Divu128(n >> 64, n, 10000000000000000);
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
    template<class Stream> constexpr void Formatu4dig(Stream&& stream, itype::u16 x) {
        itype::u32 off = (x < 10) + (x < 100) + (x < 1000);
        MemoryCopy(stream.current(), InttoStr<0>.table + (4 * x + off), 4);
        stream.skip(4 - off);
    }
    template<class Stream> constexpr void Formatu8dig(Stream&& stream, itype::u32 x) {
        const itype::u32 n = x;
        auto copy1 = [&](itype::u32 x) {
            itype::u32 off = (x < 10) + (x < 100) + (x < 1000);
            MemoryCopy(stream.current(), InttoStr<0>.table + (4 * x + off), 4);
            stream.skip(4 - off);
        };
        auto copy2 = [&](itype::u32 x) {
            MemoryCopy(stream.current(), InttoStr<0>.table + 4 * x, 4);
            stream.skip(4);
        };
        if (n < 10000) copy1(n);
        else {
            copy1(n / 10000);
            copy2(n % 10000);
        }
    }
    template<class Stream> constexpr void Formatu16dig(Stream&& stream, itype::u64 x) {
        const itype::u64 n = x;
        auto copy1 = [&](itype::u64 x) {
            itype::u32 off = (x < 10) + (x < 100) + (x < 1000);
            MemoryCopy(stream.current(), InttoStr<0>.table + (4 * x + off), 4);
            stream.skip(4 - off);
        };
        auto copy2 = [&](itype::u64 x) {
            MemoryCopy(stream.current(), InttoStr<0>.table + 4 * x, 4);
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
    template<class Stream> constexpr void operator()(Stream&& stream, itype::u16 n) const {
        stream.reload(8);
        internal::Formatu16(stream, n);
    }
};
template<> class Formatter<itype::i16> {
public:
    template<class Stream> constexpr void operator()(Stream&& stream, itype::i16 n) const {
        stream.reload(8);
        *stream.current() = '-';
        stream.skip(n < 0);
        internal::Formatu16(stream, n < 0 ? -n : n);
    }
};
template<> class Formatter<itype::u32> {
public:
    template<class Stream> constexpr void operator()(Stream&& stream, itype::u32 n) const {
        stream.reload(16);
        internal::Formatu32(stream, n);
    }
};
template<> class Formatter<itype::i32> {
public:
    template<class Stream> constexpr void operator()(Stream&& stream, itype::i32 n) const {
        stream.reload(16);
        *stream.current() = '-';
        stream.skip(n < 0);
        internal::Formatu32(stream, n < 0 ? -n : n);
    }
};
template<> class Formatter<itype::u64> {
public:
    template<class Stream> constexpr void operator()(Stream&& stream, itype::u64 n) const {
        stream.reload(32);
        internal::Formatu64(stream, n);
    }
};
template<> class Formatter<itype::i64> {
public:
    template<class Stream> constexpr void operator()(Stream&& stream, itype::i64 n) const {
        stream.reload(32);
        *stream.current() = '-';
        stream.skip(n < 0);
        internal::Formatu64(stream, n < 0 ? -n : n);
    }
};
template<> class Formatter<itype::u128> {
public:
    template<class Stream> constexpr void operator()(Stream&& stream, itype::u128 n) const {
        stream.reload(64);
        internal::Formatu128(stream, n);
    }
};
template<> class Formatter<itype::i128> {
public:
    template<class Stream> constexpr void operator()(Stream&& stream, itype::i128 n) const {
        stream.reload(64);
        *stream.current() = '-';
        stream.skip(n < 0);
        internal::Formatu128(stream, n < 0 ? -n : n);
    }
};
template<> class Formatter<itype::u4dig> {
public:
    template<class Stream> constexpr void operator()(Stream&& stream, itype::u16 n) const {
        stream.reload(4);
        internal::Formatu4dig(stream, n);
    }
};
template<> class Formatter<itype::i4dig> {
public:
    template<class Stream> constexpr void operator()(Stream&& stream, itype::i16 n) const {
        stream.reload(5);
        *stream.current() = '-';
        stream.skip(n < 0);
        internal::Formatu4dig(stream, static_cast<itype::u16>(n < 0 ? -n : n));
    }
};
template<> class Formatter<itype::u8dig> {
public:
    template<class Stream> constexpr void operator()(Stream&& stream, itype::u32 n) const {
        stream.reload(8);
        internal::Formatu8dig(stream, n);
    }
};
template<> class Formatter<itype::i8dig> {
public:
    template<class Stream> constexpr void operator()(Stream&& stream, itype::i32 n) const {
        stream.reload(9);
        *stream.current() = '-';
        stream.skip(n < 0);
        internal::Formatu8dig(stream, static_cast<itype::u32>(n < 0 ? -n : n));
    }
};
template<> class Formatter<itype::u16dig> {
public:
    template<class Stream> constexpr void operator()(Stream&& stream, itype::u64 n) const {
        stream.reload(16);
        internal::Formatu16dig(stream, n);
    }
};
template<> class Formatter<itype::i16dig> {
public:
    template<class Stream> constexpr void operator()(Stream&& stream, itype::i64 n) const {
        stream.reload(17);
        *stream.current() = '-';
        stream.skip(n < 0);
        internal::Formatu16dig(stream, static_cast<itype::u64>(n < 0 ? -n : n));
    }
};
template<> class Formatter<ctype::c8> {
public:
    template<class Stream> constexpr void operator()(Stream&& stream, ctype::c8 c) const {
        stream.reload(1);
        *stream.current() = c;
        stream.skip(1);
    }
};

namespace io {

    enum class FormatterOption : std::underlying_type_t<std::chars_format> {};
    constexpr FormatterOption operator|(FormatterOption a, FormatterOption b) noexcept {
        return static_cast<FormatterOption>(static_cast<std::underlying_type_t<FormatterOption>>(a) | static_cast<std::underlying_type_t<FormatterOption>>(b));
    }
    constexpr auto Fixed = static_cast<FormatterOption>(std::chars_format::fixed);
    constexpr auto General = static_cast<FormatterOption>(std::chars_format::general);
    constexpr auto Hex = static_cast<FormatterOption>(std::chars_format::hex);
    constexpr auto Scientific = static_cast<FormatterOption>(std::chars_format::scientific);

}  // namespace io

namespace internal {
    template<class T> class FloatFormatter {
    public:
        template<class Stream> constexpr void operator()(Stream&& stream, T f, io::FormatterOption fmt = io::Fixed, itype::i32 precision = 12) {
            stream.reload(32);
            auto [ptr, err] = std::to_chars(stream.current(), stream.current() + stream.avail(), f, static_cast<std::chars_format>(fmt), precision);
            if (err != std::errc{}) [[unlikely]] {
                stream.reload();
                auto [ptr, err] = std::to_chars(stream.current(), stream.current() + stream.avail(), f, static_cast<std::chars_format>(fmt), precision);
                if (err != std::errc{}) throw Exception("gsh::internal::FloatFormatter::operator() / The value is too large.");
                stream.skip(ptr - stream.current());
            } else {
                stream.skip(ptr - stream.current());
            }
        }
    };
}  // namespace internal
template<> class Formatter<float> : public internal::FloatFormatter<float> {};
template<> class Formatter<double> : public internal::FloatFormatter<double> {};
template<> class Formatter<long double> : public internal::FloatFormatter<long double> {};
#ifdef __STDCPP_FLOAT16_T__
template<> class Formatter<std::float16_t> : public internal::FloatFormatter<std::float16_t> {};
#endif
#ifdef __STDCPP_FLOAT32_T__
template<> class Formatter<std::float32_t> : public internal::FloatFormatter<std::float32_t> {};
#endif
#ifdef __STDCPP_FLOAT64_T__
template<> class Formatter<std::float64_t> : public internal::FloatFormatter<std::float64_t> {};
#endif
#ifdef __STDCPP_FLOAT128_T__
template<> class Formatter<std::float128_t> : public internal::FloatFormatter<std::float128_t> {};
#endif
#ifdef __STDCPP_BFLOAT16_T__
template<> class Formatter<std::bfloat16_t> : public internal::FloatFormatter<std::bfloat16_t> {};
#endif
#ifdef __SIZEOF_FLOAT128__
template<> class Formatter<__float128> : public internal::FloatFormatter<__float128> {};
#endif
template<> class Formatter<ftype::InvalidFloat16Tag> {};
template<> class Formatter<ftype::InvalidFloat128Tag> {};
template<> class Formatter<ftype::InvalidBfloat16Tag> {};
template<> class Formatter<bool> {
public:
    template<class Stream> constexpr void operator()(Stream&& stream, bool b) const {
        stream.reload(1);
        *stream.current() = '0' + b;
        stream.skip(1);
    }
};
template<> class Formatter<const ctype::c8*> {
public:
    template<class Stream> constexpr void operator()(Stream&& stream, const ctype::c8* s) const { operator()(stream, s, StrLen(s)); }
    template<class Stream> constexpr void operator()(Stream&& stream, const ctype::c8* s, itype::u32 len) const {
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
template<> class Formatter<ctype::c8*> : public Formatter<const ctype::c8*> {};

class NoOutTag {};
constexpr NoOutTag NoOut;
template<> class Formatter<NoOutTag> {
public:
    template<class Stream> constexpr void operator()(Stream&&, NoOutTag) const {}
};

template<class R> concept FormatableRange = std::ranges::forward_range<R> && requires { sizeof(Formatter<std::decay_t<std::ranges::range_value_t<R>>>) != 0; };
template<FormatableRange R> class Formatter<R> {
    template<class Stream, class T, class U> constexpr void print(Stream&& stream, T&& r, U&& sep) const {
        auto first = std::ranges::begin(r);
        auto last = std::ranges::end(r);
        if (!(first != last)) return;
        Formatter<std::decay_t<std::ranges::range_value_t<R>>> formatter;
        while (true) {
            formatter(stream, *first);
            ++first;
            if (first != last) {
                Formatter<std::decay_t<U>>()(stream, sep);
            } else break;
        }
    }
public:
    template<class Stream, class T>
        requires std::same_as<std::decay_t<T>, R>
    constexpr void operator()(Stream&& stream, T&& r) const {
        print(std::forward<Stream>(stream), std::forward<T>(r), ' ');
    }
    template<class Stream, class T, class U>
        requires std::same_as<std::decay_t<T>, R>
    constexpr void operator()(Stream&& stream, T&& r, U&& sep) const {
        print(std::forward<Stream>(stream), std::forward<T>(r), std::forward<U>(sep));
    }
};

namespace internal {
    template<class T, class U> constexpr bool FormatableTupleImpl = false;
    template<class T, std::size_t... I> constexpr bool FormatableTupleImpl<T, std::integer_sequence<std::size_t, I...>> = (... && requires { sizeof(Formatter<std::decay_t<typename std::tuple_element<I, T>::type>>) != 0; });
}  // namespace internal
template<class T> concept FormatableTuple = requires { std::tuple_size<T>::value; } && internal::FormatableTupleImpl<T, std::make_index_sequence<std::tuple_size<T>::value>>;
template<FormatableTuple T>
    requires(!FormatableRange<T>)
class Formatter<T> {
    template<itype::u32 I, class Stream, class U, class Sep> constexpr void print_element(Stream&& stream, U&& x, Sep&& sep) const {
        using std::get;
        using element_type = std::decay_t<std::tuple_element_t<I, T>>;
        if constexpr (requires { x.template get<I>(); }) Formatter<element_type>()(stream, x.template get<I>());
        else Formatter<element_type>()(stream, get<I>(x));
        if constexpr (I < std::tuple_size_v<T> - 1) Formatter<std::decay_t<Sep>>()(stream, sep);
    }
    template<class Stream, class U, class Sep> constexpr void print(Stream&& stream, U&& x, Sep&& sep) const {
        [&]<itype::u32... I>(std::integer_sequence<itype::u32, I...>) {
            (..., print_element<I>(stream, x, sep));
        }(std::make_integer_sequence<itype::u32, std::tuple_size_v<T>>());
    }
public:
    template<class Stream, class U> constexpr void operator()(Stream&& stream, U&& x) const { print(std::forward<Stream>(stream), std::forward<U>(x), ' '); }
    template<class Stream, class U, class Sep> constexpr void operator()(Stream&& stream, U&& x, Sep&& sep) const { print(std::forward<Stream>(stream), std::forward<U>(x), std::forward<Sep>(sep)); }
};

}  // namespace gsh
