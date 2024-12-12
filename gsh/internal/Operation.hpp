#pragma once
#include <utility>      // std::move
#include <type_traits>  // std::is_convertible_v

namespace gsh {

namespace internal {
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
        constexpr D operator+() const noexcept(std::is_nothrow_copy_constructible_v<D>)
            requires requires(D x) { -x; }
        {
            return derived();
        }
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
}  // namespace internal

}  // namespace gsh
