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
    template<class M> concept IsMonoidImplemented = requires(M m) {
        typename M::value_type;
        { m(std::declval<typename M::value_type>(), std::declval<typename M::value_type>()) } -> std::same_as<typename M::value_type>;
        { m.identity() } -> std::same_as<typename M::value_type>;
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

template<class M>
    requires internal::IsMonoidImplemented<M>
class SegmentTree : public ViewInterface<SegmentTree<M>, typename M::value_type> {
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
    template<class InputIt>
        requires std::forward_iterator<InputIt>
    constexpr SegmentTree(InputIt first, InputIt last, M monoid = M()) : monoid(monoid),
                                                                         n(std::ranges::distance(first, last)) {
        sz = n > 0 ? std::bit_ceil(n) : 0;
        if (n > 0) {
            tree.assign(2 * sz, monoid.identity());
            auto it = first;
            for (size_type i = 0; i < n; ++i, ++it) tree[sz + i] = *it;
            for (size_type i = sz - 1; i >= 1; --i) tree[i] = monoid(tree[2 * i], tree[2 * i + 1]);
        }
    }
    constexpr SegmentTree(size_type n, const value_type& value, M monoid = M()) : monoid(monoid) { assign(n, value); }
    constexpr SegmentTree(std::initializer_list<value_type> init, M monoid = M()) : SegmentTree(init.begin(), init.end(), monoid) {}
    constexpr SegmentTree& operator=(std::initializer_list<value_type> il) {
        assign(il);
        return *this;
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

    constexpr void resize(size_type n) { resize(n, monoid.identity()); }
    constexpr void resize(size_type n, const value_type& c) {
        Vec<value_type> tmp;
        tmp.reserve(this->n);
        for (size_type i = 0; i < this->n; ++i) tmp.emplace_back(operator[](i));
        tmp.resize(n, c);
        assign(tmp.begin(), tmp.end());
    }

    template<class InputIt>
        requires std::forward_iterator<InputIt>
    constexpr void assign(InputIt first, InputIt last) {
        n = std::ranges::distance(first, last);
        sz = n > 0 ? std::bit_ceil(n) : 0;
        if (n > 0) {
            tree.assign(2 * sz, monoid.identity());
            auto it = first;
            for (size_type i = 0; i < n; ++i, ++it) tree[sz + i] = *it;
            for (size_type i = sz - 1; i >= 1; --i) tree[i] = monoid(tree[2 * i], tree[2 * i + 1]);
        } else {
            tree.clear();
        }
    }
    constexpr void assign(size_type n, const value_type& u) {
        this->n = n;
        sz = n > 0 ? std::bit_ceil(n) : 0;
        if (n > 0) {
            tree.assign(2 * sz, monoid.identity());
            for (size_type i = 0; i < n; ++i) tree[sz + i] = u;
            for (size_type i = sz - 1; i >= 1; --i) tree[i] = monoid(tree[2 * i], tree[2 * i + 1]);
        } else {
            tree.clear();
        }
    }
    constexpr void assign(std::initializer_list<value_type> il) { assign(il.begin(), il.end()); }

    constexpr void swap(SegmentTree& r) {
        using std::swap;
        swap(monoid, r.monoid);
        swap(n, r.n);
        swap(sz, r.sz);
        swap(tree, r.tree);
    }

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
    constexpr const value_type& get(size_type i) const { return (*this)[i]; }

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
