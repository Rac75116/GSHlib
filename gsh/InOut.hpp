#pragma once
#include <cstdlib>  // std::exit
#include <cstring>  // std::memcpy, std::memmove
#include <source_location>
#include <utility>         // std::forward
#include <tuple>           // std::tuple, std::make_tuple
#include "TypeDef.hpp"     // gsh::itype, gsh::ctype
#include "Parser.hpp"      // gsh::Parser
#include "Formatter.hpp"   // gsh::Formatter
#include "Functional.hpp"  // gsh::Invoke
#include "Util.hpp"        // gsh::TypeArr

#include <unistd.h>
#if defined(__linux__)
#include <sys/mman.h>  // mmap
#include <sys/stat.h>  // stat, fstat
#endif

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
    GSH_INTERNAL_INLINE constexpr ParsingChain(D& r, std::tuple<Args...>&& a) : ref(r), args(std::move(a)) {}
    template<class... Options>
        requires(sizeof...(Args) < sizeof...(Types))
    GSH_INTERNAL_INLINE constexpr auto next_chain(Options&&... options) const {
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
    [[nodiscard]] constexpr auto option(Options&&... options) const {
        return next_chain(std::forward<Options>(options)...);
    }
    template<class... Options>
        requires(sizeof...(Args) != 0)
    [[nodiscard]] constexpr auto operator()(Options&&... options) const {
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
    constexpr void ignore() const {
        [this]<itype::u32... I>(std::integer_sequence<itype::u32, I...>) {
            (..., get<I>(*this));
        }(std::make_integer_sequence<itype::u32, sizeof...(Types)>());
    }
    template<class T> constexpr operator T() const {
        static_assert(sizeof...(Types) == 1);
        return static_cast<T>(get<0>(*this));
    }
    constexpr decltype(auto) val() const {
        static_assert(sizeof...(Types) == 1);
        return get<0>(*this);
    }
    template<class... To>
        requires(sizeof...(To) == 0 || sizeof...(To) == sizeof...(Types))
    constexpr auto bind() const {
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

namespace internal {
    template<class D> class IstreamInterface {
        constexpr D& derived() { return *static_cast<D*>(this); }
    public:
        template<class T, class... Types> [[nodiscard]] constexpr auto read() { return ParsingChain<D, TypeArr<T, Types...>>(derived(), std::tuple<>()); }
    };
}  // namespace internal

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
    template<class D> class OstreamInterface;
}

class CustomFormatterTag {};
constexpr CustomFormatterTag CustomFormatter;

template<class D, class Sep, class SepFormatter, bool SepChanged, class End, class EndFormatter, bool EndChanged, class Types, class... Args> class FormattingChain;
template<class D, class Sep, class SepFormatter, bool SepChanged, class End, class EndFormatter, bool EndChanged, class... Types, class... Args> class FormattingChain<D, Sep, SepFormatter, SepChanged, End, EndFormatter, EndChanged, TypeArr<Types...>, Args...> {
    friend class internal::OstreamInterface<D>;
    template<class D2, class Sep2, class SepFormatter2, bool SepChanged2, class End2, class EndFormatter2, bool EndChanged2, class Types2, class... Args2> friend class FormattingChain;
    D& ref;
    [[no_unique_address]] Sep sepv;
    [[no_unique_address]] SepFormatter sep_formatter;
    [[no_unique_address]] End endv;
    [[no_unique_address]] EndFormatter end_formatter;
    std::tuple<Types...> value;
    [[no_unique_address]] std::tuple<Args...> args;
    bool used = false;
    template<class SU, class SFU, class EU, class EFU> constexpr FormattingChain(D& r, SU&& s, SFU&& sf, EU&& e, EFU&& ef, std::tuple<Types...>&& v, std::tuple<Args...>&& a) : ref(r), sepv(std::forward<SU>(s)), sep_formatter(std::forward<SFU>(sf)), endv(std::forward<EU>(e)), end_formatter(std::forward<EFU>(ef)), value(std::move(v)), args(std::move(a)) {}
    template<class... Options>
        requires(sizeof...(Args) < sizeof...(Types))
    GSH_INTERNAL_INLINE constexpr auto next_chain(Options&&... options) {
        used = true;
        return FormattingChain<D, Sep, SepFormatter, SepChanged, End, EndFormatter, EndChanged, TypeArr<Types...>, Args..., std::tuple<Options...>>(ref, sepv, sep_formatter, endv, end_formatter, std::move(value), std::tuple_cat(args, std::make_tuple(std::forward_as_tuple(std::forward<Options>(options)...))));
    };
    constexpr void print_all() {
        [this]<itype::u32... I>(std::integer_sequence<itype::u32, I...>) {
            auto format = [this]<itype::u32 Idx>(std::integral_constant<itype::u32, Idx>) {
                using value_type = std::decay_t<typename TypeArr<Types...>::template type<Idx>>;
                if constexpr (Idx >= sizeof...(Args)) Invoke(Formatter<value_type>(), ref, std::get<Idx>(value));
                else {
                    using args_type = typename std::tuple_element_t<Idx, decltype(args)>;
                    if constexpr (std::tuple_size_v<args_type> == 0) {
                        Invoke(Formatter<value_type>(), ref, std::get<Idx>(value));
                    } else {
                        using first_arg = typename std::tuple_element_t<0, args_type>;
                        if constexpr (std::same_as<first_arg, CustomFormatterTag>) {
                            [this]<itype::u32... J>(std::integer_sequence<itype::u32, J...>) {
                                Invoke(std::get<1>(std::get<Idx>(args)), ref, std::get<Idx>(value), std::get<J + 2>(std::get<Idx>(args))...);
                            }(std::make_integer_sequence<itype::u32, std::tuple_size_v<args_type> - 2>());
                        } else {
                            [this]<itype::u32... J>(std::integer_sequence<itype::u32, J...>) {
                                Invoke(Formatter<value_type>(), ref, std::get<Idx>(value), std::get<J>(std::get<Idx>(args))...);
                            }(std::make_integer_sequence<itype::u32, std::tuple_size_v<args_type>>());
                        }
                    }
                }
                if constexpr (Idx < sizeof...(Types) - 1) {
                    if constexpr (!std::same_as<value_type, NoOutTag>) Invoke(sep_formatter, ref, sepv);
                } else Invoke(end_formatter, ref, endv);
            };
            (..., format(std::integral_constant<itype::u32, I>()));
        }(std::make_integer_sequence<itype::u32, sizeof...(Types)>());
    }
public:
    constexpr static bool printable = []() {
        bool result = true;
        [&result]<itype::u32... I>(std::integer_sequence<itype::u32, I...>) {
            auto format = [&result]<itype::u32 Idx>(std::integral_constant<itype::u32, Idx>) {
                using value_type = std::decay_t<typename TypeArr<Types...>::template type<Idx>>;
                if constexpr (Idx >= sizeof...(Args)) result &= requires { Invoke(Formatter<value_type>(), ref, std::get<Idx>(value)); };
                else {
                    using args_type = typename std::tuple_element_t<Idx, decltype(args)>;
                    if constexpr (std::tuple_size_v<args_type> == 0) {
                        result &= requires { Invoke(Formatter<value_type>(), ref, std::get<Idx>(value)); };
                    } else {
                        using first_arg = typename std::tuple_element_t<0, args_type>;
                        if constexpr (std::same_as<first_arg, CustomFormatterTag>) {
                            result &= []<itype::u32... J>(std::integer_sequence<itype::u32, J...>) {
                                return requires { Invoke(std::get<1>(std::get<Idx>(args)), ref, std::get<Idx>(value), std::get<J + 2>(std::get<Idx>(args))...); };
                            }(std::make_integer_sequence<itype::u32, std::tuple_size_v<args_type> - 2>());
                        } else {
                            result &= []<itype::u32... J>(std::integer_sequence<itype::u32, J...>) {
                                return requires { Invoke(Formatter<value_type>(), ref, std::get<Idx>(value), std::get<J>(std::get<Idx>(args))...); };
                            }(std::make_integer_sequence<itype::u32, std::tuple_size_v<args_type>>());
                        }
                    }
                }
                if constexpr (Idx < sizeof...(Types) - 1) {
                    if constexpr (!std::same_as<value_type, NoOutTag>) result &= requires { Invoke(sep_formatter, ref, sepv); };
                } else result &= requires { Invoke(end_formatter, ref, endv); };
            };
            (..., format(std::integral_constant<itype::u32, I>()));
        }(std::make_integer_sequence<itype::u32, sizeof...(Types)>());
        return result;
    }();
    FormattingChain() = delete;
    FormattingChain(const FormattingChain&) = delete;
    FormattingChain(FormattingChain&&) = delete;
    FormattingChain& operator=(const FormattingChain&) = delete;
    FormattingChain& operator=(FormattingChain&&) = delete;
    constexpr ~FormattingChain() {
        if (used) return;
        if constexpr (printable) print_all();
        else std::exit(1);
    }
    template<class... Options>
        requires(sizeof...(Args) == 0 && !SepChanged && !EndChanged && !FormattingChain<D, Sep, SepFormatter, SepChanged, End, EndFormatter, EndChanged, TypeArr<Types...>, Args..., std::tuple<Options...>>::printable)
    [[nodiscard]] constexpr auto option(Options&&... options) {
        return next_chain(std::forward<Options>(options)...);
    }
    template<class... Options>
        requires(sizeof...(Args) == 0 && !SepChanged && !EndChanged && FormattingChain<D, Sep, SepFormatter, SepChanged, End, EndFormatter, EndChanged, TypeArr<Types...>, Args..., std::tuple<Options...>>::printable)
    constexpr auto option(Options&&... options) {
        return next_chain(std::forward<Options>(options)...);
    }
    template<class... Options>
        requires(sizeof...(Args) != 0 && !SepChanged && !EndChanged && !FormattingChain<D, Sep, SepFormatter, SepChanged, End, EndFormatter, EndChanged, TypeArr<Types...>, Args..., std::tuple<Options...>>::printable)
    [[nodiscard]] constexpr auto operator()(Options&&... options) {
        return next_chain(std::forward<Options>(options)...);
    }
    template<class... Options>
        requires(sizeof...(Args) != 0 && !SepChanged && !EndChanged && FormattingChain<D, Sep, SepFormatter, SepChanged, End, EndFormatter, EndChanged, TypeArr<Types...>, Args..., std::tuple<Options...>>::printable)
    constexpr auto operator()(Options&&... options) {
        return next_chain(std::forward<Options>(options)...);
    }
    template<class T, class F = Formatter<std::decay_t<T>>>
        requires(!SepChanged && !FormattingChain<D, T, F, true, End, EndFormatter, EndChanged, TypeArr<Types...>, Args...>::printable)
    [[nodiscard]] constexpr auto sep(T&& val, F&& formatter = {}) {
        used = true;
        return FormattingChain<D, T, F, true, End, EndFormatter, EndChanged, TypeArr<Types...>, Args...>(ref, std::forward<T>(val), std::forward<F>(formatter), endv, end_formatter, std::move(value), std::move(args));
    }
    template<class T, class F = Formatter<std::decay_t<T>>>
        requires(!SepChanged && FormattingChain<D, T, F, true, End, EndFormatter, EndChanged, TypeArr<Types...>, Args...>::printable)
    constexpr auto sep(T&& val, F&& formatter = {}) {
        used = true;
        return FormattingChain<D, T, F, true, End, EndFormatter, EndChanged, TypeArr<Types...>, Args...>(ref, std::forward<T>(val), std::forward<F>(formatter), endv, end_formatter, std::move(value), std::move(args));
    }
    template<class T, class F = Formatter<std::decay_t<T>>>
        requires(!EndChanged && !FormattingChain<D, Sep, SepFormatter, SepChanged, T, F, true, TypeArr<Types...>, Args...>::printable)
    [[nodiscard]] constexpr auto end(T&& val, F&& formatter = {}) {
        used = true;
        return FormattingChain<D, Sep, SepFormatter, SepChanged, T, F, true, TypeArr<Types...>, Args...>(ref, sepv, sep_formatter, std::forward<T>(val), std::forward<F>(formatter), std::move(value), std::move(args));
    }
    template<class T, class F = Formatter<std::decay_t<T>>>
        requires(!EndChanged && FormattingChain<D, Sep, SepFormatter, SepChanged, T, F, true, TypeArr<Types...>, Args...>::printable)
    constexpr auto end(T&& val, F&& formatter = {}) {
        used = true;
        return FormattingChain<D, Sep, SepFormatter, SepChanged, T, F, true, TypeArr<Types...>, Args...>(ref, sepv, sep_formatter, std::forward<T>(val), std::forward<F>(formatter), std::move(value), std::move(args));
    }
};

namespace internal {
    template<class D> class OstreamInterface {
        constexpr D& derived() { return *static_cast<D*>(this); }
    public:
        template<class... Args> [[nodiscard]] constexpr auto write(Args&&... args) { return FormattingChain<D, ctype::c8, Formatter<ctype::c8>, false, const NoOutTag&, Formatter<NoOutTag>, false, TypeArr<Args...>>(derived(), ' ', Formatter<ctype::c8>(), NoOut, Formatter<NoOutTag>(), std::forward_as_tuple(std::forward<Args>(args)...), {}); }
        template<class... Args>
            requires FormattingChain<D, ctype::c8, Formatter<ctype::c8>, false, const NoOutTag&, Formatter<NoOutTag>, false, TypeArr<Args...>>::printable
        constexpr auto write(Args&&... args) {
            return FormattingChain<D, ctype::c8, Formatter<ctype::c8>, false, const NoOutTag&, Formatter<NoOutTag>, false, TypeArr<Args...>>(derived(), ' ', Formatter<ctype::c8>(), NoOut, Formatter<NoOutTag>(), std::forward_as_tuple(std::forward<Args>(args)...), {});
        }
        template<class... Args> [[nodiscard]] constexpr auto writeln(Args&&... args) { return FormattingChain<D, ctype::c8, Formatter<ctype::c8>, false, ctype::c8, Formatter<ctype::c8>, false, TypeArr<Args...>>(derived(), ' ', Formatter<ctype::c8>(), '\n', Formatter<ctype::c8>(), std::forward_as_tuple(std::forward<Args>(args)...), {}); }
        template<class... Args>
            requires FormattingChain<D, ctype::c8, Formatter<ctype::c8>, false, ctype::c8, Formatter<ctype::c8>, false, TypeArr<Args...>>::printable
        constexpr auto writeln(Args&&... args) {
            return FormattingChain<D, ctype::c8, Formatter<ctype::c8>, false, ctype::c8, Formatter<ctype::c8>, false, TypeArr<Args...>>(derived(), ' ', Formatter<ctype::c8>(), '\n', Formatter<ctype::c8>(), std::forward_as_tuple(std::forward<Args>(args)...), {});
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

class MmapReader : public internal::IstreamInterface<MmapReader> {
    [[maybe_unused]] const itype::i32 fh;
    ctype::c8 *buf, *cur, *eof;
public:
    MmapReader() : fh(0) {
#if !defined(__linux__)
        buf = nullptr;
        BasicWriter<128> wt(2);
        wt.write("gsh::MmapReader / gsh::MmapReader is not available for Windows.\n");
        wt.reload();
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

}  // namespace gsh
