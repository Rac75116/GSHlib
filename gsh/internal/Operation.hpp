#pragma once
#include <utility>      // std::move
#include <type_traits>  // std::is_convertible_v

namespace gsh {

namespace internal {
    template<class D> class ArithmeticInterface {
        constexpr D& derived() { return *reinterpret_cast<D*>(this); }
        constexpr const D& derived() const { return *reinterpret_cast<const D*>(this); }
    public:
        constexpr D operator++(int) {
            D copy = derived();
            ++derived();
            return copy;
        }
        constexpr D operator--(int) {
            D copy = derived();
            --derived();
            return copy;
        }
        constexpr D operator+() const
            requires requires(D x) { -x; }
        {
            return derived();
        }
        friend constexpr auto operator*(const D& t1, const D& t2) { return D(t1) *= t2; }
        friend constexpr auto operator/(const D& t1, const D& t2) { return D(t1) /= t2; }
        friend constexpr auto operator%(const D& t1, const D& t2) { return D(t1) %= t2; }
        friend constexpr auto operator+(const D& t1, const D& t2) { return D(t1) += t2; }
        friend constexpr auto operator-(const D& t1, const D& t2) { return D(t1) -= t2; }
        template<class T>
            requires(!std::is_convertible_v<T, D>)
        friend constexpr auto operator-(const D& t1, const T& t2) {
            return D(t1) -= t2;
        }
        template<class T> friend constexpr auto operator<<(const D& t1, const T& t2) { return D(t1) <<= t2; }
        template<class T> friend constexpr auto operator>>(const D& t1, const T& t2) { return D(t1) >>= t2; }
        friend constexpr auto operator&(const D& t1, const D& t2) { return D(t1) &= t2; }
        friend constexpr auto operator|(const D& t1, const D& t2) { return D(t1) |= t2; }
        friend constexpr auto operator^(const D& t1, const D& t2) { return D(t1) ^= t2; }
    };
}  // namespace internal

}  // namespace gsh
