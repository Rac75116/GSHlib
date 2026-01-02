#pragma once
#include "Exception.hpp"
#include "Functional.hpp"
#include "Range.hpp"
#include "Vec.hpp"
#include <algorithm>
#include <bit>
#include <concepts>
#include <cstddef>
#include <iterator>
#include <ranges>
#include <type_traits>
#include <utility>
namespace gsh {
namespace internal {
template<class Spec> concept IsMergeSortTreeSpecImplemented = requires(Spec spec) {
  typename Spec::value_type;
  // Spec may be stateful or stateless.
  // Optional hooks (enabled if present):
  //   - on_build_begin(level, len)
  //   - build_node(level, start, sorted_subrange)
  //   - on_build_end()
  //   - aux(node_index)
  //   - comp(value, value)
};
template<class T, class BuildAux, class Comp = Less> class DefaultMergeSortTreeSpec {
  [[no_unique_address]] mutable BuildAux build_aux_func;
  [[maybe_unused]] mutable Comp comp_func;
public:
  using value_type = T;
  using range_type = decltype(Subrange(std::declval<const T*>(), std::declval<const T*>()));
  using aux_type = std::remove_cvref_t<std::invoke_result_t<BuildAux, range_type>>;
private:
  Vec<aux_type> aux_tree;
public:
  constexpr DefaultMergeSortTreeSpec() = default;
  constexpr DefaultMergeSortTreeSpec(BuildAux build_aux, Comp comp = Comp()) : build_aux_func(build_aux), comp_func(comp) {}
  constexpr void on_build_begin(u32 node_cnt) { aux_tree.assign(static_cast<u32>(node_cnt), aux_type{}); }
  constexpr void build_node(u32 node_index, range_type sorted) { aux_tree[static_cast<u32>(node_index)] = static_cast<aux_type>(std::invoke(build_aux_func, sorted)); }
  constexpr const aux_type& aux(u32 node_index) const { return aux_tree[static_cast<u32>(node_index)]; }
  constexpr bool comp(const value_type& a, const value_type& b) const { return std::invoke(comp_func, a, b); }
};
}
template<class T, class BuildAux> constexpr internal::DefaultMergeSortTreeSpec<T, BuildAux> MakeMergeSortTreeSpec() { return {}; }
template<class T, class BuildAux> constexpr internal::DefaultMergeSortTreeSpec<T, BuildAux> MakeMergeSortTreeSpec(BuildAux build_aux) { return internal::DefaultMergeSortTreeSpec<T, BuildAux>(build_aux); }
namespace merge_sort_tree_specs {
template<class T, class Comp = Less> class NoAux {
  [[maybe_unused]] mutable Comp comp_func;
public:
  using value_type = T;
  constexpr NoAux() = default;
  constexpr NoAux(Comp comp) : comp_func(comp) {}
  constexpr bool comp(const T& a, const T& b) const { return std::invoke(comp_func, a, b); }
};
} // namespace merge_sort_tree_specs
template<class Spec> requires internal::IsMergeSortTreeSpecImplemented<Spec> class MergeSortTree {
  [[no_unique_address]] Spec spec;
public:
  using value_type = typename Spec::value_type;
  using size_type = u32;
  using difference_type = i32;
private:
  using range_type = decltype(Subrange(std::declval<const value_type*>(), std::declval<const value_type*>()));
  constexpr static bool spec_has_on_build_begin = requires { spec.on_build_begin(0u, 0u); };
  constexpr static bool spec_has_build_node = requires { spec.build_node(0u, 0u, std::declval<range_type>()); };
  constexpr static bool spec_has_on_build_end = requires { spec.on_build_end(); };
  constexpr static bool spec_has_aux = requires { spec.aux(0u, 0u); };
  u32 n;
  Vec<value_type> seq;
  GSH_INTERNAL_INLINE constexpr bool comp(const value_type& a, const value_type& b) const {
    constexpr bool spec_has_comp = requires {
      { spec.comp(a, b) } -> std::same_as<bool>;
    };
    if constexpr(spec_has_comp) return spec.comp(a, b);
    else return Less()(a, b);
  }
  template<class Iter> constexpr void build(Iter iter, Iter sent) {
    n = static_cast<u32>(std::ranges::distance(iter, sent));
    if(n == 0) {
      clear();
      return;
    }
    u32 dep = static_cast<u32>(std::bit_width(n - 1u) + 1u);
    seq.resize(n * dep);
    if constexpr(spec_has_on_build_begin) spec.on_build_begin(dep, n);
    for(u32 i = 0; i != n; ++i) {
      seq[i] = *(iter++);
      if constexpr(spec_has_build_node) spec.build_node(0, i, range_type(seq.data() + i, seq.data() + i + 1));
    }
    for(u32 i = 0; i != dep - 1; ++i) {
      for(u32 j = 0; j < n; j += (2u << i)) {
        u32 len1 = 1u << i;
        u32 len2 = std::min<u32>(len1, n - j - len1);
        u32 idx1 = n * i + j;
        u32 idx2 = idx1 + len1;
        u32 idx3 = idx2 + len2;
        u32 idx4 = idx1 + n;
        u32 a = idx1, b = idx2, c = idx4;
        while(a != idx2 && b != idx3) {
          const auto& a_ref = seq[a];
          const auto& b_ref = seq[b];
          seq[c++] = comp(a_ref, b_ref) ? a_ref : b_ref;
        }
        while(a != idx2) seq[c++] = seq[a++];
        while(b != idx3) seq[c++] = seq[b++];
        if constexpr(spec_has_build_node) spec.build_node(i + 1, j >> (i + 1), range_type(seq.data() + idx4, seq.data() + c));
      }
    }
    if constexpr(spec_has_on_build_end) spec.on_build_end();
  }
public:
  constexpr MergeSortTree() = default;
  constexpr explicit MergeSortTree(Spec spec) : spec(spec) {}
  template<std::forward_iterator Iter> constexpr MergeSortTree(Iter iter, Iter sent, Spec spec = Spec()) : spec(spec) { build(iter, sent); }
  constexpr MergeSortTree(std::initializer_list<value_type> init, Spec spec = Spec()) : MergeSortTree(init.begin(), init.end(), spec) {}
  template<std::forward_iterator Iter> constexpr void assign(Iter iter, Iter sent) { build(iter, sent); }
  template<class Comp = Less> constexpr void assign(std::initializer_list<value_type> init) { assign(init.begin(), init.end()); }
  constexpr void clear() {
    n = 0;
    seq.clear();
    if constexpr(spec_has_on_build_begin) spec.on_build_begin(0u);
  }
  constexpr bool empty() const { return n == 0; }
  constexpr u32 size() const { return n; }
  template<class F> constexpr void visit(u32 l, u32 r, F f) const {
#ifndef NDEBUG
    if(l > r || r > n) throw Exception("MergeSortTree::visit: invalid range [", l, ", ", r, ") with size ", n);
#endif
    if(l == r) return;
    while(l < r) {
      const u32 rem = r - l;
      u32 len = static_cast<u32>(std::bit_floor(rem));
      if(l != 0) {
        const u32 lowbit = l & (~l + 1u);
        if(lowbit < len) len = lowbit;
      }
      const u32 level = static_cast<u32>(std::countr_zero(len));
      const u32 base = n * level + l;
      const range_type sorted(seq.data() + base, seq.data() + base + len);
      if constexpr(spec_has_aux && std::invocable<F, range_type, decltype(spec.aux(0u, 0u))>) std::invoke(f, sorted, spec.aux(level, l >> level));
      else std::invoke(f, sorted);
      l += len;
    }
  }
  constexpr u32 count_less_or_equal(u32 l, u32 r, const value_type& value) const {
    u32 ans = 0;
    visit(l, r, [&](const range_type sorted) {
      const auto it = sorted.upper_bound(value);
      ans += static_cast<u32>(std::ranges::distance(sorted.begin(), it));
    });
    return ans;
  }
  constexpr u32 count_less_than(u32 l, u32 r, const value_type& value) const {
    u32 ans = 0;
    visit(l, r, [&](const range_type sorted) {
      const auto it = sorted.lower_bound(value);
      ans += static_cast<u32>(std::ranges::distance(sorted.begin(), it));
    });
    return ans;
  }
  constexpr u32 count_greater_or_equal(u32 l, u32 r, const value_type& value) const {
    u32 ans = 0;
    visit(l, r, [&](const range_type sorted) {
      const auto it = sorted.lower_bound(value);
      const u32 less = static_cast<u32>(std::ranges::distance(sorted.begin(), it));
      ans += static_cast<u32>(sorted.size()) - less;
    });
    return ans;
  }
  constexpr u32 count_greater_then(u32 l, u32 r, const value_type& value) const {
    u32 ans = 0;
    visit(l, r, [&](const range_type sorted) {
      const auto it = sorted.upper_bound(value);
      const u32 leq = static_cast<u32>(std::ranges::distance(sorted.begin(), it));
      ans += static_cast<u32>(sorted.size()) - leq;
    });
    return ans;
  }
  constexpr u32 count_equal(u32 l, u32 r, const value_type& value) const {
    u32 ans = 0;
    visit(l, r, [&](const range_type sorted) {
      const auto it1 = sorted.lower_bound(value);
      const auto it2 = sorted.upper_bound(value);
      ans += static_cast<u32>(std::ranges::distance(it1, it2));
    });
    return ans;
  }
};
} // namespace gsh
