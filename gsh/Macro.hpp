#pragma once
#include <cstdlib>  // std::exit
#include <tuple>    // std::forward_as_tuple
#if __has_include(<source_location>)
#include <source_location>  // std::source_location
#endif
#include <ranges>       // std::ranges
#include "TypeDef.hpp"  // gsh::itype
#include "InOut.hpp"    // gsh::BasicWriter, gsh::Formatter
#include "Range.hpp"    // gsh::Subrange

#define RANGE(V)  gsh::Subrange(std::ranges::begin(V), std::ranges::end(V));
#define RRANGE(V) gsh::Subrange(std::ranges::rbegin(V), std::ranges::rend(V));
#define NMIN(T)   (std::numeric_limits<T>::lowest())
#define NMAX(T)   (std::numeric_limits<T>::max())
// clang-format off
#define RET_WITH(...) { __VA_ARGS__; return; } []{}
#define RETV_WITH(val, ...) { __VA_ARGS__; return val; } []{}
#define BRK_WITH(...) { __VA_ARGS__; break; } []{}
#define CTN_WITH(...) { __VA_ARGS__; continue; } []{}
#define EXT_WITH(...) { __VA_ARGS__; std::exit(0); } []{}

#define GSH_INTERNAL_ARGS0() ()
#define GSH_INTERNAL_ARGS1(a) (auto&& a)
#define GSH_INTERNAL_ARGS2(a, b) (auto&& a, auto&& b)
#define GSH_INTERNAL_ARGS3(a, b, c) (auto&& a, auto&& b, auto&& c)
#define GSH_INTERNAL_ARGS4(a, b, c, d) (auto&& a, auto&& b, auto&& c, auto&& d)
#define GSH_INTERNAL_ARGS5(a, b, c, d, e) (auto&& a, auto&& b, auto&& c, auto&& d, auto&& e)
#define GSH_INTERNAL_ARGS6(a, b, c, d, e, f) (auto&& a, auto&& b, auto&& c, auto&& d, auto&& e, auto&& f)
#define GSH_INTERNAL_ARGS7(a, b, c, d, e, f, g) (auto&& a, auto&& b, auto&& c, auto&& d, auto&& e, auto&& f, auto&& g)
#define GSH_INTERNAL_ARGS(...) GSH_INTERNAL_SELECT8(__VA_ARGS__, GSH_INTERNAL_ARGS7, GSH_INTERNAL_ARGS6, GSH_INTERNAL_ARGS5, GSH_INTERNAL_ARGS4, GSH_INTERNAL_ARGS3, GSH_INTERNAL_ARGS2, GSH_INTERNAL_ARGS1, GSH_INTERNAL_ARGS0)(__VA_ARGS__)
#define GSH_INTERNAL_LAMBDA1(...) [&]() { return (__VA_ARGS__); }
#define GSH_INTERNAL_LAMBDA2(args, ...) [&] GSH_INTERNAL_ARGS##args { return (__VA_ARGS__); }
#define LAMBDA(...) GSH_INTERNAL_SELECT3(__VA_ARGS__, GSH_INTERNAL_LAMBDA2, GSH_INTERNAL_LAMBDA1)(__VA_ARGS__)
// clang-format on


#define GSH_INTERNAL_REP1(n)    std::views::iota(decltype(n)(), n)
#define GSH_INTERNAL_REP2(n, m) std::views::iota(static_cast<std::common_type_t<decltype(n), decltype(m)>>(n), static_cast<std::common_type_t<decltype(n), decltype(m)>>(m))
#define REP(varname, ...)       for ([[maybe_unused]] const auto& varname : GSH_INTERNAL_SELECT3(__VA_ARGS__, GSH_INTERNAL_REP2, GSH_INTERNAL_REP1)(__VA_ARGS__))
#define RREP(varname, ...)      for ([[maybe_unused]] const auto& varname : GSH_INTERNAL_SELECT3(__VA_ARGS__, GSH_INTERNAL_REP2, GSH_INTERNAL_REP1)(__VA_ARGS__) | std::ranges::reverse)

namespace gsh {
namespace internal {
#ifdef __cpp_lib_source_location
    template<itype::u32> GSH_INTERNAL_INLINE constexpr void Assert(const bool cond, const ctype::c8* message, std::source_location loc = std::source_location::current()) {
        if (!cond) [[unlikely]] {
            if (std::is_constant_evaluated()) {
                throw 0;
            } else {
                BasicWriter<2048> w(2);
#if !defined(_MSC_VER) && defined(GSH_DIAGNOSTICS_COLOR)
                w.write("\e[2m[from gsh::internal::Assert] \e[0mDuring the execution of \e[1m\e[3m'");
                w.write(loc.function_name());
                w.write("'\e[0m\n");
                w.write(loc.file_name());
                w.write(':');
                w.write(loc.line());
                w.write(':');
                w.write(loc.column());
                w.write(':');
                w.write(" \e[31mAssertion Failed:\e[0m \e[1m\e[3m'");
                w.write(message);
                w.write("'\e[0m\n");
                w.reload();
#else
                w.write("[from gsh::internal::Assert] During the execution of '");
                w.writeln(loc.function_name());
                w.write(loc.file_name());
                w.write(':');
                w.write(loc.line());
                w.write(':');
                w.write(loc.column());
                w.write(':');
                w.write(" Assertion Failed: '");
                w.write(message);
                w.writeln("'");
                w.reload();
#endif
                std::exit(1);
            }
        }
    }
#else
    template<itype::u32> GSH_INTERNAL_INLINE constexpr void Assert(const bool cond, const ctype::c8* message) {
        if (!cond) [[unlikely]] {
            if (std::is_constant_evaluated()) {
                throw 0;
            } else {
                BasicWriter<2048> w(2);
#if !defined(_MSC_VER) && defined(GSH_DIAGNOSTICS_COLOR)
                w.write("\e[2m[from gsh::internal::Assert]\e[0m ");
                w.write("\e[31mAssertion Failed:\e[0m \e[1m\e[3m'");
                w.write(message);
                w.write("'\e[0m\n");
                w.reload();
#else
                w.write("[from gsh::internal::Assert] ");
                w.write(" Assertion Failed: '");
                w.write(message);
                w.writeln("'");
                w.reload();
#endif
                std::exit(1);
            }
        }
    }
#endif
}  // namespace internal
}  // namespace gsh
#define GSH_INTERNAL_ASSERT1(cond)          gsh::internal::Assert<0>(cond, #cond)
#define GSH_INTERNAL_ASSERT2(cond, message) gsh::internal::Assert<0>(cond, message)
#define ASSERT(...)                         GSH_INTERNAL_SELECT3(__VA_ARGS__, GSH_INTERNAL_ASSERT2, GSH_INTERNAL_ASSERT1)(__VA_ARGS__)

namespace gsh {
namespace internal {
    template<class T> class InputAdapter {
        template<class... Args> class with_options {
            friend class InputAdapter;
            T& ref;
            std::tuple<Args...> args;
            constexpr with_options(T& r, Args&&... a) : ref(r), args(std::forward<Args>(a)...) {}
        public:
            template<class U> constexpr operator U() const {
                return [&]<std::size_t... I>(std::index_sequence<I...>) {
                    return ref.template read<U>().option(std::get<I>(args)...).val();
                }(std::make_index_sequence<sizeof...(Args)>{});
            }
        };
        T& ref;
    public:
        constexpr InputAdapter(T& r) noexcept : ref(r) {}
        template<class U> constexpr operator U() const { return ref.template read<U>().val(); }
        template<class... Args> constexpr auto operator()(Args&&... args) const { return with_options<Args...>(ref, std::forward<Args>(args)...); }
    };
}  // namespace internal
}  // namespace gsh
// clang-format off
#define DEF_INPUT_STREAM(name)                   [[maybe_unused]] gsh::internal::InputAdapter GSH_INTERNAL_INPUT{ name };
#define GSH_INTERNAL_INPUT1(a)                   a = GSH_INTERNAL_INPUT
#define GSH_INTERNAL_INPUT2(a, b)                a = GSH_INTERNAL_INPUT, b = GSH_INTERNAL_INPUT
#define GSH_INTERNAL_INPUT3(a, b, c)             a = GSH_INTERNAL_INPUT, b = GSH_INTERNAL_INPUT, c = GSH_INTERNAL_INPUT
#define GSH_INTERNAL_INPUT4(a, b, c, d)          a = GSH_INTERNAL_INPUT, b = GSH_INTERNAL_INPUT, c = GSH_INTERNAL_INPUT, d = GSH_INTERNAL_INPUT
#define GSH_INTERNAL_INPUT5(a, b, c, d, e)       a = GSH_INTERNAL_INPUT, b = GSH_INTERNAL_INPUT, c = GSH_INTERNAL_INPUT, d = GSH_INTERNAL_INPUT, e = GSH_INTERNAL_INPUT
#define GSH_INTERNAL_INPUT6(a, b, c, d, e, f)    a = GSH_INTERNAL_INPUT, b = GSH_INTERNAL_INPUT, c = GSH_INTERNAL_INPUT, d = GSH_INTERNAL_INPUT, e = GSH_INTERNAL_INPUT, f = GSH_INTERNAL_INPUT
#define GSH_INTERNAL_INPUT7(a, b, c, d, e, f, g) a = GSH_INTERNAL_INPUT, b = GSH_INTERNAL_INPUT, c = GSH_INTERNAL_INPUT, d = GSH_INTERNAL_INPUT, e = GSH_INTERNAL_INPUT, f = GSH_INTERNAL_INPUT, g = GSH_INTERNAL_INPUT
#define INPUT(...)                               GSH_INTERNAL_SELECT8(__VA_ARGS__, GSH_INTERNAL_INPUT7, GSH_INTERNAL_INPUT6, GSH_INTERNAL_INPUT5, GSH_INTERNAL_INPUT4, GSH_INTERNAL_INPUT3, GSH_INTERNAL_INPUT2, GSH_INTERNAL_INPUT1)(__VA_ARGS__)
// clang-format on
