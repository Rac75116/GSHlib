#pragma once
#include <type_traits>  // std::is_convertible_v

namespace gsh {

namespace internal {
    template<class D> class ArithmeticInterface {
        constexpr D& get_ref() { return *reinterpret_cast<D*>(this); }
        constexpr const D& get_ref() const { return *reinterpret_cast<const D*>(this); }
    public:
        constexpr D operator++(int) {
            D copy = get_ref();
            ++get_ref();
            return copy;
        }
        constexpr D operator--(int) {
            D copy = get_ref();
            --get_ref();
            return copy;
        }
        constexpr D operator+() const { return get_ref(); }
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