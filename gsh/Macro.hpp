#pragma once
#include "Formatter.hpp"  // gsh::NoOut
#include "Parser.hpp"     // gsh::Parser
#include "Range.hpp"      // gsh::Subrange
#include <cstdlib>        // std::exit
#include <ranges>         // std::ranges
#include <tuple>          // std::forward_as_tuple


#define NMIN(T) (std::numeric_limits<T>::lowest())
#define NMAX(T) (std::numeric_limits<T>::max())
// clang-format off
#define RET_WITH(...) { __VA_ARGS__; return; } []{}()
#define RETV_WITH(val, ...) { __VA_ARGS__; return val; } []{}()
#define BRK_WITH(...) { __VA_ARGS__; break; } []{}()
#define CTN_WITH(...) { __VA_ARGS__; continue; } []{}()
#define EXT_WITH(...) [&](gsh::i32 ret = 0){ __VA_ARGS__; std::exit(ret); }

#define GSH_INTERNAL_ARGS0() ()
#define GSH_INTERNAL_ARGS1(a) (auto&& a)
#define GSH_INTERNAL_ARGS2(a, b) (auto&& a, auto&& b)
#define GSH_INTERNAL_ARGS3(a, b, c) (auto&& a, auto&& b, auto&& c)
#define GSH_INTERNAL_ARGS4(a, b, c, d) (auto&& a, auto&& b, auto&& c, auto&& d)
#define GSH_INTERNAL_ARGS5(a, b, c, d, e) (auto&& a, auto&& b, auto&& c, auto&& d, auto&& e)
#define GSH_INTERNAL_ARGS6(a, b, c, d, e, f) (auto&& a, auto&& b, auto&& c, auto&& d, auto&& e, auto&& f)
#define GSH_INTERNAL_ARGS7(a, b, c, d, e, f, g) (auto&& a, auto&& b, auto&& c, auto&& d, auto&& e, auto&& f, auto&& g)
#define GSH_INTERNAL_ARGS(...) GSH_INTERNAL_SELECT8(__VA_ARGS__, GSH_INTERNAL_ARGS7, GSH_INTERNAL_ARGS6, GSH_INTERNAL_ARGS5, GSH_INTERNAL_ARGS4, GSH_INTERNAL_ARGS3, GSH_INTERNAL_ARGS2, GSH_INTERNAL_ARGS1, GSH_INTERNAL_ARGS0)(__VA_ARGS__)
#define GSH_INTERNAL_LAMBDA_BODY(...) { return (__VA_ARGS__); }
#define LAMBDA(...) [&] GSH_INTERNAL_ARGS(__VA_ARGS__) GSH_INTERNAL_LAMBDA_BODY
#define GSH_INTERNAL_ITER_CALLBACK(...) __VA_ARGS__)
#define ITERATE(...) iterate([&] GSH_INTERNAL_ARGS(__VA_ARGS__) GSH_INTERNAL_ITER_CALLBACK
// clang-format on

namespace gsh {
namespace internal {
    template<class T, class... Args> constexpr bool MatchAny(T&& value, Args&&... args) {
        return (!!(value == args) || ...);
    }
    template<class T, class Left, class Right> constexpr bool InRange(T&& value, Left&& left, Right&& right) {
        return !(left > value) && !!(value < right);
    }
}  // namespace internal
}  // namespace gsh
// clang-format off
#define MATCH(...) if (auto&& gsh_internal_match = (__VA_ARGS__); false)
#define THEN(...) else if (gsh::internal::MatchAny(gsh_internal_match, __VA_ARGS__))
#define INRANGE(...) else if (gsh::internal::InRange(gsh_internal_match, __VA_ARGS__))
// clang-format on


#define GSH_INTERNAL_REP1(n)    std::views::iota(std::decay_t<decltype(n)>(), n)
#define GSH_INTERNAL_REP2(n, m) std::views::iota(static_cast<std::common_type_t<std::decay_t<decltype(n)>, std::decay_t<decltype(m)>>>(n), static_cast<std::common_type_t<std::decay_t<decltype(n)>, std::decay_t<decltype(m)>>>(m))
#define REP(varname, ...)       for ([[maybe_unused]] const auto& varname : GSH_INTERNAL_SELECT3(__VA_ARGS__, GSH_INTERNAL_REP2, GSH_INTERNAL_REP1)(__VA_ARGS__))
#define RREP(varname, ...)      for ([[maybe_unused]] const auto& varname : GSH_INTERNAL_SELECT3(__VA_ARGS__, GSH_INTERNAL_REP2, GSH_INTERNAL_REP1)(__VA_ARGS__) | std::views::reverse)

namespace gsh {

namespace internal {

    template<class T> class InputAdapter {
        T& ref;
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
    public:
        constexpr InputAdapter(T& r) noexcept : ref(r) {}
        template<class U> constexpr operator U() const { return ref.template read<U>().val(); }
        template<class... Args> constexpr auto operator()(Args&&... args) const { return with_options<Args...>(ref, std::forward<Args>(args)...); }
        template<class... Args> constexpr auto tied_containers(u32 n) const {
            std::tuple<Args...> containers;
            std::tuple<Parser<typename Args::value_type>...> parsers;
            [&]<std::size_t... I>(std::index_sequence<I...>) {
                auto reserve = [&](auto& container) {
                    if constexpr (requires { container.reserve(n); }) container.reserve(n);
                };
                (..., reserve(std::get<I>(containers)));
            }(std::make_index_sequence<sizeof...(Args)>());
            for (u32 i = 0; i != n; ++i) {
                [&]<std::size_t... I>(std::index_sequence<I...>) {
                    auto add_value = [&]<class C>(C& container, auto& parser) {
                        if constexpr (requires { container.push_back(parser(ref)); }) container.push_back(parser(ref));
                        else if constexpr (requires { container.insert(parser(ref)); }) container.insert(parser(ref));
                        else if constexpr (requires { container.push(parser(ref)); }) container.push(parser(ref));
                        else static_assert((container, false));
                    };
                    (..., add_value(std::get<I>(containers), std::get<I>(parsers)));
                }(std::make_index_sequence<sizeof...(Args)>());
            }
            return containers;
        }
    };

    template<class T> class OutputAdapter {
        T& ref;
    public:
        constexpr OutputAdapter(T& r) : ref(r) {}
        template<class... Args> constexpr void operator()(Args&&... args) const { ref.writeln(std::forward<Args>(args)...); }
    };

}  // namespace internal

}  // namespace gsh

// clang-format off
#define DECLARE_INPUT_STREAM(name) [[maybe_unused]] const gsh::internal::InputAdapter GSH_INTERNAL_INPUT{ name }; []{}()
#define DECLARE_OUTPUT_STREAM(name) [[maybe_unused]] const gsh::internal::OutputAdapter GSH_INTERNAL_OUTPUT{ name }; []{}()
// clang-format on
#define GSH_INTERNAL_INPUT1(a)                   a = GSH_INTERNAL_INPUT
#define GSH_INTERNAL_INPUT2(a, b)                a = GSH_INTERNAL_INPUT, b = GSH_INTERNAL_INPUT
#define GSH_INTERNAL_INPUT3(a, b, c)             a = GSH_INTERNAL_INPUT, b = GSH_INTERNAL_INPUT, c = GSH_INTERNAL_INPUT
#define GSH_INTERNAL_INPUT4(a, b, c, d)          a = GSH_INTERNAL_INPUT, b = GSH_INTERNAL_INPUT, c = GSH_INTERNAL_INPUT, d = GSH_INTERNAL_INPUT
#define GSH_INTERNAL_INPUT5(a, b, c, d, e)       a = GSH_INTERNAL_INPUT, b = GSH_INTERNAL_INPUT, c = GSH_INTERNAL_INPUT, d = GSH_INTERNAL_INPUT, e = GSH_INTERNAL_INPUT
#define GSH_INTERNAL_INPUT6(a, b, c, d, e, f)    a = GSH_INTERNAL_INPUT, b = GSH_INTERNAL_INPUT, c = GSH_INTERNAL_INPUT, d = GSH_INTERNAL_INPUT, e = GSH_INTERNAL_INPUT, f = GSH_INTERNAL_INPUT
#define GSH_INTERNAL_INPUT7(a, b, c, d, e, f, g) a = GSH_INTERNAL_INPUT, b = GSH_INTERNAL_INPUT, c = GSH_INTERNAL_INPUT, d = GSH_INTERNAL_INPUT, e = GSH_INTERNAL_INPUT, f = GSH_INTERNAL_INPUT, g = GSH_INTERNAL_INPUT
#define INPUT(...)                               GSH_INTERNAL_SELECT8(__VA_ARGS__, GSH_INTERNAL_INPUT7, GSH_INTERNAL_INPUT6, GSH_INTERNAL_INPUT5, GSH_INTERNAL_INPUT4, GSH_INTERNAL_INPUT3, GSH_INTERNAL_INPUT2, GSH_INTERNAL_INPUT1)(__VA_ARGS__)
#define TIED(...)                                auto [__VA_ARGS__] = GSH_INTERNAL_INPUT.tied_containers
#define OUTPUT(...)                              GSH_INTERNAL_OUTPUT(__VA_ARGS__)
