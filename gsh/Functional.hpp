#pragma once
#include "Int128.hpp"
#include "TypeDef.hpp"
#include "internal/UtilMacro.hpp"
#include <array>
#include <bit>
#include <concepts>
#include <cstddef>
#include <cstring>
#include <functional>
#include <iterator>
#include <type_traits>
#include <typeindex>
#include <utility>
#include <variant>
namespace gsh {
namespace internal {
template<typename T, typename U> concept LessPtrCmp = requires(T&& t, U&& u) {
  { t < u } -> std::same_as<bool>;
} && std::convertible_to<T, const volatile void*> && std::convertible_to<U, const volatile void*> && (!requires(T&& t, U&& u) { operator<(std::forward<T>(t), std::forward<U>(u)); } && !requires(T&& t, U&& u) { std::forward<T>(t).operator<(std::forward<U>(u)); });
} // namespace internal
class Less {
public:
  template<class T, class U> requires std::totally_ordered_with<T, U> GSH_INTERNAL_INLINE constexpr bool operator()(T&& t, U&& u) const noexcept(noexcept(std::declval<T>() < std::declval<U>())) {
    if constexpr(internal::LessPtrCmp<T, U>) {
      if(std::is_constant_evaluated()) return t < u;
      auto x = reinterpret_cast<u64>(static_cast<const volatile void*>(std::forward<T>(t)));
      auto y = reinterpret_cast<u64>(static_cast<const volatile void*>(std::forward<U>(u)));
      return x < y;
    } else return std::forward<T>(t) < std::forward<U>(u);
  }
  using is_transparent = void;
};
class Greater {
public:
  template<class T, class U> requires std::totally_ordered_with<T, U> GSH_INTERNAL_INLINE constexpr bool operator()(T&& t, U&& u) const noexcept(noexcept(std::declval<U>() < std::declval<T>())) {
    if constexpr(internal::LessPtrCmp<U, T>) {
      if(std::is_constant_evaluated()) return u < t;
      auto x = reinterpret_cast<u64>(static_cast<const volatile void*>(std::forward<T>(t)));
      auto y = reinterpret_cast<u64>(static_cast<const volatile void*>(std::forward<U>(u)));
      return y < x;
    } else return std::forward<U>(u) < std::forward<T>(t);
  }
  using is_transparent = void;
};
class EqualTo {
public:
  template<class T, class U> requires std::equality_comparable_with<T, U> GSH_INTERNAL_INLINE constexpr bool operator()(T&& t, U&& u) const noexcept(noexcept(std::declval<T>() == std::declval<U>())) { return std::forward<T>(t) == std::forward<U>(u); }
  using is_transparent = void;
};
class Identity {
public:
  template<class T> [[nodiscard]] GSH_INTERNAL_INLINE constexpr T&& operator()(T&& t) const noexcept { return std::forward<T>(t); }
  using is_transparent = void;
};
template<class F> class SwapArgs : public F {
public:
  constexpr SwapArgs() noexcept(std::is_nothrow_default_constructible_v<F>) : F() {}
  constexpr SwapArgs(const F& f) noexcept(std::is_nothrow_copy_constructible_v<F>) : F(f) {}
  constexpr SwapArgs(F&& f) noexcept(std::is_nothrow_move_constructible_v<F>) : F(std::move(f)) {}
  constexpr SwapArgs& operator=(const F& f) noexcept(std::is_nothrow_copy_assignable_v<F>) {
    F::operator=(f);
    return *this;
  }
  constexpr SwapArgs& operator=(F&& f) noexcept(std::is_nothrow_move_assignable_v<F>) {
    F::operator=(std::move(f));
    return *this;
  }
  constexpr SwapArgs& operator=(const SwapArgs&) noexcept(std::is_nothrow_copy_assignable_v<F>) = default;
  constexpr SwapArgs& operator=(SwapArgs&&) noexcept(std::is_nothrow_move_assignable_v<F>) = default;
  template<class T, class U> GSH_INTERNAL_INLINE constexpr decltype(auto) operator()(T&& x, U&& y) noexcept(noexcept(F::operator()(std::declval<U>(), std::declval<T>()))) { return F::operator()(std::forward<U>(y), std::forward<T>(x)); }
  template<class T, class U> GSH_INTERNAL_INLINE constexpr decltype(auto) operator()(T&& x, U&& y) const noexcept(noexcept(F::operator()(std::declval<U>(), std::declval<T>()))) { return F::operator()(std::forward<U>(y), std::forward<T>(x)); }
};
template<class F, class... G> class BindFront {
  [[no_unique_address]] F func;
  [[no_unique_address]] BindFront<G...> bind;
public:
  constexpr BindFront() noexcept(std::is_nothrow_default_constructible_v<F> && noexcept(BindFront<G...>())) : func(), bind() {}
  template<class Arg, class... Args> requires (sizeof...(Args) == sizeof...(G)) constexpr BindFront(Arg&& arg, Args&&... args) noexcept(std::is_nothrow_constructible_v<F, Arg> && noexcept(BindFront<G...>(std::forward<Args>(args)...))) : func(std::forward<Arg>(arg)), bind(std::forward<Args>(args)...) {}
  template<class... Args> constexpr decltype(auto) operator()(Args&&... args) & noexcept(std::is_nothrow_invocable_v<F, Args...>) { return std::invoke(bind, std::invoke(func, std::forward<Args>(args)...)); }
  template<class... Args> constexpr decltype(auto) operator()(Args&&... args) && noexcept(std::is_nothrow_invocable_v<F, Args...>) { return std::invoke(std::move(bind), std::invoke(std::move(func), std::forward<Args>(args)...)); }
  template<class... Args> constexpr decltype(auto) operator()(Args&&... args) const& noexcept(std::is_nothrow_invocable_v<F, Args...>) { return std::invoke(bind, std::invoke(func, std::forward<Args>(args)...)); }
  template<class... Args> constexpr decltype(auto) operator()(Args&&... args) const&& noexcept(std::is_nothrow_invocable_v<F, Args...>) { return std::invoke(std::move(bind), std::invoke(std::move(func), std::forward<Args>(args)...)); }
};
template<class F> class BindFront<F> : public F {
public:
  constexpr BindFront() noexcept(std::is_nothrow_default_constructible_v<F>) : F() {}
  template<class... Args> constexpr BindFront(Args&&... args) noexcept(std::is_nothrow_constructible_v<F, Args...>) : F(std::forward<Args>(args)...) {}
};
template<class T> class CustomizedHash;
namespace internal {
GSH_INTERNAL_INLINE constexpr u64 HashMix(u64 x) {
  constexpr u64 m = 0xe9846af9b1a615d;
  x ^= x >> 32;
  x *= m;
  x ^= x >> 32;
  x *= m;
  x ^= x >> 28;
  return x;
}
GSH_INTERNAL_INLINE constexpr u64 Mulx(u64 x, u64 y) {
  u128 r = static_cast<u128>(x) * y;
  return static_cast<u64>(r) ^ static_cast<u64>(r >> 64);
}
template<class T> concept IsCharType = std::same_as<T, char> || std::same_as<T, signed char> || std::same_as<T, unsigned char> || std::same_as<T, char8_t> || std::same_as<T, std::byte>;
template<std::random_access_iterator It> requires IsCharType<typename std::iterator_traits<It>::value_type> GSH_INTERNAL_INLINE constexpr u64 HashRangeBytes(It first, It last) {
  const auto* p = std::to_address(first);
  const u64 n = last - first;
  constexpr u64 q = 0x9e3779b97f4a7c15;
  constexpr u64 k = 0xdf442d22ce4859b9;
  u64 w = Mulx(0ull + q, k);
  u64 h = w ^ n;
  u64 remaining = n;
  while(remaining >= 8) {
    u64 v1;
    if(std::is_constant_evaluated()) {
      v1 = 0;
      for(int i = 0; i < 8; ++i) v1 |= static_cast<u64>(static_cast<unsigned char>(p[i])) << (i * 8);
    } else {
      std::memcpy(&v1, p, sizeof(v1));
    }
    w += q;
    h ^= Mulx(v1 + w, k);
    p += 8;
    remaining -= 8;
  }
  u64 v1 = 0;
  if(remaining > 0) {
    if(std::is_constant_evaluated()) {
      for(u64 i = 0; i < remaining; ++i) v1 |= static_cast<u64>(static_cast<unsigned char>(p[i])) << (i * 8);
    } else {
      std::memcpy(&v1, p, remaining);
    }
  }
  w += q;
  h ^= Mulx(v1 + w, k);
  return Mulx(h + w, k);
}
template<class T> concept CustomizedHashCallable = requires(T x) {
  { CustomizedHash<T>{}(x) } -> std::integral;
};
} // namespace internal
// Copyright 2022 Peter Dimov
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
class Hash {
public:
  template<class T> GSH_INTERNAL_INLINE constexpr u64 operator()(const T& v) const {
    using Type = std::remove_cvref_t<T>;
    constexpr bool tuple_like = requires {
      std::get<0>(v);
      std::tuple_size_v<Type>;
    };
    constexpr bool optional_like = requires {
      v.has_value();
      *v;
    };
    constexpr bool variant_like = requires {
      v.index();
      std::visit([](auto&&) {}, v);
    };
    if constexpr(internal::CustomizedHashCallable<Type>) {
      return static_cast<u64>(CustomizedHash<Type>{}(v));
    } else if constexpr(std::is_same_v<Type, std::nullptr_t>) {
      return (*this)(static_cast<void*>(nullptr));
    } else if constexpr(std::is_pointer_v<Type>) {
      auto x = reinterpret_cast<std::uintptr_t>(v);
      return (*this)(x + (x >> 3));
    } else if constexpr(std::is_enum_v<Type>) {
      return (*this)(static_cast<std::underlying_type_t<Type>>(v));
    } else if constexpr(std::integral<Type>) {
      if constexpr(sizeof(Type) <= sizeof(u64)) {
        return internal::HashMix(static_cast<u64>(v));
      } else {
        u128 val = static_cast<u128>(v);
        u64 seed = (*this)(static_cast<u64>(val >> 64));
        seed ^= (*this)(static_cast<u64>(val)) + 0x9e3779b97f4a7c15 + (seed << 6) + (seed >> 2);
        return seed;
      }
    } else if constexpr(std::floating_point<Type>) {
      if constexpr(sizeof(Type) == 4) return (*this)(std::bit_cast<u32>(v));
      else if constexpr(sizeof(Type) == 8) return (*this)(std::bit_cast<u64>(v));
      else {
        u128 bits{};
        if(std::is_constant_evaluated()) {
          auto bytes = std::bit_cast<std::array<c8, sizeof(Type)>>(v);
          for(size_t i = 0; i < sizeof(Type); ++i) { reinterpret_cast<c8*>(&bits)[i] = bytes[i]; }
        } else {
          std::memcpy(&bits, &v, sizeof(v));
        }
        return (*this)(bits);
      }
    } else if constexpr(std::ranges::range<T>) {
      constexpr bool has_ptr = requires {
        v.data();
        v.size();
      };
      if constexpr(has_ptr && internal::IsCharType<typename std::iterator_traits<decltype(v.begin())>::value_type>) {
        return internal::HashRangeBytes(v.data(), v.data() + v.size());
      } else {
        u64 seed = 0;
        for(const auto& elem : v) { seed ^= (*this)(elem) + 0x9e3779b97f4a7c15 + (seed << 6) + (seed >> 2); }
        return seed;
      }
    } else if constexpr(tuple_like) {
      u64 seed = 0;
      [&]<std::size_t... I>(std::index_sequence<I...>) { ((seed ^= (*this)(std::get<I>(v)) + 0x9e3779b97f4a7c15 + (seed << 6) + (seed >> 2)), ...); }(std::make_index_sequence<std::tuple_size_v<Type>>{});
      return seed;
    } else if constexpr(optional_like) {
      if(v.has_value()) { return (*this)(*v); }
      return 0;
    } else if constexpr(variant_like) {
      u64 seed = (*this)(v.index());
      std::visit([&](auto&& val) { seed ^= (*this)(val) + 0x9e3779b97f4a7c15 + (seed << 6) + (seed >> 2); }, v);
      return seed;
    } else if constexpr(std::is_standard_layout_v<Type> && std::is_trivial_v<Type>) {
      return internal::HashRangeBytes(reinterpret_cast<const c8*>(&v), reinterpret_cast<const c8*>(&v) + sizeof(Type));
    } else {
      static_assert(sizeof(T), "gsh::Hash cannot hash this type. Please provide a gsh::CustomizedHash specialization.");
      return 0;
    }
  }
  using is_transparent = void;
  using is_avalanching = void;
};
class Plus {
public:
  template<class T, class U> constexpr decltype(auto) operator()(T&& t, U&& u) const noexcept(noexcept(std::forward<T>(t) + std::forward<U>(u))) { return std::forward<T>(t) + std::forward<U>(u); }
  using is_transparent = void;
};
class Minus {
public:
  template<class T, class U> constexpr decltype(auto) operator()(T&& t, U&& u) const noexcept(noexcept(std::forward<T>(t) - std::forward<U>(u))) { return std::forward<T>(t) - std::forward<U>(u); }
  using is_transparent = void;
};
class Multiplies {
public:
  template<class T, class U> constexpr decltype(auto) operator()(T&& t, U&& u) const noexcept(noexcept(std::forward<T>(t) * std::forward<U>(u))) { return std::forward<T>(t) * std::forward<U>(u); }
  using is_transparent = void;
};
class Divides {
public:
  template<class T, class U> constexpr decltype(auto) operator()(T&& t, U&& u) const noexcept(noexcept(std::forward<T>(t) / std::forward<U>(u))) { return std::forward<T>(t) / std::forward<U>(u); }
  using is_transparent = void;
};
class Negate {
public:
  template<class T> constexpr decltype(auto) operator()(T&& t) const noexcept(noexcept(-std::forward<T>(t))) { return -std::forward<T>(t); }
  using is_transparent = void;
};
class True {
public:
  template<class... Args> constexpr bool operator()(Args&&...) noexcept { return true; }
  using is_transparent = void;
};
class False {
public:
  template<class... Args> constexpr bool operator()(Args&&...) noexcept { return false; }
  using is_transparent = void;
};
} // namespace gsh
