
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
template<class BuildAux, class T> concept IsValidMergeSortTreeBuildAux = requires(BuildAux build_aux, const T* first, const T* last) { build_aux(Subrange(first, last)); };
template<class Spec> concept IsMergeSortTreeSpecImplemented = requires(Spec spec, const typename Spec::value_type* first, const typename Spec::value_type* last, std::size_t node_index, std::size_t node_cnt) {
  typename Spec::value_type;
  // Spec may be stateful or stateless.
  // Optional build hooks (enabled if present):
  //   - on_build_begin(node_cnt)
  //   - build_node(node_index, sorted_subrange)
  //   - on_build_end()
  // Optional aux access (enabled if present):
  //   - aux(node_index)
  { Subrange(first, last) };
};
template<class T, class BuildAux> requires IsValidMergeSortTreeBuildAux<BuildAux, T> class DefaultMergeSortTreeSpec {
  [[no_unique_address]] mutable BuildAux build_aux_func;
public:
  using value_type = T;
  using range_type = decltype(Subrange(std::declval<const T*>(), std::declval<const T*>()));
  using aux_type = std::remove_cvref_t<std::invoke_result_t<BuildAux, range_type>>;
private:
  Vec<aux_type> aux_tree;
public:
  constexpr DefaultMergeSortTreeSpec() = default;
  constexpr explicit DefaultMergeSortTreeSpec(BuildAux build_aux) : build_aux_func(build_aux) {}
  constexpr void on_build_begin(std::size_t node_cnt) { aux_tree.assign(static_cast<u32>(node_cnt), aux_type{}); }
  constexpr void build_node(std::size_t node_index, range_type sorted) { aux_tree[static_cast<u32>(node_index)] = static_cast<aux_type>(std::invoke(build_aux_func, sorted)); }
  constexpr const aux_type& aux(std::size_t node_index) const { return aux_tree[static_cast<u32>(node_index)]; }
};
}
template<class T, class BuildAux> constexpr internal::DefaultMergeSortTreeSpec<T, BuildAux> MakeMergeSortTreeSpec() { return {}; }
template<class T, class BuildAux> constexpr internal::DefaultMergeSortTreeSpec<T, BuildAux> MakeMergeSortTreeSpec(BuildAux build_aux) { return internal::DefaultMergeSortTreeSpec<T, BuildAux>(build_aux); }
namespace merge_sort_tree_specs {
template<class T> struct NoAux {
  using value_type = T;
  // No aux / no hooks.
};
} // namespace merge_sort_tree_specs
template<class Spec> requires internal::IsMergeSortTreeSpecImplemented<Spec> class MergeSortTree {
  [[no_unique_address]] Spec spec;
public:
  using value_type = typename Spec::value_type;
  using size_type = std::size_t;
  using difference_type = std::ptrdiff_t;
  using range_type = decltype(Subrange(std::declval<const value_type*>(), std::declval<const value_type*>()));
  struct NodeView {
    range_type sorted;
    size_type node_index;
  };
private:
  size_type n = 0;
  size_type sz = 0;
  Vec<value_type> pool;
  Vec<size_type> offs;
  Vec<size_type> lens;
  static constexpr size_type parent(size_type i) { return i >> 1; }
  static constexpr size_type left(size_type i) { return i << 1; }
  static constexpr size_type right(size_type i) { return (i << 1) | 1; }
  template<class Comp> constexpr void build_from_values(const Vec<value_type>& values, Comp comp) {
    n = values.size();
    sz = n > 0 ? std::bit_ceil(n) : 0;
    if(n == 0) {
      pool.clear();
      offs.clear();
      lens.clear();
      return;
    }
    const size_type node_cnt = 2 * sz;
    offs.assign(static_cast<u32>(node_cnt), 0);
    lens.assign(static_cast<u32>(node_cnt), 0);
    if constexpr(requires { spec.on_build_begin(node_cnt); }) { spec.on_build_begin(node_cnt); }
    for(size_type i = 0; i < n; ++i) lens[static_cast<u32>(sz + i)] = 1;
    for(size_type i = sz - 1; i >= 1; --i) { lens[static_cast<u32>(i)] = lens[static_cast<u32>(left(i))] + lens[static_cast<u32>(right(i))]; }
    size_type total = 0;
    for(size_type i = 1; i < node_cnt; ++i) total += lens[static_cast<u32>(i)];
    pool.assign(static_cast<u32>(total), value_type{});
    size_type cur = 0;
    for(size_type i = 1; i < node_cnt; ++i) {
      offs[static_cast<u32>(i)] = cur;
      cur += lens[static_cast<u32>(i)];
    }
    for(size_type i = 0; i < n; ++i) {
      const auto node = sz + i;
      pool[static_cast<u32>(offs[static_cast<u32>(node)])] = values[static_cast<u32>(i)];
    }
    for(size_type i = sz - 1; i >= 1; --i) {
      const auto li = left(i);
      const auto ri = right(i);
      const auto lo = offs[static_cast<u32>(li)];
      const auto ro = offs[static_cast<u32>(ri)];
      const auto co = offs[static_cast<u32>(i)];
      const auto ll = lens[static_cast<u32>(li)];
      const auto rl = lens[static_cast<u32>(ri)];
      const value_type* lfirst = pool.begin() + lo;
      const value_type* llast = pool.begin() + lo + ll;
      const value_type* rfirst = pool.begin() + ro;
      const value_type* rlast = pool.begin() + ro + rl;
      value_type* out = pool.begin() + co;
      std::merge(lfirst, llast, rfirst, rlast, out, comp);
    }
    if constexpr(requires { spec.build_node(std::size_t{}, range_type{}); }) {
      for(size_type i = 1; i < node_cnt; ++i) {
        const auto o = offs[static_cast<u32>(i)];
        const auto l = lens[static_cast<u32>(i)];
        const value_type* first = pool.begin() + o;
        const value_type* last = pool.begin() + o + l;
        spec.build_node(i, Subrange(first, last));
      }
    }
    if constexpr(requires { spec.on_build_end(); }) { spec.on_build_end(); }
  }
  constexpr NodeView node_view(size_type node_index) const {
    const auto o = offs[static_cast<u32>(node_index)];
    const auto l = lens[static_cast<u32>(node_index)];
    const value_type* first = pool.begin() + o;
    const value_type* last = pool.begin() + o + l;
    return NodeView{Subrange(first, last), node_index};
  }
public:
  constexpr MergeSortTree() = default;
  constexpr explicit MergeSortTree(Spec spec) : spec(spec) {}
  template<class InputIt, class Comp = Less> requires std::forward_iterator<InputIt> constexpr MergeSortTree(InputIt first, InputIt last, Spec spec = Spec(), Comp comp = Comp()) : spec(spec) {
    Vec<value_type> values;
    const size_type m = std::ranges::distance(first, last);
    values.reserve(static_cast<u32>(m));
    for(auto it = first; it != last; ++it) values.emplace_back(*it);
    build_from_values(values, comp);
  }
  template<class Comp = Less> constexpr MergeSortTree(std::initializer_list<value_type> init, Spec spec = Spec(), Comp comp = Comp()) : spec(spec) {
    Vec<value_type> values(init.begin(), init.end());
    build_from_values(values, comp);
  }
  template<class InputIt, class Comp = Less> requires std::forward_iterator<InputIt> constexpr void assign(InputIt first, InputIt last, Comp comp = Comp()) {
    Vec<value_type> values;
    const size_type m = std::ranges::distance(first, last);
    values.reserve(static_cast<u32>(m));
    for(auto it = first; it != last; ++it) values.emplace_back(*it);
    build_from_values(values, comp);
  }
  template<class Comp = Less> constexpr void assign(std::initializer_list<value_type> init, Comp comp = Comp()) {
    Vec<value_type> values(init.begin(), init.end());
    build_from_values(values, comp);
  }
  constexpr void clear() {
    n = 0;
    sz = 0;
    pool.clear();
    offs.clear();
    lens.clear();
  }
  constexpr bool empty() const { return n == 0; }
  constexpr size_type size() const { return n; }
  template<class F> constexpr void visit(size_type l, size_type r, F&& f) const {
#ifndef NDEBUG
    if(l > r || r > n) throw Exception("MergeSortTree::visit: invalid range [", l, ", ", r, ") with size ", n);
#endif
    if(n == 0 || l == r) return;
    l += sz;
    r += sz;
    Vec<size_type> right_nodes;
    right_nodes.reserve(64);
    while(l < r) {
      if(l & 1) {
        const auto nv = node_view(l);
        if constexpr(requires { std::invoke(f, nv.sorted, spec.aux(nv.node_index)); }) {
          std::invoke(f, nv.sorted, spec.aux(nv.node_index));
        } else {
          std::invoke(f, nv.sorted);
        }
        ++l;
      }
      if(r & 1) {
        --r;
        right_nodes.emplace_back(r);
      }
      l >>= 1;
      r >>= 1;
    }
    for(auto it = right_nodes.rbegin(); it != right_nodes.rend(); ++it) {
      const auto nv = node_view(*it);
      if constexpr(requires { std::invoke(f, nv.sorted, spec.aux(nv.node_index)); }) {
        std::invoke(f, nv.sorted, spec.aux(nv.node_index));
      } else {
        std::invoke(f, nv.sorted);
      }
    }
  }
};
} // namespace gsh
