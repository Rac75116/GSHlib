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
  std::declval<Subrange<typename Spec::value_type>>();
  // Spec may be stateful or stateless.
  // Optional hooks (enabled if present):
  //   - on_build_begin(node_cnt)
  //   - build_node(node_index, sorted_subrange)
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
  constexpr static bool spec_has_on_build_begin = requires { spec.on_build_begin(0u); };
  constexpr static bool spec_has_build_node = requires { spec.build_node(0u, std::declval<range_type>()); };
  constexpr static bool spec_has_on_build_end = requires { spec.on_build_end(); };
  constexpr static bool spec_has_aux = requires { spec.aux(0u); };
  size_type n;
  Vec<value_type> seq;
  GSH_INTERNAL_INLINE constexpr bool comp(const value_type& a, const value_type& b) const {
    constexpr bool spec_has_comp = requires {
      { spec.comp(a, b) } -> std::same_as<bool>;
    };
    if constexpr(spec_has_comp) return spec.comp(a, b);
    else return Less()(a, b);
  }
  template<class Iter, class Sent> constexpr void build(Iter iter, Sent sent) {
    n = std::ranges::distance(iter, sent);
    if(n == 0) return;
    u32 dep = std::bit_width(n - 1) + 1;
    seq.resize(n * dep);
    if constexpr(spec_has_on_build_begin) {
      u32 node_cnt = 0;
      for(u32 i = 0; i != dep; ++i) node_cnt += (n + (1u << i) - 1) >> i;
      seq.on_build_begin(node_cnt);
    }
    for(u32 i = 0; i != n; ++i) {
      seq[i] = *(iter++);
      if constexpr(spec_has_build_node) spec.build_node(i, range_type(seq.data() + i, seq.data() + i + 1));
    }
    u32 node_idx = n;
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
        if constexpr(spec_has_build_node) spec.build_node(node_idx, range_type(seq.data() + idx4, seq.data() + c));
        ++node_idx;
      }
    }
    if constexpr(spec_has_on_build_end) spec.on_build_end();
  }
public:
  constexpr MergeSortTree() = default;
  constexpr explicit MergeSortTree(Spec spec) : spec(spec) {}
  template<std::forward_iterator Iter, std::sentinel_for<Iter> Sent> constexpr MergeSortTree(Iter iter, Sent sent, Spec spec = Spec()) : spec(spec) { build(iter, sent); }
  constexpr MergeSortTree(std::initializer_list<value_type> init, Spec spec = Spec()) : MergeSortTree(init.begin(), init.end(), spec) {}
  template<std::forward_iterator Iter, std::sentinel_for<Iter> Sent> constexpr void assign(Iter iter, Sent sent) { build(iter, sent); }
  template<class Comp = Less> constexpr void assign(std::initializer_list<value_type> init) { assign(init.begin(), init.end()); }
  constexpr void clear();
  constexpr bool empty() const;
  constexpr size_type size() const;
  template<class F> constexpr void visit(size_type l, size_type r, F&& f) const {
#ifndef NDEBUG
    if(l > r || r > n) throw Exception("MergeSortTree::visit: invalid range [", l, ", ", r, ") with size ", n);
#endif
  }
};
} // namespace gsh
