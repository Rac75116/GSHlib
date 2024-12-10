#pragma once
#include <cstdlib>  // std::exit
#include <cstring>  // std::memcpy, std::memmove
#include <utility>  // std::forward
#include <tuple>    // std::tuple, std::make_tuple
#if __has_include(<unistd.h>)
#include <unistd.h>  // read, write
#endif
#ifndef _WIN32
#include <sys/mman.h>  // mmap
#include <sys/stat.h>  // stat, fstat
#endif
#include "TypeDef.hpp"     // gsh::itype, gsh::ctype
#include "Parser.hpp"      // gsh::Parser
#include "Formatter.hpp"   // gsh::Formatter
#include "Functional.hpp"  // gsh::Invoke
#include "Util.hpp"        // gsh::TypeArr

namespace gsh {

namespace internal {
    template<class D> class IstreamInterface;
}  // namespace internal

template<class D, class Types, class... Args> class ParsingChain;

class NoParsingResult {
    template<class D, class Types, class... Args> friend class ParsingChain;
    constexpr NoParsingResult() noexcept {}
    NoParsingResult(const NoParsingResult&) = delete;
    NoParsingResult(NoParsingResult&&) = delete;
};
class CustomParser {
    ~CustomParser() = delete;
};

template<class D, class... Types, class... Args> class ParsingChain<D, TypeArr<Types...>, Args...> {
    friend class internal::IstreamInterface<D>;
    template<class D2, class Types2, class... Args2> friend class ParsingChain;
    D& ref;
    [[no_unique_address]] std::tuple<Args...> args;
    GSH_INTERNAL_INLINE constexpr ParsingChain(D& r, std::tuple<Args...>&& a) noexcept : ref(r), args(std::move(a)) {}
    template<class... Options>
        requires(sizeof...(Args) < sizeof...(Types))
    GSH_INTERNAL_INLINE constexpr auto next_chain(Options&&... options) const noexcept {
        return ParsingChain<D, TypeArr<Types...>, Args..., std::tuple<Options...>>(ref, std::tuple_cat(args, std::make_tuple(std::forward_as_tuple(std::forward<Options>(options)...))));
    };
public:
    ParsingChain() = delete;
    ParsingChain(const ParsingChain&) = delete;
    ParsingChain(ParsingChain&&) = delete;
    ParsingChain& operator=(const ParsingChain&) = delete;
    ParsingChain& operator=(ParsingChain&&) = delete;
    template<class... Options>
        requires(sizeof...(Args) == 0)
    [[nodiscard]] constexpr auto option(Options&&... options) const noexcept {
        return next_chain(std::forward<Options>(options)...);
    }
    template<class... Options>
        requires(sizeof...(Args) != 0)
    [[nodiscard]] constexpr auto operator()(Options&&... options) const noexcept {
        return next_chain(std::forward<Options>(options)...);
    }
    template<std::size_t N> friend constexpr decltype(auto) get(const ParsingChain& chain) {
        auto get_result = [](auto&& parser, auto&&... args) GSH_INTERNAL_INLINE -> decltype(auto) {
            if constexpr (std::is_void_v<std::invoke_result_t<decltype(parser), decltype(args)...>>) {
                Invoke(std::forward<decltype(parser)>(parser), std::forward<decltype(args)>(args)...);
                return NoParsingResult{};
            } else {
                return Invoke(std::forward<decltype(parser)>(parser), std::forward<decltype(args)>(args)...);
            }
        };
        using value_type = typename TypeArr<Types...>::template type<N>;
        if constexpr (N < sizeof...(Args)) {
            if constexpr (std::same_as<CustomParser, value_type>) {
                return std::apply([get_result, &chain](auto&& parser, auto&&... args) -> decltype(auto) { return get_result(std::forward<decltype(parser)>(parser), chain.ref, std::forward<decltype(args)>(args)...); }, std::get<N>(chain.args));
            } else {
                return std::apply([get_result, &chain](auto&&... args) -> decltype(auto) { return get_result(Parser<value_type>(), chain.ref, std::forward<decltype(args)>(args)...); }, std::get<N>(chain.args));
            }
        } else {
            return get_result(Parser<value_type>(), chain.ref);
        }
    }
    constexpr void ignore() const noexcept {
        [this]<itype::u32... I>(std::integer_sequence<itype::u32, I...>) {
            (..., get<I>(*this));
        }(std::make_integer_sequence<itype::u32, sizeof...(Types)>());
    }
    template<class T> constexpr operator T() const noexcept {
        static_assert(sizeof...(Types) == 1);
        return static_cast<T>(get<0>(*this));
    }
    constexpr decltype(auto) val() const noexcept {
        static_assert(sizeof...(Types) == 1);
        return get<0>(*this);
    }
    template<class... To>
        requires(sizeof...(To) == 0 || sizeof...(To) == sizeof...(Types))
    constexpr auto bind() const noexcept {
        if constexpr (sizeof...(To) == 0) {
            return [this]<itype::u32... I>(std::integer_sequence<itype::u32, I...>) {
                return std::tuple{ get<I>(*this)... };
            }(std::make_integer_sequence<itype::u32, sizeof...(Types)>());
        } else {
            return [this]<itype::u32... I>(std::integer_sequence<itype::u32, I...>) {
                return std::tuple<To...>{ static_cast<To>(get<I>(*this))... };
            }(std::make_integer_sequence<itype::u32, sizeof...(Types)>());
        }
    }
};

}  // namespace gsh

namespace std {
template<class D, class... Types, class... Args> class tuple_size<gsh::ParsingChain<D, gsh::TypeArr<Types...>, Args...>> : public integral_constant<size_t, sizeof...(Types)> {};
template<size_t N, class D, class... Types, class... Args> class tuple_element<N, gsh::ParsingChain<D, gsh::TypeArr<Types...>, Args...>> {
public:
    using type = decltype(get<N>(std::declval<const gsh::ParsingChain<D, gsh::TypeArr<Types...>, Args...>&>()));
};
}  // namespace std

namespace gsh {

namespace internal {
    template<class D> class IstreamInterface {
        constexpr D& derived() { return *static_cast<D*>(this); }
    public:
        template<class T, class... Types> [[nodiscard]] constexpr auto read() { return ParsingChain<D, TypeArr<T, Types...>>(derived(), std::tuple<>()); }
    };
    template<class D> class OstreamInterface {
        constexpr D& derived() { return *static_cast<D*>(this); }
    public:
        template<class Sep> constexpr void write_sep(Sep&&) {}
        template<class Sep, class T, class... Args> constexpr void write_sep(Sep&& sep, T&& x, Args&&... args) {
            Formatter<std::decay_t<T>>{}(derived(), std::forward<T>(x));
            if constexpr (sizeof...(Args) != 0) {
                Formatter<std::decay_t<Sep>>{}(derived(), sep);
                write_sep(std::forward<Sep>(sep), std::forward<Args>(args)...);
            }
        }
        template<class... Args> constexpr void write(Args&&... args) { write_sep(' ', std::forward<Args>(args)...); }
        template<class Sep, class... Args> constexpr void writeln_sep(Sep&& sep, Args&&... args) {
            write_sep(std::forward<Sep>(sep), std::forward<Args>(args)...);
            Formatter<ctype::c8>{}(derived(), '\n');
        }
        template<class... Args> constexpr void writeln(Args&&... args) {
            write_sep(' ', std::forward<Args>(args)...);
            Formatter<ctype::c8>{}(derived(), '\n');
        }
    };
}  // namespace internal

template<itype::u32 Bufsize = (1 << 17)> class BasicReader : public internal::IstreamInterface<BasicReader<Bufsize>> {
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
class MmapReader : public internal::IstreamInterface<MmapReader> {
    [[maybe_unused]] const itype::i32 fh;
    [[maybe_unused]] ctype::c8 *buf, *cur, *eof;
public:
    MmapReader() : fh(0) {
#ifdef _WIN32
        write(1, "gsh::MmapReader / gsh::MmapReader is not available for Windows.\n", 64);
        std::exit(1);
#else
        struct stat st;
        fstat(0, &st);
        buf = reinterpret_cast<ctype::c8*>(mmap(nullptr, st.st_size, PROT_READ, MAP_PRIVATE, 0, 0));
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
class StaticStrReader : public internal::IstreamInterface<StaticStrReader> {
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

template<itype::u32 Bufsize = (1 << 17)> class BasicWriter : public internal::OstreamInterface<BasicWriter<Bufsize>> {
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
class StaticStrWriter : public internal::OstreamInterface<StaticStrWriter> {
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
