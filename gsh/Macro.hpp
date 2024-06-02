#pragma once
#include <iterator>           // std::begin, std::end, std::rbegin, std::rend, std::next
#include <cstdlib>            // std::exit
#include <tuple>              // std::forward_as_tuple
#include <gsh/Exception.hpp>  // gsh::Exception
#include <gsh/Option.hpp>     // gsh::Option

#define ALL(V)       std::begin(V), std::end(V)
#define RALL(V)      std::rbegin(V), std::rend(V)
#define ALLMID(V, n) std::begin(V), std::next(std::begin(V), n), std::end(V)
#define VALUE(...)   (([&]() __VA_ARGS__)())
// clang-format off
#define RET_WITH(...) { __VA_ARGS__; return; } []{}
#define RETV_WITH(val, ...) { __VA_ARGS__; return val; } []{}
#define BRK_WITH(...) { __VA_ARGS__; break; } []{}
#define CTN_WITH(...) { __VA_ARGS__; continue; } []{}
#define EXT_WITH(...) { __VA_ARGS__; std::exit(0); } []{}
#define ARGS(...) __VA_ARGS__
#define EXPR(args, ...) [&](auto&&... GSH_EXPR_ARGS){ auto [args] = std::forward_as_tuple(GSH_EXPR_ARGS...); return (__VA_ARGS__); }
// clang-format on
#define UNWRAP(var_name, ...) if (auto [var_name, GSH_UNWRAP_FLAG_##var_name] = (__VA_ARGS__); GSH_UNWRAP_FLAG_##var_name)
