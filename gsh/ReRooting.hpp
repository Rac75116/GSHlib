#pragma once
#include "Exception.hpp"
#include "Range.hpp"
#include "TypeDef.hpp"
#include "Vec.hpp"
#include <concepts>
#include <functional>
#include <type_traits>
#include <utility>
namespace gsh {
namespace internal {
template<class Edge> concept HasToMethod = requires(const Edge& e) {
  { e.to() } -> std::convertible_to<u32>;
};
template<class Edge> concept IsEdgeLike = HasToMethod<Edge> || std::convertible_to<Edge, u32>;
template<class Edge> GSH_INTERNAL_INLINE constexpr u32 EdgeTo(const Edge& e) noexcept {
  if constexpr(HasToMethod<Edge>) return static_cast<u32>(e.to());
  else return static_cast<u32>(e);
}
template<class Adj> concept IsAdjListLike = requires(const Adj& a, u32 i) {
  { a.size() } -> std::convertible_to<u32>;
  { a[i] };
} && IsEdgeLike<std::remove_cvref_t<decltype(std::declval<const Adj&>()[0])>>;
template<class Graph> concept IsTreeGraphLike = requires(const Graph& g, u32 v) {
  { g.size() } -> std::convertible_to<u32>;
  { g[v] };
} && IsAdjListLike<std::remove_cvref_t<decltype(std::declval<const Graph&>()[0])>>;
template<class Op, class Id, class Transfer, class AddRoot, class T, class Edge> concept IsValidReRootingSpec = std::invocable<Id> && requires(Op op, Id id, Transfer tr, AddRoot add, const T& a, const T& b, u32 from, u32 to, u32 v, const Edge& e) {
  { std::invoke(id) } -> std::convertible_to<T>;
  { std::invoke(op, a, b) } -> std::convertible_to<T>;
  { std::invoke(tr, a, from, to, e) } -> std::convertible_to<T>;
  { std::invoke(add, a, v) } -> std::convertible_to<T>;
};
template<class T, class Op, class Id, class Transfer, class AddRoot> class DefaultReRootingSpec {
  [[no_unique_address]] mutable Op op_func;
  [[no_unique_address]] mutable Id id_func;
  [[no_unique_address]] mutable Transfer transfer_func;
  [[no_unique_address]] mutable AddRoot add_root_func;
public:
  using value_type = T;
  constexpr DefaultReRootingSpec() = default;
  constexpr DefaultReRootingSpec(Op op, Id id, Transfer tr, AddRoot add) : op_func(op), id_func(id), transfer_func(tr), add_root_func(add) {}
  constexpr value_type op(const value_type& a, const value_type& b) const noexcept(noexcept(std::is_nothrow_invocable_v<Op, const value_type&, const value_type&>)) { return static_cast<value_type>(std::invoke(op_func, a, b)); }
  constexpr value_type e() const noexcept(noexcept(std::is_nothrow_invocable_v<Id>)) { return static_cast<value_type>(std::invoke(id_func)); }
  template<class Edge> constexpr value_type transfer(const value_type& x, u32 from, u32 to, const Edge& e) const noexcept(noexcept(std::is_nothrow_invocable_v<Transfer, const value_type&, u32, u32, const Edge&>)) { return static_cast<value_type>(std::invoke(transfer_func, x, from, to, e)); }
  constexpr value_type add_root(const value_type& x, u32 v) const noexcept(noexcept(std::is_nothrow_invocable_v<AddRoot, const value_type&, u32>)) { return static_cast<value_type>(std::invoke(add_root_func, x, v)); }
};
template<class Spec, class Edge> concept IsReRootingSpecImplemented = requires(Spec spec, const typename Spec::value_type& a, const typename Spec::value_type& b, u32 from, u32 to, u32 v, const Edge& e) {
  typename Spec::value_type;
  { spec.op(a, b) } -> std::same_as<typename Spec::value_type>;
  { spec.e() } -> std::same_as<typename Spec::value_type>;
  { spec.transfer(a, from, to, e) } -> std::same_as<typename Spec::value_type>;
  { spec.add_root(a, v) } -> std::same_as<typename Spec::value_type>;
};
} // namespace internal
template<class T, class Op, class Id, class Transfer, class AddRoot> constexpr internal::DefaultReRootingSpec<T, Op, Id, Transfer, AddRoot> MakeReRootingSpec() { return {}; }
template<class T, class Op, class Id, class Transfer, class AddRoot> constexpr internal::DefaultReRootingSpec<T, Op, Id, Transfer, AddRoot> MakeReRootingSpec(Op op, Id id, Transfer tr, AddRoot add) { return {op, id, tr, add}; }
template<class Spec> class ReRooting : public ViewInterface<ReRooting<Spec>, typename Spec::value_type> {
  [[no_unique_address]] Spec spec;
public:
  using value_type = typename Spec::value_type;
  using size_type = u32;
  using difference_type = i32;
private:
  size_type n = 0;
  Vec<value_type> res;
public:
  constexpr ReRooting() = default;
  constexpr explicit ReRooting(Spec spec) : spec(spec) {}
  template<class Graph> requires internal::IsTreeGraphLike<Graph> constexpr ReRooting(const Graph& g, Spec spec = Spec(), u32 root = 0) : spec(spec) { build(g, root); }
  constexpr auto begin() const { return res.cbegin(); }
  constexpr auto end() const { return res.cend(); }
  constexpr auto cbegin() const { return res.cbegin(); }
  constexpr auto cend() const { return res.cend(); }
  constexpr bool empty() const { return n == 0; }
  constexpr size_type size() const { return n; }
  constexpr const value_type& operator[](size_type i) const {
#ifndef NDEBUG
    if(i >= n) throw Exception("ReRooting::operator[]: index ", i, " is out of range [0, ", n, ")");
#endif
    return res[i];
  }
  constexpr const value_type& get(size_type i) const { return (*this)[i]; }
  constexpr const Vec<value_type>& data() const { return res; }
  template<class Graph> requires internal::IsTreeGraphLike<Graph> constexpr void build(const Graph& g, u32 root = 0) {
    n = static_cast<u32>(g.size());
    res.clear();
    if(n == 0) return;
#ifndef NDEBUG
    if(root >= n) throw Exception("ReRooting::build: root ", root, " is out of range [0, ", n, ")");
#endif
    using adj_type = std::remove_cvref_t<decltype(g[0])>;
    using edge_type = std::remove_cvref_t<decltype(std::declval<const adj_type&>()[0])>;
    static_assert(internal::IsReRootingSpecImplemented<Spec, edge_type>, "ReRooting: Spec is not implemented for given edge type.");
    const u32 NONE = 0xffffffffu;
    Vec<u32> parent(n, NONE);
    Vec<u32> parent_idx(n, NONE);
    Vec<u32> order;
    order.reserve(n);
    Vec<u32> st;
    st.reserve(n);
    parent[root] = root;
    st.push_back(root);
    while(!st.empty()) {
      const u32 v = st.back();
      st.pop_back();
      order.push_back(v);
      const auto& adj = g[v];
      const u32 deg = static_cast<u32>(adj.size());
      for(u32 i = 0; i < deg; ++i) {
        const auto& e = adj[i];
        const u32 to = internal::EdgeTo(e);
        if(to >= n) {
#ifndef NDEBUG
          throw Exception("ReRooting::build: edge endpoint ", to, " is out of range [0, ", n, ")");
#else
          continue;
#endif
        }
        if(parent[to] != NONE) continue;
        parent[to] = v;
        // Find reverse edge index at 'to' that points to 'v'
        const auto& adj_to = g[to];
        const u32 deg_to = static_cast<u32>(adj_to.size());
        u32 rev = NONE;
        for(u32 j = 0; j < deg_to; ++j) {
          if(internal::EdgeTo(adj_to[j]) == v) {
            rev = j;
            break;
          }
        }
#ifndef NDEBUG
        if(rev == NONE) throw Exception("ReRooting::build: missing reverse edge for (", v, " -> ", to, ")");
#endif
        parent_idx[to] = rev;
        st.push_back(to);
      }
    }
#ifndef NDEBUG
    if(order.size() != n) throw Exception("ReRooting::build: graph is not connected as a tree (visited=", order.size(), ", n=", n, ")");
#endif
    Vec<value_type> dp_sub(n, spec.e());
    for(i32 k = static_cast<i32>(n) - 1; k >= 0; --k) {
      const u32 v = order[static_cast<u32>(k)];
      value_type agg = spec.e();
      const auto& adj = g[v];
      const u32 deg = static_cast<u32>(adj.size());
      for(u32 i = 0; i < deg; ++i) {
        const u32 to = internal::EdgeTo(adj[i]);
        if(v != root && to == parent[v]) continue;
        // 'to' is a child in the rooted tree
        const auto& e_child_to_v = g[to][parent_idx[to]];
        agg = spec.op(agg, spec.transfer(dp_sub[to], to, v, e_child_to_v));
      }
      dp_sub[v] = spec.add_root(agg, v);
    }
    res.assign(n, spec.e());
    Vec<value_type> from_parent(n, spec.e());
    from_parent[root] = spec.e();
    for(u32 idx = 0; idx < n; ++idx) {
      const u32 v = order[idx];
      const auto& adj = g[v];
      const u32 deg = static_cast<u32>(adj.size());
      Vec<value_type> contrib(deg, spec.e());
      for(u32 i = 0; i < deg; ++i) {
        const u32 to = internal::EdgeTo(adj[i]);
        if(v != root && to == parent[v]) {
          contrib[i] = from_parent[v];
        } else {
          const auto& e_child_to_v = g[to][parent_idx[to]];
          contrib[i] = spec.transfer(dp_sub[to], to, v, e_child_to_v);
        }
      }
      Vec<value_type> pref(deg + 1, spec.e());
      for(u32 i = 0; i < deg; ++i) pref[i + 1] = spec.op(pref[i], contrib[i]);
      Vec<value_type> suf(deg + 1, spec.e());
      for(i32 i = static_cast<i32>(deg) - 1; i >= 0; --i) suf[static_cast<u32>(i)] = spec.op(contrib[static_cast<u32>(i)], suf[static_cast<u32>(i) + 1]);
      res[v] = spec.add_root(pref[deg], v);
      for(u32 i = 0; i < deg; ++i) {
        const u32 to = internal::EdgeTo(adj[i]);
        if(parent[to] != v) continue;
        const value_type excl = spec.op(pref[i], suf[i + 1]);
        const value_type dp_excl = spec.add_root(excl, v);
        from_parent[to] = spec.transfer(dp_excl, v, to, adj[i]);
      }
    }
  }
};
} // namespace gsh
