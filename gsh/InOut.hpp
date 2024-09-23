#pragma once
#include <cstdlib>  // std::exit
#include <cstring>  // std::memcpy, std::memmove
#include <utility>  // std::forward
#include <tuple>    // std::tuple, std::tuple_cat
#if __has_include(<unistd.h>)
#include <unistd.h>  // read, write
#endif
#ifndef _WIN32
#include <sys/mman.h>  // mmap
#include <sys/stat.h>  // stat, fstat
#endif
#include <gsh/TypeDef.hpp>     // gsh::itype, gsh::ctype
#include <gsh/Parser.hpp>      // gsh::Parser
#include <gsh/Formatter.hpp>   // gsh::Formatter
#include <gsh/Functional.hpp>  // gsh::Invoke

namespace gsh {

namespace internal {
    template<class D> class IstreamInterface {
        constexpr D& derived() { return *static_cast<D*>(this); }
        constexpr const D& derived() const { return *static_cast<const D*>(this); }
    public:
        constexpr auto read() { return std::tuple{}; }
        template<class T, class... Types> constexpr auto read() {
            if constexpr (sizeof...(Types) == 0) return Parser<T>{}(derived());
            else if constexpr (sizeof...(Types) == 1) {
                auto res = Parser<T>{}(derived());
                return std::tuple_cat(std::tuple(std::move(res)), std::tuple(read<Types...>()));
            } else {
                auto res = Parser<T>{}(derived());
                return std::tuple_cat(std::tuple(std::move(res)), read<Types...>());
            }
        }
    };
    template<class D> class OstreamInterface {
        constexpr D& derived() { return *static_cast<D*>(this); }
        constexpr const D& derived() const { return *static_cast<const D*>(this); }
    public:
        constexpr void write_sep(ctype::c8) {}
        template<class Sep, class T, class... Args> constexpr void write_sep(Sep&& sep, T&& x, Args&&... args) {
            Formatter<std::remove_cvref_t<T>>{}(derived(), std::forward<T>(x));
            if constexpr (sizeof...(Args) != 0) {
                Formatter<std::remove_cvref_t<Sep>>{}(derived(), std::forward<Sep>(sep));
                write(std::forward<Args>(args)...);
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
