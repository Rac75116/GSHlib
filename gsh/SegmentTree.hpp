#pragma once
#include "Algorithm.hpp"
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
template<class Op, class Id> concept IsValidSegmentSpec = std::invocable<Id> && requires(Op op, const std::invoke_result_t<Id>& a, const std::invoke_result_t<Id>& b) {
  { op(a, b) } -> std::convertible_to<std::remove_cvref_t<std::invoke_result_t<Id>>>;
};
template<class Spec> concept IsSegmentSpecImplemented = requires(Spec spec) {
  typename Spec::value_type;
  { spec.op(std::declval<typename Spec::value_type>(), std::declval<typename Spec::value_type>()) } -> std::same_as<typename Spec::value_type>;
  { spec.e() } -> std::same_as<typename Spec::value_type>;
};
template<class T, class Op, class Id> requires IsValidSegmentSpec<Op, Id> class DefaultSegmentSpec {
  [[no_unique_address]] mutable Op op_func;
  [[no_unique_address]] mutable Id id_func;
public:
  using value_type = T;
  constexpr DefaultSegmentSpec() = default;
  constexpr DefaultSegmentSpec(const Op& op, const Id& id) : op_func(op), id_func(id) {}
  constexpr value_type op(const value_type& a, const value_type& b) const noexcept(noexcept(std::is_nothrow_invocable_v<Op, const value_type&, const value_type&>)) { return static_cast<value_type>(std::invoke(op_func, a, b)); }
  constexpr value_type e() const noexcept(noexcept(std::is_nothrow_invocable_v<Id>)) { return static_cast<value_type>(std::invoke(id_func)); }
};
}
template<class T, class Op, class Id> constexpr internal::DefaultSegmentSpec<T, Op, Id> MakeSegmentSpec(const Op& op = Op(), const Id& id = Id()) { return {op, id}; }
namespace segment_specs {
template<class T> class RangePlus : public decltype(MakeSegmentSpec<T>(Plus, []() -> T { return static_cast<T>(0); })) {};
template<class T> class RangeMultiplies : public decltype(MakeSegmentSpec<T>(Multiplies, []() -> T { return static_cast<T>(1); })) {};
template<class T> class RangeOr : public decltype(MakeSegmentSpec<T>(Or, []() -> T { return static_cast<T>(0); })) {};
template<class T> class RangeAnd : public decltype(MakeSegmentSpec<T>(And, []() -> T { return ~static_cast<T>(0); })) {};
template<class T> class RangeXor : public decltype(MakeSegmentSpec<T>(Xor, []() -> T { return static_cast<T>(0); })) {};
template<class T> class RangeMin : public decltype(MakeSegmentSpec<T>(Min, []() -> T { return std::numeric_limits<T>::max(); })) {};
template<class T> class RangeMax : public decltype(MakeSegmentSpec<T>(Max, []() -> T { return std::numeric_limits<T>::min(); })) {};
template<class T> class RangeGCD : public decltype(MakeSegmentSpec<T>(GCD, []() -> T { return static_cast<T>(0); })) {};
template<class T> class RangeLCM : public decltype(MakeSegmentSpec<T>(LCM, []() -> T { return static_cast<T>(1); })) {};
template<class T> class RangeComposite : public decltype(MakeSegmentSpec<T>([](const std::pair<T, T>& a, const std::pair<T, T>& b) { return std::pair{a.first * b.first, a.second * b.first + b.second}; }, []() { return std::pair{static_cast<T>(1), static_cast<T>(0)}; })) {};
}
template<class Spec> requires internal::IsSegmentSpecImplemented<Spec> class SegmentTree : public ViewInterface<SegmentTree<Spec>, typename Spec::value_type> {
  [[no_unique_address]] Spec spec;
public:
  using value_type = typename Spec::value_type;
  using size_type = u32;
  using difference_type = i32;
private:
  size_type n;
  size_type sz;
  Vec<value_type> tree;
public:
  constexpr SegmentTree() : n(0), sz(0) {}
  constexpr SegmentTree(Spec spec) : spec(spec), n(0), sz(0) {}
  constexpr SegmentTree(size_type n, Spec spec = Spec()) : spec(spec), n(n) {
    sz = n > 0 ? std::bit_ceil(n) : 0;
    if(n > 0) tree.assign(2 * sz, spec.e());
  }
  template<class InputIt> requires std::forward_iterator<InputIt> constexpr SegmentTree(InputIt first, InputIt last, Spec spec = Spec()) : spec(spec), n(std::ranges::distance(first, last)) {
    sz = n > 0 ? std::bit_ceil(n) : 0;
    if(n > 0) {
      tree.assign(2 * sz, spec.e());
      auto it = first;
      for(size_type i = 0; i < n; ++i, ++it) tree[sz + i] = *it;
      for(size_type i = sz - 1; i >= 1; --i) tree[i] = spec.op(tree[2 * i], tree[2 * i + 1]);
    }
  }
  constexpr SegmentTree(size_type n, const value_type& value, Spec spec = Spec()) : spec(spec) { assign(n, value); }
  constexpr SegmentTree(std::initializer_list<value_type> init, Spec spec = Spec()) : SegmentTree(init.begin(), init.end(), spec) {}
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
  constexpr void resize(size_type n) { resize(n, spec.e()); }
  constexpr void resize(size_type n, const value_type& c) {
    Vec<value_type> tmp;
    tmp.reserve(this->n);
    for(size_type i = 0; i < this->n; ++i) tmp.emplace_back(operator[](i));
    tmp.resize(n, c);
    assign(tmp.begin(), tmp.end());
  }
  template<class InputIt> requires std::forward_iterator<InputIt> constexpr void assign(InputIt first, InputIt last) {
    n = std::ranges::distance(first, last);
    sz = n > 0 ? std::bit_ceil(n) : 0;
    if(n > 0) {
      tree.assign(2 * sz, spec.e());
      auto it = first;
      for(size_type i = 0; i < n; ++i, ++it) tree[sz + i] = *it;
      for(size_type i = sz - 1; i >= 1; --i) tree[i] = spec.op(tree[2 * i], tree[2 * i + 1]);
    } else {
      tree.clear();
    }
  }
  constexpr void assign(size_type n, const value_type& u) {
    this->n = n;
    sz = n > 0 ? std::bit_ceil(n) : 0;
    if(n > 0) {
      tree.assign(2 * sz, spec.e());
      for(size_type i = 0; i < n; ++i) tree[sz + i] = u;
      for(size_type i = sz - 1; i >= 1; --i) tree[i] = spec.op(tree[2 * i], tree[2 * i + 1]);
    } else {
      tree.clear();
    }
  }
  constexpr void assign(std::initializer_list<value_type> il) { assign(il.begin(), il.end()); }
  constexpr void swap(SegmentTree& r) {
    using std::swap;
    swap(spec, r.spec);
    swap(n, r.n);
    swap(sz, r.sz);
    swap(tree, r.tree);
  }
  constexpr value_type prod(size_type l, size_type r) const {
#ifndef NDEBUG
    if(l > r || r > n) throw Exception("SegmentTree::prod: invalid range [", l, ", ", r, ") with size ", n);
#endif
    value_type sml = spec.e(), smr = spec.e();
    l += sz;
    r += sz;
    while(l < r) {
      if(l & 1) sml = spec.op(sml, tree[l++]);
      if(r & 1) smr = spec.op(tree[--r], smr);
      l >>= 1;
      r >>= 1;
    }
    return spec.op(sml, smr);
  }
  constexpr value_type all_prod() const { return n > 0 ? tree[1] : spec.e(); }
  constexpr void set(size_type i, const value_type& x) {
#ifndef NDEBUG
    if(i >= n) throw Exception("SegmentTree::set: index ", i, " is out of range [0, ", n, ")");
#endif
    i += sz;
    tree[i] = x;
    while(i >>= 1) tree[i] = spec.op(tree[2 * i], tree[2 * i + 1]);
  }
  constexpr const value_type& operator[](size_type i) const {
#ifndef NDEBUG
    if(i >= n) throw Exception("SegmentTree::operator[]: index ", i, " is out of range [0, ", n, ")");
#endif
    return tree[sz + i];
  }
  constexpr const value_type& get(size_type i) const { return (*this)[i]; }
  // Returns the maximum r (l <= r <= n) such that f(prod(l, r)) is true.
  // Constraint: f(spec.e()) must be true.
  template<class F> constexpr size_type max_right(size_type l, F f) const {
#ifndef NDEBUG
    if(l > n) throw Exception("SegmentTree::max_right: index ", l, " is out of range [0, ", n, "]");
    if(!std::invoke(f, spec.e())) throw Exception("SegmentTree::max_right: predicate must be true for identity");
#endif
    if(l == n) return n;
    value_type sm = spec.e();
    l += sz;
    do {
      while(l % 2 == 0) l >>= 1;
      if(!std::invoke(f, spec.op(sm, tree[l]))) {
        while(l < sz) {
          l = (2 * l);
          if(std::invoke(f, spec.op(sm, tree[l]))) {
            sm = spec.op(sm, tree[l]);
            l++;
          }
        }
        return l - sz;
      }
      sm = spec.op(sm, tree[l]);
      l++;
    } while((l & -l) != l);
    return n;
  }
  // Returns the minimum l (0 <= l <= r) such that f(prod(l, r)) is true.
  // Constraint: f(spec.e()) must be true.
  template<class F> constexpr size_type min_left(size_type r, F f) const {
#ifndef NDEBUG
    if(r > n) throw Exception("SegmentTree::min_left: index ", r, " is out of range [0, ", n, "]");
    if(!std::invoke(f, spec.e())) throw Exception("SegmentTree::min_left: predicate must be true for identity");
#endif
    if(r == 0) return 0;
    value_type sm = spec.e();
    r += sz;
    do {
      r--;
      while(r > 1 && (r % 2)) r >>= 1;
      if(!std::invoke(f, spec.op(tree[r], sm))) {
        while(r < sz) {
          r = (2 * r + 1);
          if(std::invoke(f, spec.op(tree[r], sm))) {
            sm = spec.op(tree[r], sm);
            r--;
          }
        }
        return r + 1 - sz;
      }
      sm = spec.op(tree[r], sm);
    } while((r & -r) != r);
    return 0;
  }
};
}
