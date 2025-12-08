#pragma once
#include "Functional.hpp"
#include "Numeric.hpp"
#include "Vec.hpp"
#include <concepts>
#include <limits>

namespace gsh {

namespace internal {
    template<class Op, class Id> concept IsValidMonoid = std::invocable<Id> && requires(Op op, const std::invoke_result_t<Id>& a, const std::invoke_result_t<Id>& b) {
        { op(a, b) } -> std::convertible_to<std::remove_cvref_t<std::invoke_result_t<Id>>>;
    };
}  // namespace internal

template<class Op, class Id>
    requires internal::IsValidMonoid<Op, Id>
class Monoid {
    [[no_unique_address]] Op op;
    [[no_unique_address]] Id id;
public:
    using value_type = std::remove_cvref_t<std::invoke_result_t<Id>>;
    constexpr Monoid() = default;
    constexpr Monoid(Op op, Id id) : op(op), id(id) {}
    constexpr value_type operator()(const value_type& a, const value_type& b) noexcept(noexcept(std::is_nothrow_invocable_v<Op, const value_type&, const value_type&>)) { return static_cast<value_type>(std::invoke(op, a, b)); }
    constexpr value_type identity() noexcept(noexcept(std::is_nothrow_invocable_v<Id>)) { return static_cast<value_type>(std::invoke(id)); }
};

namespace monoids {
    template<class T> class Plus : public decltype(Monoid(gsh::Plus(), []() -> T { return static_cast<T>(0); })){};
    template<class T> class Multiplies : public decltype(gsh::Multiplies(), []() -> T { return static_cast<T>(1); }){};
    template<class T> class Min : public decltype(Monoid([](const T& a, const T& b) { return std::min(a, b); }, []() -> T { return std::numeric_limits<T>::max(); })){};
    template<class T> class Max : public decltype(Monoid([](const T& a, const T& b) { return std::max(a, b); }, []() -> T { return std::numeric_limits<T>::min(); })){};
    template<class T> class GCD : public decltype(Monoid([](const T& a, const T& b) { return gsh::GCD(a, b); }, []() -> T { return static_cast<T>(0); })){};
    template<class T> class LCM : public decltype(Monoid([](const T& a, const T& b) { return gsh::LCM(a, b); }, []() -> T { return static_cast<T>(1); })){};
}  // namespace monoids

template<class M> class SegmentTree {
    [[no_unique_address]] M monoid;
    Vec<typename M::value_type> tree;
public:
    using value_type = typename M::value_type;
    constexpr SegmentTree() = default;
    constexpr SegmentTree(M monoid) : monoid(monoid) {}
    // TODO: begin, end, cbegin, cend, clear, empty, size, prod, all_prod, set, operator[]
};

}  // namespace gsh
