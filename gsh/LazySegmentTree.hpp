#pragma once
#include "Algorithm.hpp"
#include "Exception.hpp"
#include "Functional.hpp"
#include "Range.hpp"
#include "TypeDef.hpp"
#include "Vec.hpp"
#include "internal/UtilMacro.hpp"
#include <bit>
#include <concepts>
#include <functional>
#include <initializer_list>
#include <iterator>
#include <limits>
#include <optional>
#include <tuple>
#include <utility>
namespace gsh {
namespace internal {
template<class T, class Expected> concept IsValueOrOptional = std::same_as<T, Expected> || std::same_as<T, std::optional<Expected>>;
template<class Spec> concept IsLazySegmentSpecImplemented = requires(Spec spec) {
  typename Spec::value_type;
  typename Spec::operator_type;
  typename Spec::internal_value_type;
  typename Spec::internal_operator_type;
  { spec.op(std::declval<typename Spec::internal_value_type>(), std::declval<typename Spec::internal_value_type>()) } -> std::same_as<typename Spec::internal_value_type>;
  { spec.e() } -> std::same_as<typename Spec::internal_value_type>;
  { spec.mapping(std::declval<typename Spec::internal_operator_type>(), std::declval<typename Spec::internal_value_type>()) } -> IsValueOrOptional<typename Spec::internal_value_type>;
  { spec.composition(std::declval<typename Spec::internal_operator_type>(), std::declval<typename Spec::internal_operator_type>()) } -> std::same_as<typename Spec::internal_operator_type>;
  { spec.id() } -> std::same_as<typename Spec::internal_operator_type>;
  { spec.extract(std::declval<typename Spec::internal_value_type>()) } -> std::same_as<typename Spec::value_type>;
  { spec.embed_value(std::declval<typename Spec::value_type>()) } -> std::same_as<typename Spec::internal_value_type>;
  { spec.embed_operator(std::declval<typename Spec::operator_type>()) } -> std::same_as<typename Spec::internal_operator_type>;
};
template<class T, class U, class InternalT, class InternalU, class Op, class E, class Mapping, class Composition, class Id, class Extract, class EmbedValue, class EmbedOperator> class LazySegmentSpec {
  [[no_unique_address]] mutable Op op_func;
  [[no_unique_address]] mutable E e_func;
  [[no_unique_address]] mutable Mapping mapping_func;
  [[no_unique_address]] mutable Composition composition_func;
  [[no_unique_address]] mutable Id id_func;
  [[no_unique_address]] mutable Extract extract_func;
  [[no_unique_address]] mutable EmbedValue embed_value_func;
  [[no_unique_address]] mutable EmbedOperator embed_operator_func;
public:
  using value_type = T;
  using operator_type = U;
  using internal_value_type = InternalT;
  using internal_operator_type = InternalU;
  constexpr LazySegmentSpec() = default;
  constexpr LazySegmentSpec(const Op& op, const E& e, const Mapping& mapping, const Composition& composition, const Id& id, const Extract& extract, const EmbedValue& embed_value, const EmbedOperator& embed_operator) : op_func(op), e_func(e), mapping_func(mapping), composition_func(composition), id_func(id), extract_func(extract), embed_value_func(embed_value), embed_operator_func(embed_operator) {}
  GSH_INTERNAL_INLINE constexpr internal_value_type op(const internal_value_type& a, const internal_value_type& b) const { return static_cast<internal_value_type>(std::invoke(op_func, a, b)); }
  GSH_INTERNAL_INLINE constexpr internal_value_type e() const { return static_cast<internal_value_type>(std::invoke(e_func)); }
  GSH_INTERNAL_INLINE constexpr auto mapping(const internal_operator_type& f, const internal_value_type& x) const { return std::invoke(mapping_func, f, x); }
  GSH_INTERNAL_INLINE constexpr internal_operator_type composition(const internal_operator_type& f, const internal_operator_type& g) const { return static_cast<internal_operator_type>(std::invoke(composition_func, f, g)); }
  GSH_INTERNAL_INLINE constexpr internal_operator_type id() const { return static_cast<internal_operator_type>(std::invoke(id_func)); }
  GSH_INTERNAL_INLINE constexpr value_type extract(const internal_value_type& x) const { return static_cast<value_type>(std::invoke(extract_func, x)); }
  GSH_INTERNAL_INLINE constexpr internal_value_type embed_value(const value_type& x) const { return static_cast<internal_value_type>(std::invoke(embed_value_func, x)); }
  GSH_INTERNAL_INLINE constexpr internal_operator_type embed_operator(const operator_type& x) const { return static_cast<internal_operator_type>(std::invoke(embed_operator_func, x)); }
};
}
template<class T, class U, class InternalT = T, class InternalU = U, class Op, class E, class Mapping, class Composition, class Id, class Extract = Identity, class EmbedValue = Identity, class EmbedOperator = Identity> constexpr auto MakeLazySegmentSpec(const Op& op = Op(), const E& e = E(), const Mapping& mapping = Mapping(), const Composition& composition = Composition(), const Id& id = Id(), const Extract& extract = Extract(), const EmbedValue& embed_value = EmbedValue(), const EmbedOperator& embed_operator = EmbedOperator()) { return internal::LazySegmentSpec<T, U, InternalT, InternalU, Op, E, Mapping, Composition, Id, Extract, EmbedValue, EmbedOperator>{op, e, mapping, composition, id, extract, embed_value, embed_operator}; }
namespace segment_specs {
template<class T> class RangeAddRangeMin : public decltype(MakeLazySegmentSpec<T, T>(Min, []() -> T { return std::numeric_limits<T>::max(); }, Plus, Plus, []() -> T { return static_cast<T>(0); })) {};
template<class T> class RangeAddRangeMax : public decltype(MakeLazySegmentSpec<T, T>(Max, []() -> T { return std::numeric_limits<T>::lowest(); }, Plus, Plus, []() -> T { return static_cast<T>(0); })) {};
template<class T> class RangeAddRangeSum : public decltype(MakeLazySegmentSpec<T, T, std::pair<T, u32>, T>([](const std::pair<T, u32>& a, const std::pair<T, u32>& b) { return std::pair<T, u32>{a.first + b.first, a.second + b.second}; }, []() -> std::pair<T, u32> { return {static_cast<T>(0), 0}; }, [](const T& f, const std::pair<T, u32>& x) { return std::pair<T, u32>{x.first + f * static_cast<T>(x.second), x.second}; }, Plus, []() -> T { return static_cast<T>(0); }, [](const std::pair<T, u32>& x) -> T { return x.first; }, [](const T& x) -> std::pair<T, u32> { return {x, 1}; }, [](const T& x) -> T { return x; })) {};
template<class T> class RangeSetRangeMin : public decltype(MakeLazySegmentSpec<T, std::optional<T>>(Min, []() -> T { return std::numeric_limits<T>::max(); }, [](const std::optional<T>& f, const T& x) { return f ? *f : x; }, [](const std::optional<T>& f, const std::optional<T>& g) { return f ? f : g; }, []() -> std::optional<T> { return std::nullopt; })) {};
template<class T> class RangeSetRangeMax : public decltype(MakeLazySegmentSpec<T, std::optional<T>>(Max, []() -> T { return std::numeric_limits<T>::lowest(); }, [](const std::optional<T>& f, const T& x) { return f ? *f : x; }, [](const std::optional<T>& f, const std::optional<T>& g) { return f ? f : g; }, []() -> std::optional<T> { return std::nullopt; })) {};
template<class T> class RangeSetRangeSum : public decltype(MakeLazySegmentSpec<T, T, std::pair<T, u32>, std::optional<T>>([](const std::pair<T, u32>& a, const std::pair<T, u32>& b) { return std::pair<T, u32>{a.first + b.first, a.second + b.second}; }, []() -> std::pair<T, u32> { return {static_cast<T>(0), 0}; }, [](const std::optional<T>& f, const std::pair<T, u32>& x) { return f ? std::pair<T, u32>{(*f) * static_cast<T>(x.second), x.second} : x; }, [](const std::optional<T>& f, const std::optional<T>& g) { return f ? f : g; }, []() -> std::optional<T> { return std::nullopt; }, [](const std::pair<T, u32>& x) -> T { return x.first; }, [](const T& x) -> std::pair<T, u32> { return {x, 1}; }, [](const T& x) -> std::optional<T> { return x; })) {};
template<class T> class RangeXorRangeXor : public decltype(MakeLazySegmentSpec<T, T, std::pair<T, bool>, T>([](const std::pair<T, bool>& a, const std::pair<T, bool>& b) { return std::pair<T, bool>{a.first ^ b.first, a.second ^ b.second}; }, []() -> std::pair<T, bool> { return {static_cast<T>(0), false}; }, [](const T& f, const std::pair<T, bool>& x) { return std::pair<T, bool>{static_cast<T>(x.first ^ (x.second ? f : static_cast<T>(0))), x.second}; }, Xor, []() -> T { return static_cast<T>(0); }, [](const std::pair<T, bool>& x) -> T { return x.first; }, [](const T& x) -> std::pair<T, bool> { return {x, true}; }, [](const T& x) -> T { return x; })) {};
template<class T> class RangeOrRangeOr : public decltype(MakeLazySegmentSpec<T, T>(Or, []() -> T { return static_cast<T>(0); }, Or, Or, []() -> T { return static_cast<T>(0); })) {};
template<class T> class RangeAndRangeAnd : public decltype(MakeLazySegmentSpec<T, T>(And, []() -> T { return ~static_cast<T>(0); }, And, And, []() -> T { return ~static_cast<T>(0); })) {};
template<class T> class RangeAffineRangeSum : public decltype(MakeLazySegmentSpec<T, std::pair<T, T>, std::pair<T, u32>, std::pair<T, T>>([](const std::pair<T, u32>& a, const std::pair<T, u32>& b) { return std::pair<T, u32>{a.first + b.first, a.second + b.second}; }, []() -> std::pair<T, u32> { return {static_cast<T>(0), 0}; }, [](const std::pair<T, T>& f, const std::pair<T, u32>& x) { return std::pair<T, u32>{f.first * x.first + f.second * static_cast<T>(x.second), x.second}; }, [](const std::pair<T, T>& f, const std::pair<T, T>& g) { return std::pair<T, T>{f.first * g.first, f.first * g.second + f.second}; }, []() -> std::pair<T, T> { return {static_cast<T>(1), static_cast<T>(0)}; }, [](const std::pair<T, u32>& x) -> T { return x.first; }, [](const T& x) -> std::pair<T, u32> { return {x, 1}; }, [](const std::pair<T, T>& x) -> std::pair<T, T> { return x; })) {};
}
namespace internal {
template<class T> struct RangeChminChmaxAddRangeSumNode {
  T sum, max_val, max2_val, min_val, min2_val;
  u32 max_cnt, min_cnt, size;
};
template<class T> struct RangeChminChmaxAddRangeSumOp {
  T chmin_val, chmax_val, add_val;
};
template<class T> constexpr auto MakeRangeChminChmaxAddRangeSumSpec() {
  using Node = RangeChminChmaxAddRangeSumNode<T>;
  using Op = RangeChminChmaxAddRangeSumOp<T>;
  auto op = [](const Node& a, const Node& b) -> Node {
    if(a.size == 0) return b;
    if(b.size == 0) return a;
    Node res;
    res.sum = a.sum + b.sum;
    res.size = a.size + b.size;
    res.max_val = std::max(a.max_val, b.max_val);
    res.min_val = std::min(a.min_val, b.min_val);
    if(a.max_val > b.max_val) {
      res.max_cnt = a.max_cnt;
      res.max2_val = std::max(a.max2_val, b.max_val);
    } else if(a.max_val < b.max_val) {
      res.max_cnt = b.max_cnt;
      res.max2_val = std::max(a.max_val, b.max2_val);
    } else {
      res.max_cnt = a.max_cnt + b.max_cnt;
      res.max2_val = std::max(a.max2_val, b.max2_val);
    }
    if(a.min_val < b.min_val) {
      res.min_cnt = a.min_cnt;
      res.min2_val = std::min(a.min2_val, b.min_val);
    } else if(a.min_val > b.min_val) {
      res.min_cnt = b.min_cnt;
      res.min2_val = std::min(a.min_val, b.min2_val);
    } else {
      res.min_cnt = a.min_cnt + b.min_cnt;
      res.min2_val = std::min(a.min2_val, b.min2_val);
    }
    return res;
  };
  auto e = []() -> Node {
    constexpr T INF_MAX = std::numeric_limits<T>::max();
    constexpr T INF_MIN = std::numeric_limits<T>::lowest();
    return {static_cast<T>(0), INF_MIN, INF_MIN, INF_MAX, INF_MAX, 0, 0, 0};
  };
  auto mapping = [](const Op& f, const Node& x) -> std::optional<Node> {
    constexpr T INF_MAX = std::numeric_limits<T>::max();
    constexpr T INF_MIN = std::numeric_limits<T>::lowest();
    if(x.size == 0) return x;
    T upper = f.chmin_val;
    T lower = f.chmax_val;
    if(lower > upper) upper = lower;
    if(lower == upper) {
      Node res;
      res.size = x.size;
      res.sum = lower * static_cast<T>(res.size);
      res.max_val = lower;
      res.min_val = lower;
      res.max_cnt = res.size;
      res.min_cnt = res.size;
      res.max2_val = INF_MIN;
      res.min2_val = INF_MAX;
      return res;
    }
    Node res = x;
    if(f.add_val != static_cast<T>(0)) {
      res.sum += f.add_val * static_cast<T>(res.size);
      res.max_val += f.add_val;
      if(res.max_cnt != res.size) res.max2_val += f.add_val;
      res.min_val += f.add_val;
      if(res.min_cnt != res.size) res.min2_val += f.add_val;
    }
    if(res.max_val > upper) {
      if(res.max_cnt == res.size || res.max2_val < upper) {
        const T old_max = res.max_val;
        res.sum += (upper - old_max) * static_cast<T>(res.max_cnt);
        res.max_val = upper;
        if(res.min_val == old_max) res.min_val = upper;
        if(res.min2_val == old_max) res.min2_val = upper;
        if(res.max_val == res.min_val) {
          res.max_cnt = res.min_cnt = res.size;
          res.max2_val = INF_MIN;
          res.min2_val = INF_MAX;
        }
      } else {
        return std::nullopt;
      }
    }
    if(res.min_val < lower) {
      if(res.min_cnt == res.size || res.min2_val > lower) {
        const T old_min = res.min_val;
        res.sum += (lower - old_min) * static_cast<T>(res.min_cnt);
        res.min_val = lower;
        if(res.max_val == old_min) res.max_val = lower;
        if(res.max2_val == old_min) res.max2_val = lower;
        if(res.max_val == res.min_val) {
          res.max_cnt = res.min_cnt = res.size;
          res.max2_val = INF_MIN;
          res.min2_val = INF_MAX;
        }
      } else {
        return std::nullopt;
      }
    }
    return res;
  };
  auto composition = [](const Op& f, const Op& g) -> Op {
    constexpr T INF_MAX = std::numeric_limits<T>::max();
    constexpr T INF_MIN = std::numeric_limits<T>::lowest();
    auto clamp = [](const T& v, const T& lo, const T& hi) -> T { return std::min(std::max(v, lo), hi); };
    T f_upper = f.chmin_val;
    T f_lower = f.chmax_val;
    if(f_lower > f_upper) f_upper = f_lower;
    T g_upper = g.chmin_val;
    T g_lower = g.chmax_val;
    if(g_lower > g_upper) g_upper = g_lower;
    Op res;
    res.add_val = f.add_val + g.add_val;
    const T lo = f_lower;
    const T hi = f_upper;
    const T new_lower = (g_lower == INF_MIN) ? lo : clamp(g_lower + f.add_val, lo, hi);
    const T new_upper = (g_upper == INF_MAX) ? hi : clamp(g_upper + f.add_val, lo, hi);
    res.chmax_val = new_lower;
    res.chmin_val = new_upper;
    if(res.chmax_val > res.chmin_val) res.chmin_val = res.chmax_val;
    return res;
  };
  auto id = []() -> Op {
    constexpr T INF_MAX = std::numeric_limits<T>::max();
    constexpr T INF_MIN = std::numeric_limits<T>::lowest();
    return {INF_MAX, INF_MIN, static_cast<T>(0)};
  };
  auto extract = [](const Node& x) -> T { return x.sum; };
  auto embed_value = [](const T& x) -> Node {
    constexpr T INF_MAX = std::numeric_limits<T>::max();
    constexpr T INF_MIN = std::numeric_limits<T>::lowest();
    return {x, x, INF_MIN, x, INF_MAX, 1, 1, 1};
  };
  auto embed_operator = [](const std::tuple<T, T, T>& x) -> Op {
    Op res{std::get<0>(x), std::get<1>(x), std::get<2>(x)};
    if(res.chmax_val > res.chmin_val) res.chmin_val = res.chmax_val;
    return res;
  };
  return MakeLazySegmentSpec<T, std::tuple<T, T, T>, Node, Op>(op, e, mapping, composition, id, extract, embed_value, embed_operator);
}
}
namespace segment_specs {
template<class T> class RangeChminChmaxAddRangeSum : public decltype(internal::MakeRangeChminChmaxAddRangeSumSpec<T>()) {};
}
template<class Spec> requires internal::IsLazySegmentSpecImplemented<Spec> class LazySegmentTree : public ViewInterface<LazySegmentTree<Spec>, typename Spec::value_type> {
  [[no_unique_address]] Spec spec;
public:
  using value_type = typename Spec::value_type;
  using operator_type = typename Spec::operator_type;
  using internal_value_type = typename Spec::internal_value_type;
  using internal_operator_type = typename Spec::internal_operator_type;
  using size_type = u32;
  using difference_type = i32;
private:
  size_type n;
  size_type sz;
  size_type log;
  Vec<internal_value_type> tree;
  Vec<internal_operator_type> lazy;
  constexpr static bool is_beats = requires(Spec s, internal_operator_type f, internal_value_type x) {
    { s.mapping(f, x) } -> std::same_as<std::optional<internal_value_type>>;
  };
  GSH_INTERNAL_INLINE void update(size_type k) { tree[k] = spec.op(tree[2 * k], tree[2 * k + 1]); }
  GSH_INTERNAL_INLINE void all_apply(size_type k, const internal_operator_type& f) {
    if constexpr(is_beats) {
      auto res = spec.mapping(f, tree[k]);
      if(res.has_value()) {
        tree[k] = *res;
        if(k < sz) lazy[k] = spec.composition(f, lazy[k]);
      } else {
        all_apply_failed(k, f);
      }
    } else {
      tree[k] = spec.mapping(f, tree[k]);
      if(k < sz) lazy[k] = spec.composition(f, lazy[k]);
    }
  }
  void all_apply_failed(size_type k, const internal_operator_type& f) requires (is_beats) {
#ifndef NDEBUG
    if(k >= sz) [[unlikely]]
      throw Exception("LazySegmentTree: The operation cannot be applied.");
#endif
    internal_operator_type composed = spec.composition(f, lazy[k]);
    all_apply(2 * k, composed);
    all_apply(2 * k + 1, composed);
    lazy[k] = spec.id();
    update(k);
  }
  GSH_INTERNAL_INLINE void push(size_type k) {
    all_apply(2 * k, lazy[k]);
    all_apply(2 * k + 1, lazy[k]);
    lazy[k] = spec.id();
  }
public:
  constexpr LazySegmentTree() : n(0), sz(0), log(0) {}
  constexpr LazySegmentTree(Spec spec) : spec(spec), n(0), sz(0), log(0) {}
  constexpr LazySegmentTree(size_type n, Spec spec = Spec()) : spec(spec), n(n) {
    sz = n > 0 ? std::bit_ceil(n) : 0;
    log = sz ? std::countr_zero(sz) : 0;
    if(n > 0) {
      tree.assign(2 * sz, spec.e());
      lazy.assign(sz, spec.id());
    }
  }
  template<class InputIt> requires std::forward_iterator<InputIt> constexpr LazySegmentTree(InputIt first, InputIt last, Spec spec = Spec()) : spec(spec), n(std::ranges::distance(first, last)) {
    sz = n > 0 ? std::bit_ceil(n) : 0;
    log = sz ? std::countr_zero(sz) : 0;
    if(n > 0) {
      tree.assign(2 * sz, spec.e());
      lazy.assign(sz, spec.id());
      auto it = first;
      for(size_type i = 0; i < n; ++i, ++it) tree[sz + i] = spec.embed_value(*it);
      for(size_type i = sz - 1; i >= 1; --i) update(i);
    }
  }
  constexpr LazySegmentTree(size_type n, const value_type& value, Spec spec = Spec()) : LazySegmentTree(n, spec) {
    internal_value_type embedded = spec.embed_value(value);
    for(size_type i = 0; i < n; ++i) tree[sz + i] = embedded;
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
  constexpr void resize(size_type n) { resize(n, spec.extract(spec.e())); }
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
    log = sz ? std::countr_zero(sz) : 0;
    if(n > 0) {
      tree.assign(2 * sz, spec.e());
      lazy.assign(sz, spec.id());
      auto it = first;
      for(size_type i = 0; i < n; ++i, ++it) tree[sz + i] = spec.embed_value(*it);
      for(size_type i = sz - 1; i >= 1; --i) update(i);
    } else {
      tree.clear();
      lazy.clear();
    }
  }
  constexpr void assign(size_type n, const value_type& u) {
    this->n = n;
    sz = n > 0 ? std::bit_ceil(n) : 0;
    log = sz ? std::countr_zero(sz) : 0;
    if(n > 0) {
      tree.assign(2 * sz, spec.e());
      lazy.assign(sz, spec.id());
      internal_value_type embedded = spec.embed_value(u);
      for(size_type i = 0; i < n; ++i) tree[sz + i] = embedded;
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
    tree[p] = spec.embed_value(x);
    for(size_type i = 1; i <= log; ++i) update(p >> i);
  }
  constexpr value_type get(size_type p) {
#ifndef NDEBUG
    if(p >= n) throw Exception("LazySegmentTree::get: index ", p, " is out of range [0, ", n, ")");
#endif
    p += sz;
    for(size_type i = log; i >= 1; --i) push(p >> i);
    return spec.extract(tree[p]);
  }
  constexpr value_type operator[](size_type p) { return get(p); }
  constexpr value_type prod(size_type l, size_type r) {
#ifndef NDEBUG
    if(l > r || r > n) throw Exception("LazySegmentTree::prod: invalid range [", l, ", ", r, ") with size ", n);
#endif
    if(l == r) return spec.extract(spec.e());
    l += sz;
    r += sz;
    for(size_type i = log; i >= 1; --i) {
      if(((l >> i) << i) != l) push(l >> i);
      if(((r >> i) << i) != r) push((r - 1) >> i);
    }
    internal_value_type sml = spec.e(), smr = spec.e();
    while(l < r) {
      if(l & 1) sml = spec.op(sml, tree[l++]);
      if(r & 1) smr = spec.op(tree[--r], smr);
      l >>= 1;
      r >>= 1;
    }
    return spec.extract(spec.op(sml, smr));
  }
  constexpr value_type all_prod() const { return n > 0 ? spec.extract(tree[1]) : spec.extract(spec.e()); }
  constexpr void apply(size_type p, const operator_type& f) {
#ifndef NDEBUG
    if(p >= n) throw Exception("LazySegmentTree::apply: index ", p, " is out of range [0, ", n, ")");
#endif
    p += sz;
    for(size_type i = log; i >= 1; --i) push(p >> i);
    all_apply(p, spec.embed_operator(f));
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
    internal_operator_type embedded_f = spec.embed_operator(f);
    {
      size_type l2 = l, r2 = r;
      while(l < r) {
        if(l & 1) all_apply(l++, embedded_f);
        if(r & 1) all_apply(--r, embedded_f);
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
    if(!std::invoke(f, spec.extract(spec.e()))) throw Exception("LazySegmentTree::max_right: predicate must be true for identity");
#endif
    if(l == n) return n;
    l += sz;
    for(size_type i = log; i >= 1; --i) push(l >> i);
    internal_value_type sm = spec.e();
    do {
      while(l % 2 == 0) l >>= 1;
      if(!std::invoke(f, spec.extract(spec.op(sm, tree[l])))) {
        while(l < sz) {
          push(l);
          l = (2 * l);
          if(std::invoke(f, spec.extract(spec.op(sm, tree[l])))) {
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
    if(!std::invoke(f, spec.extract(spec.e()))) throw Exception("LazySegmentTree::min_left: predicate must be true for identity");
#endif
    if(r == 0) return 0;
    r += sz;
    for(size_type i = log; i >= 1; --i) push((r - 1) >> i);
    internal_value_type sm = spec.e();
    do {
      r--;
      while(r > 1 && (r % 2)) r >>= 1;
      if(!std::invoke(f, spec.extract(spec.op(tree[r], sm)))) {
        while(r < sz) {
          push(r);
          r = (2 * r + 1);
          if(std::invoke(f, spec.extract(spec.op(tree[r], sm)))) {
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
