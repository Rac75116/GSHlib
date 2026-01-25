#pragma once
#include "SegmentTree.hpp"
#include <bit>
#include <concepts>
#include <iterator>
#include <limits>
#include <optional>
#include <utility>
namespace gsh {
namespace internal {
template<class Op, class E, class Mapping, class Composition, class Id> concept IsValidLazySegmentSpecFunctors = std::invocable<E> && std::invocable<Id> && requires(Op op, Mapping mapping, Composition composition, const std::invoke_result_t<E>& v, const std::invoke_result_t<Id>& f) {
  { op(v, v) } -> std::convertible_to<std::remove_cvref_t<std::invoke_result_t<E>>>;
  { mapping(f, v) } -> std::convertible_to<std::remove_cvref_t<std::invoke_result_t<E>>>;
  { composition(f, f) } -> std::convertible_to<std::remove_cvref_t<std::invoke_result_t<Id>>>;
};
template<class Spec> concept IsLazySegmentSpecImplemented = requires(Spec spec) {
  typename Spec::value_type;
  typename Spec::operator_type;
  { spec.op(std::declval<typename Spec::value_type>(), std::declval<typename Spec::value_type>()) } -> std::same_as<typename Spec::value_type>;
  { spec.e() } -> std::same_as<typename Spec::value_type>;
  { spec.mapping(std::declval<typename Spec::operator_type>(), std::declval<typename Spec::value_type>()) } -> std::same_as<typename Spec::value_type>;
  { spec.composition(std::declval<typename Spec::operator_type>(), std::declval<typename Spec::operator_type>()) } -> std::same_as<typename Spec::operator_type>;
  { spec.id() } -> std::same_as<typename Spec::operator_type>;
};
template<class T, class U, class Op, class E, class Mapping, class Composition, class Id> requires IsValidLazySegmentSpecFunctors<Op, E, Mapping, Composition, Id> class LazySegmentSpec {
  [[no_unique_address]] mutable Op op_func;
  [[no_unique_address]] mutable E e_func;
  [[no_unique_address]] mutable Mapping mapping_func;
  [[no_unique_address]] mutable Composition composition_func;
  [[no_unique_address]] mutable Id id_func;
public:
  using value_type = T;
  using operator_type = U;
  constexpr LazySegmentSpec() = default;
  constexpr LazySegmentSpec(const Op& op, const E& e, const Mapping& mapping, const Composition& composition, const Id& id) : op_func(op), e_func(e), mapping_func(mapping), composition_func(composition), id_func(id) {}
  constexpr value_type op(const value_type& a, const value_type& b) const { return static_cast<value_type>(std::invoke(op_func, a, b)); }
  constexpr value_type e() const { return static_cast<value_type>(std::invoke(e_func)); }
  constexpr value_type mapping(const operator_type& f, const value_type& x) const { return static_cast<value_type>(std::invoke(mapping_func, f, x)); }
  constexpr operator_type composition(const operator_type& f, const operator_type& g) const { return static_cast<operator_type>(std::invoke(composition_func, f, g)); }
  constexpr operator_type id() const { return static_cast<operator_type>(std::invoke(id_func)); }
};
}
template<class T, class U, class Op, class E, class Mapping, class Composition, class Id> constexpr internal::LazySegmentSpec<T, U, Op, E, Mapping, Composition, Id> MakeLazySegmentSpec(const Op& op = Op(), const E& e = E(), const Mapping& mapping = Mapping(), const Composition& composition = Composition(), const Id& id = Id()) { return {op, e, mapping, composition, id}; }
namespace segment_specs {
template<class T> class RangeAddRangeMin : public decltype(MakeLazySegmentSpec<T, T>(Min, []() -> T { return std::numeric_limits<T>::max(); }, Plus, Plus, []() -> T { return static_cast<T>(0); })) {};
template<class T> class RangeAddRangeMax : public decltype(MakeLazySegmentSpec<T, T>(Max, []() -> T { return std::numeric_limits<T>::min(); }, Plus, Plus, []() -> T { return static_cast<T>(0); })) {};
template<class T> class RangeAddRangeSum : public decltype(MakeLazySegmentSpec<std::pair<T, u32>, T>([](const std::pair<T, u32>& a, const std::pair<T, u32>& b) { return std::pair<T, u32>{a.first + b.first, a.second + b.second}; }, []() -> std::pair<T, u32> { return {static_cast<T>(0), 0}; }, [](const T& f, const std::pair<T, u32>& x) { return std::pair<T, u32>{x.first + f * static_cast<T>(x.second), x.second}; }, Plus, []() -> T { return static_cast<T>(0); })) {};
template<class T> class RangeSetRangeMin : public decltype(MakeLazySegmentSpec<T, std::optional<T>>(Min, []() -> T { return std::numeric_limits<T>::max(); }, [](const std::optional<T>& f, const T& x) { return f ? *f : x; }, [](const std::optional<T>& f, const std::optional<T>& g) { return f ? f : g; }, []() -> std::optional<T> { return std::nullopt; })) {};
template<class T> class RangeSetRangeMax : public decltype(MakeLazySegmentSpec<T, std::optional<T>>(Max, []() -> T { return std::numeric_limits<T>::min(); }, [](const std::optional<T>& f, const T& x) { return f ? *f : x; }, [](const std::optional<T>& f, const std::optional<T>& g) { return f ? f : g; }, []() -> std::optional<T> { return std::nullopt; })) {};
template<class T> class RangeSetRangeSum : public decltype(MakeLazySegmentSpec<std::pair<T, u32>, std::optional<T>>([](const std::pair<T, u32>& a, const std::pair<T, u32>& b) { return std::pair<T, u32>{a.first + b.first, a.second + b.second}; }, []() -> std::pair<T, u32> { return {static_cast<T>(0), 0}; }, [](const std::optional<T>& f, const std::pair<T, u32>& x) { return f ? std::pair<T, u32>{(*f) * static_cast<T>(x.second), x.second} : x; }, [](const std::optional<T>& f, const std::optional<T>& g) { return f ? f : g; }, []() -> std::optional<T> { return std::nullopt; })) {};
template<class T> class RangeXorRangeXor : public decltype(MakeLazySegmentSpec<std::pair<T, bool>, T>([](const std::pair<T, bool>& a, const std::pair<T, bool>& b) { return std::pair<T, u32>{a.first ^ b.first, a.second ^ b.second}; }, []() -> std::pair<T, bool> { return {static_cast<T>(0), false}; }, [](const T& f, const std::pair<T, bool>& x) { return std::pair<T, bool>{static_cast<T>(x.first ^ (x.second ? f : static_cast<T>(0))), x.second}; }, Xor, []() -> T { return static_cast<T>(0); })) {};
template<class T> class RangeOrRangeOr : public decltype(MakeLazySegmentSpec<T, T>(Or, []() -> T { return static_cast<T>(0); }, Or, Or, []() -> T { return static_cast<T>(0); })) {};
template<class T> class RangeAndRangeAnd : public decltype(MakeLazySegmentSpec<T, T>(And, []() -> T { return ~static_cast<T>(0); }, And, And, []() -> T { return ~static_cast<T>(0); })) {};
template<class T> class RangeAffineRangeSum : public decltype(MakeLazySegmentSpec<std::pair<T, u32>, std::pair<T, T>>([](const std::pair<T, u32>& a, const std::pair<T, u32>& b) { return std::pair<T, u32>{a.first + b.first, a.second + b.second}; }, []() -> std::pair<T, u32> { return {static_cast<T>(0), 0}; }, [](const std::pair<T, T>& f, const std::pair<T, u32>& x) { return std::pair<T, u32>{f.first * x.first + f.second * static_cast<T>(x.second), x.second}; }, [](const std::pair<T, T>& f, const std::pair<T, T>& g) { return std::pair<T, T>{f.first * g.first, f.first * g.second + f.second}; }, []() -> std::pair<T, T> { return {static_cast<T>(1), static_cast<T>(0)}; })) {};
} // namespace segment_specs
template<class Spec> requires internal::IsLazySegmentSpecImplemented<Spec> class LazySegmentTree : public ViewInterface<LazySegmentTree<Spec>, typename Spec::value_type> {
  [[no_unique_address]] Spec spec;
public:
  using value_type = typename Spec::value_type;
  using operator_type = typename Spec::operator_type;
  using size_type = u32;
  using difference_type = i32;
private:
  size_type n;
  size_type sz;
  size_type log;
  Vec<value_type> tree;
  Vec<operator_type> lazy;
  void update(size_type k) { tree[k] = spec.op(tree[2 * k], tree[2 * k + 1]); }
  void all_apply(size_type k, const operator_type& f) {
    tree[k] = spec.mapping(f, tree[k]);
    if(k < sz) lazy[k] = spec.composition(f, lazy[k]);
  }
  void push(size_type k) {
    all_apply(2 * k, lazy[k]);
    all_apply(2 * k + 1, lazy[k]);
    lazy[k] = spec.id();
  }
public:
  constexpr LazySegmentTree() : n(0), sz(0), log(0) {}
  constexpr LazySegmentTree(Spec spec) : spec(spec), n(0), sz(0), log(0) {}
  constexpr LazySegmentTree(size_type n, Spec spec = Spec()) : spec(spec), n(n) {
    sz = n > 0 ? std::bit_ceil(n) : 0;
    log = std::countr_zero(sz);
    if(n > 0) {
      tree.assign(2 * sz, spec.e());
      lazy.assign(sz, spec.id());
    }
  }
  template<class InputIt> requires std::forward_iterator<InputIt> constexpr LazySegmentTree(InputIt first, InputIt last, Spec spec = Spec()) : spec(spec), n(std::ranges::distance(first, last)) {
    sz = n > 0 ? std::bit_ceil(n) : 0;
    log = std::countr_zero(sz);
    if(n > 0) {
      tree.assign(2 * sz, spec.e());
      lazy.assign(sz, spec.id());
      auto it = first;
      for(size_type i = 0; i < n; ++i, ++it) tree[sz + i] = *it;
      for(size_type i = sz - 1; i >= 1; --i) update(i);
    }
  }
  constexpr LazySegmentTree(size_type n, const value_type& value, Spec spec = Spec()) : LazySegmentTree(n, spec) {
    for(size_type i = 0; i < n; ++i) tree[sz + i] = value;
    for(size_type i = sz - 1; i >= 1; --i) update(i);
  }
  constexpr LazySegmentTree(std::initializer_list<value_type> init, Spec spec = Spec()) : LazySegmentTree(init.begin(), init.end(), spec) {}
  constexpr LazySegmentTree& operator=(std::initializer_list<value_type> il) {
    assign(il);
    return *this;
  }
  constexpr void clear() {
    n = 0;
    sz = 0;
    log = 0;
    tree.clear();
    lazy.clear();
  }
  constexpr bool empty() const { return n == 0; }
  constexpr size_type size() const { return n; }
  constexpr void resize(size_type n) { resize(n, spec.e()); }
  constexpr void resize(size_type n, const value_type& c) {
    Vec<value_type> tmp;
    tmp.reserve(this->n);
    for(size_type i = 0; i < this->n; ++i) tmp.emplace_back(get(i));
    tmp.resize(n, c);
    assign(tmp.begin(), tmp.end());
  }
  template<class InputIt> requires std::forward_iterator<InputIt> constexpr void assign(InputIt first, InputIt last) {
    n = std::ranges::distance(first, last);
    sz = n > 0 ? std::bit_ceil(n) : 0;
    log = std::countr_zero(sz);
    if(n > 0) {
      tree.assign(2 * sz, spec.e());
      lazy.assign(sz, spec.id());
      auto it = first;
      for(size_type i = 0; i < n; ++i, ++it) tree[sz + i] = *it;
      for(size_type i = sz - 1; i >= 1; --i) update(i);
    } else {
      tree.clear();
      lazy.clear();
    }
  }
  constexpr void assign(size_type n, const value_type& u) {
    this->n = n;
    sz = n > 0 ? std::bit_ceil(n) : 0;
    log = std::countr_zero(sz);
    if(n > 0) {
      tree.assign(2 * sz, spec.e());
      lazy.assign(sz, spec.id());
      for(size_type i = 0; i < n; ++i) tree[sz + i] = u;
      for(size_type i = sz - 1; i >= 1; --i) update(i);
    } else {
      tree.clear();
      lazy.clear();
    }
  }
  constexpr void assign(std::initializer_list<value_type> il) { assign(il.begin(), il.end()); }
  constexpr void swap(LazySegmentTree& r) {
    using std::swap;
    swap(spec, r.spec);
    swap(n, r.n);
    swap(sz, r.sz);
    swap(log, r.log);
    swap(tree, r.tree);
    swap(lazy, r.lazy);
  }
  constexpr void set(size_type p, const value_type& x) {
#ifndef NDEBUG
    if(p >= n) throw Exception("LazySegmentTree::set: index ", p, " is out of range [0, ", n, ")");
#endif
    p += sz;
    for(size_type i = log; i >= 1; --i) push(p >> i);
    tree[p] = x;
    for(size_type i = 1; i <= log; ++i) update(p >> i);
  }
  constexpr const value_type& get(size_type p) {
#ifndef NDEBUG
    if(p >= n) throw Exception("LazySegmentTree::get: index ", p, " is out of range [0, ", n, ")");
#endif
    p += sz;
    for(size_type i = log; i >= 1; --i) push(p >> i);
    return tree[p];
  }
  constexpr const value_type& operator[](size_type p) { return get(p); }
  constexpr value_type prod(size_type l, size_type r) {
#ifndef NDEBUG
    if(l > r || r > n) throw Exception("LazySegmentTree::prod: invalid range [", l, ", ", r, ") with size ", n);
#endif
    if(l == r) return spec.e();
    l += sz;
    r += sz;
    for(size_type i = log; i >= 1; --i) {
      if(((l >> i) << i) != l) push(l >> i);
      if(((r >> i) << i) != r) push((r - 1) >> i);
    }
    value_type sml = spec.e(), smr = spec.e();
    while(l < r) {
      if(l & 1) sml = spec.op(sml, tree[l++]);
      if(r & 1) smr = spec.op(tree[--r], smr);
      l >>= 1;
      r >>= 1;
    }
    return spec.op(sml, smr);
  }
  constexpr value_type all_prod() const { return n > 0 ? tree[1] : spec.e(); }
  constexpr void apply(size_type p, const operator_type& f) {
#ifndef NDEBUG
    if(p >= n) throw Exception("LazySegmentTree::apply: index ", p, " is out of range [0, ", n, ")");
#endif
    p += sz;
    for(size_type i = log; i >= 1; --i) push(p >> i);
    tree[p] = spec.mapping(f, tree[p]);
    for(size_type i = 1; i <= log; ++i) update(p >> i);
  }
  constexpr void apply(size_type l, size_type r, const operator_type& f) {
#ifndef NDEBUG
    if(l > r || r > n) throw Exception("LazySegmentTree::apply: invalid range [", l, ", ", r, ") with size ", n);
#endif
    if(l == r) return;
    l += sz;
    r += sz;
    for(size_type i = log; i >= 1; --i) {
      if(((l >> i) << i) != l) push(l >> i);
      if(((r >> i) << i) != r) push((r - 1) >> i);
    }
    {
      size_type l2 = l, r2 = r;
      while(l < r) {
        if(l & 1) all_apply(l++, f);
        if(r & 1) all_apply(--r, f);
        l >>= 1;
        r >>= 1;
      }
      l = l2;
      r = r2;
    }
    for(size_type i = 1; i <= log; ++i) {
      if(((l >> i) << i) != l) update(l >> i);
      if(((r >> i) << i) != r) update((r - 1) >> i);
    }
  }
  template<class F> constexpr size_type max_right(size_type l, F f) {
#ifndef NDEBUG
    if(l > n) throw Exception("LazySegmentTree::max_right: index ", l, " is out of range [0, ", n, "]");
    if(!std::invoke(f, spec.e())) throw Exception("LazySegmentTree::max_right: predicate must be true for identity");
#endif
    if(l == n) return n;
    l += sz;
    for(size_type i = log; i >= 1; --i) push(l >> i);
    value_type sm = spec.e();
    do {
      while(l % 2 == 0) l >>= 1;
      if(!std::invoke(f, spec.op(sm, tree[l]))) {
        while(l < sz) {
          push(l);
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
  template<class F> constexpr size_type min_left(size_type r, F f) {
#ifndef NDEBUG
    if(r > n) throw Exception("LazySegmentTree::min_left: index ", r, " is out of range [0, ", n, "]");
    if(!std::invoke(f, spec.e())) throw Exception("LazySegmentTree::min_left: predicate must be true for identity");
#endif
    if(r == 0) return 0;
    r += sz;
    for(size_type i = log; i >= 1; --i) push((r - 1) >> i);
    value_type sm = spec.e();
    do {
      r--;
      while(r > 1 && (r % 2)) r >>= 1;
      if(!std::invoke(f, spec.op(tree[r], sm))) {
        while(r < sz) {
          push(r);
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
} // namespace gsh
