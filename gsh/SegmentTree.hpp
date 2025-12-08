#pragma once
#include "Exception.hpp"
#include "Functional.hpp"
#include "Numeric.hpp"
#include "Vec.hpp"
#include <bit>
#include <concepts>
#include <iterator>
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
    [[no_unique_address]] mutable Op op;
    [[no_unique_address]] mutable Id id;
public:
    using value_type = std::remove_cvref_t<std::invoke_result_t<Id>>;
    constexpr Monoid() = default;
    constexpr Monoid(Op op, Id id) : op(op), id(id) {}
    constexpr value_type operator()(const value_type& a, const value_type& b) const noexcept(noexcept(std::is_nothrow_invocable_v<Op, const value_type&, const value_type&>)) { return static_cast<value_type>(std::invoke(op, a, b)); }
    constexpr value_type identity() const noexcept(noexcept(std::is_nothrow_invocable_v<Id>)) { return static_cast<value_type>(std::invoke(id)); }
};

namespace monoids {
    template<class T> class Plus : public decltype(Monoid(gsh::Plus(), []() -> T { return static_cast<T>(0); })){};
    template<class T> class Multiplies : public decltype(Monoid(gsh::Multiplies(), []() -> T { return static_cast<T>(1); })){};
    template<class T> class Min : public decltype(Monoid([](const T& a, const T& b) { return std::min(a, b); }, []() -> T { return std::numeric_limits<T>::max(); })){};
    template<class T> class Max : public decltype(Monoid([](const T& a, const T& b) { return std::max(a, b); }, []() -> T { return std::numeric_limits<T>::min(); })){};
    template<class T> class GCD : public decltype(Monoid([](const T& a, const T& b) { return gsh::GCD(a, b); }, []() -> T { return static_cast<T>(0); })){};
    template<class T> class LCM : public decltype(Monoid([](const T& a, const T& b) { return gsh::LCM(a, b); }, []() -> T { return static_cast<T>(1); })){};
}  // namespace monoids

template<class M> class SegmentTree : public ViewInterface<SegmentTree<M>, typename M::value_type> {
    [[no_unique_address]] M monoid;
public:
    using value_type = typename M::value_type;
    using size_type = std::size_t;
    using difference_type = std::ptrdiff_t;
private:
    size_type n;
    size_type sz;
    Vec<value_type> tree;
public:
    constexpr SegmentTree() : n(0), sz(0) {}
    constexpr SegmentTree(size_type n, M monoid = M()) : monoid(monoid), n(n) {
        sz = n > 0 ? std::bit_ceil(n) : 0;
        if (n > 0) tree.assign(2 * sz, monoid.identity());
    }
    template<class InputIt> constexpr SegmentTree(InputIt first, InputIt last, M monoid = M()) : monoid(monoid), n(std::distance(first, last)) {
        sz = n > 0 ? std::bit_ceil(n) : 0;
        if (n > 0) {
            tree.assign(2 * sz, monoid.identity());
            auto it = first;
            for (size_type i = 0; i < n; ++i, ++it) tree[sz + i] = *it;
            for (size_type i = sz - 1; i >= 1; --i) tree[i] = monoid(tree[2 * i], tree[2 * i + 1]);
        }
    }

    constexpr auto begin() const { return tree.cbegin() + sz; }
    constexpr auto end() const { return tree.cbegin() + sz + n; }
    constexpr auto cbegin() const { return tree.cbegin() + sz; }
    constexpr auto cend() const { return tree.cbegin() + sz + n; }
    constexpr void clear() {
        n = 0;
        sz = 0;
        tree.clear();
    }
    constexpr bool empty() const { return n == 0; }
    constexpr size_type size() const { return n; }

    constexpr value_type prod(size_type l, size_type r) const {
#ifndef NDEBUG
        if (l > r || r > n) throw Exception("SegmentTree::prod: invalid range [", l, ", ", r, ") with size ", n);
#endif
        value_type sml = monoid.identity(), smr = monoid.identity();
        l += sz;
        r += sz;
        while (l < r) {
            if (l & 1) sml = monoid(sml, tree[l++]);
            if (r & 1) smr = monoid(tree[--r], smr);
            l >>= 1;
            r >>= 1;
        }
        return monoid(sml, smr);
    }
    constexpr value_type all_prod() const { return n > 0 ? tree[1] : monoid.identity(); }

    constexpr void set(size_type i, const value_type& x) {
#ifndef NDEBUG
        if (i >= n) throw Exception("SegmentTree::set: index ", i, " is out of range [0, ", n, ")");
#endif
        i += sz;
        tree[i] = x;
        while (i >>= 1) tree[i] = monoid(tree[2 * i], tree[2 * i + 1]);
    }
    constexpr const value_type& operator[](size_type i) const {
#ifndef NDEBUG
        if (i >= n) throw Exception("SegmentTree::operator[]: index ", i, " is out of range [0, ", n, ")");
#endif
        return tree[sz + i];
    }

    // Returns the maximum r (l <= r <= n) such that f(prod(l, r)) is true.
    // Constraint: f(monoid.identity()) must be true.
    template<class F> constexpr size_type max_right(size_type l, F f) const {
#ifndef NDEBUG
        if (l > n) throw Exception("SegmentTree::max_right: index ", l, " is out of range [0, ", n, "]");
        if (!std::invoke(f, monoid.identity())) throw Exception("SegmentTree::max_right: predicate must be true for identity");
#endif
        if (l == n) return n;
        value_type sm = monoid.identity();
        l += sz;
        do {
            while (l % 2 == 0) l >>= 1;
            if (!std::invoke(f, monoid(sm, tree[l]))) {
                while (l < sz) {
                    l = (2 * l);
                    if (std::invoke(f, monoid(sm, tree[l]))) {
                        sm = monoid(sm, tree[l]);
                        l++;
                    }
                }
                return l - sz;
            }
            sm = monoid(sm, tree[l]);
            l++;
        } while ((l & -l) != l);
        return n;
    }

    // Returns the minimum l (0 <= l <= r) such that f(prod(l, r)) is true.
    // Constraint: f(monoid.identity()) must be true.
    template<class F> constexpr size_type min_left(size_type r, F f) const {
#ifndef NDEBUG
        if (r > n) throw Exception("SegmentTree::min_left: index ", r, " is out of range [0, ", n, "]");
        if (!std::invoke(f, monoid.identity())) throw Exception("SegmentTree::min_left: predicate must be true for identity");
#endif
        if (r == 0) return 0;
        value_type sm = monoid.identity();
        r += sz;
        do {
            r--;
            while (r > 1 && (r % 2)) r >>= 1;
            if (!std::invoke(f, monoid(tree[r], sm))) {
                while (r < sz) {
                    r = (2 * r + 1);
                    if (std::invoke(f, monoid(tree[r], sm))) {
                        sm = monoid(tree[r], sm);
                        r--;
                    }
                }
                return r + 1 - sz;
            }
            sm = monoid(tree[r], sm);
        } while ((r & -r) != r);
        return 0;
    }
};

}  // namespace gsh
