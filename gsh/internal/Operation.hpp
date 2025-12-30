#pragma once
#include "../TypeDef.hpp"
#include <type_traits>
#include <utility>
namespace gsh { namespace internal {
template<class D> class ArithmeticInterface {
  constexpr D& derived() { return *static_cast<D*>(this); }
  constexpr const D& derived() const { return *static_cast<const D*>(this); }
public:
  constexpr D operator++(int) noexcept(std::is_nothrow_copy_constructible_v<D> && noexcept(++derived())) {
    D copy = derived();
    ++derived();
    return copy;
  }
  constexpr D operator--(int) noexcept(std::is_nothrow_copy_constructible_v<D> && noexcept(--derived())) {
    D copy = derived();
    --derived();
    return copy;
  }
  constexpr D operator+() const noexcept(std::is_nothrow_copy_constructible_v<D>) requires requires(D x) { -x; } { return derived(); }
  constexpr bool operator!() const noexcept(noexcept(static_cast<bool>(derived()))) { return !static_cast<bool>(derived()); }
  friend constexpr auto operator+(const D& t1, const D& t2) noexcept(noexcept(D(t1) += t2)) { return D(t1) += t2; }
  friend constexpr auto operator-(const D& t1, const D& t2) noexcept(noexcept(D(t1) -= t2)) { return D(t1) -= t2; }
  friend constexpr auto operator*(const D& t1, const D& t2) noexcept(noexcept(D(t1) *= t2)) { return D(t1) *= t2; }
  friend constexpr auto operator/(const D& t1, const D& t2) noexcept(noexcept(D(t1) /= t2)) { return D(t1) /= t2; }
  friend constexpr auto operator%(const D& t1, const D& t2) noexcept(noexcept(D(t1) %= t2)) { return D(t1) %= t2; }
  friend constexpr auto operator&(const D& t1, const D& t2) noexcept(noexcept(D(t1) &= t2)) { return D(t1) &= t2; }
  friend constexpr auto operator|(const D& t1, const D& t2) noexcept(noexcept(D(t1) |= t2)) { return D(t1) |= t2; }
  friend constexpr auto operator^(const D& t1, const D& t2) noexcept(noexcept(D(t1) ^= t2)) { return D(t1) ^= t2; }
  template<class T> friend constexpr auto operator<<(const D& t1, const T& t2) noexcept(noexcept(D(t1) <<= t2)) { return D(t1) <<= t2; }
  template<class T> friend constexpr auto operator>>(const D& t1, const T& t2) noexcept(noexcept(D(t1) >>= t2)) { return D(t1) >>= t2; }
};
template<class D> class IteratorInterface {
  constexpr D& derived() { return *static_cast<D*>(this); }
  constexpr const D& derived() const { return *static_cast<const D*>(this); }
public:
  using size_type = u32;
  using difference_type = i32;
  constexpr D operator++(int) noexcept(std::is_nothrow_copy_constructible_v<D> && noexcept(++derived())) {
    D copy = derived();
    ++derived();
    return copy;
  }
  constexpr D operator--(int) noexcept(std::is_nothrow_copy_constructible_v<D> && noexcept(--derived())) {
    D copy = derived();
    --derived();
    return copy;
  }
  constexpr auto operator->() noexcept(noexcept(&*derived())) { return &*derived(); }
  constexpr auto operator->() const noexcept(noexcept(&*derived())) { return &*derived(); }
  template<class T> friend constexpr D operator+(const D& a, T&& n) noexcept(noexcept(D(a) += std::forward<T>(n))) { return D(a) += std::forward<T>(n); }
  template<class T> friend constexpr D operator-(const D& a, T&& n) noexcept(noexcept(D(a) -= std::forward<T>(n))) { return D(a) -= std::forward<T>(n); }
};
} // namespace internal
} // namespace gsh
