#pragma once
#include "Algorithm.hpp"
#include "Memory.hpp"
namespace gsh {
namespace internal {
template<class V, class E> requires std::is_convertible_v<V, E> struct ReRootingNoOpPutEdge {
  constexpr E operator()(const V& x, u32, bool) const { return static_cast<E>(x); }
};
template<class E, class V> requires std::is_convertible_v<E, V> struct ReRootingNoOpPutVertex {
  constexpr V operator()(const E& x, u32) const { return static_cast<V>(x); }
};
template<class E, class V, class Merge, class Identity, class PutEdge, class PutVertex> class DefaultReRootingSpec {
  [[no_unique_address]] mutable Merge merge_func;
  [[no_unique_address]] mutable Identity id_func;
  [[no_unique_address]] mutable PutEdge put_edge_func;
  [[no_unique_address]] mutable PutVertex put_vertex_func;
public:
  using edge_type = E;
  using value_type = V;
  constexpr DefaultReRootingSpec() = default;
  constexpr DefaultReRootingSpec(const Merge& merge, const Identity& id, const PutEdge& put_edge, const PutVertex& put_vertex) : merge_func(merge), id_func(id), put_edge_func(put_edge), put_vertex_func(put_vertex) {}
  constexpr E merge(const E& a, const E& b) const { return static_cast<E>(std::invoke(merge_func, a, b)); }
  constexpr E identity() const noexcept(noexcept(std::is_nothrow_invocable_v<Identity>)) { return static_cast<E>(std::invoke(id_func)); }
  constexpr E put_edge(const V& x, u32 edge_idx, bool edge_rev) const {
    if constexpr(requires { std::invoke(put_edge_func, x, edge_idx, edge_rev); }) return static_cast<E>(std::invoke(put_edge_func, x, edge_idx, edge_rev));
    else if constexpr(requires { std::invoke(put_edge_func, x, edge_idx); }) return static_cast<E>(std::invoke(put_edge_func, x, edge_idx));
    else return static_cast<E>(std::invoke(put_edge_func, x));
  }
  constexpr V put_vertex(const E& x, u32 v) const {
    if constexpr(requires { std::invoke(put_vertex_func, x, v); }) return static_cast<V>(std::invoke(put_vertex_func, x, v));
    else return static_cast<V>(std::invoke(put_vertex_func, x));
  }
};
} // namespace internal
template<class E, class V = E, class Merge, class Identity, class PutEdge = internal::ReRootingNoOpPutEdge<V, E>, class PutVertex = internal::ReRootingNoOpPutVertex<E, V>> constexpr internal::DefaultReRootingSpec<E, V, Merge, Identity, PutEdge, PutVertex> MakeReRootingSpec(const Merge& merge = Merge(), const Identity& id = Identity(), const PutEdge& put_edge = PutEdge(), const PutVertex& put_vertex = PutVertex()) { return {merge, id, put_edge, put_vertex}; }
template<class Spec, std::ranges::forward_range EdgeRange> requires std::ranges::sized_range<EdgeRange> constexpr Vec<typename Spec::value_type> ReRooting(const EdgeRange& edges, Spec spec = Spec()) {
  using E = typename Spec::edge_type;
  using V = typename Spec::value_type;
  const u32 m = static_cast<u32>(std::ranges::size(edges));
  u32 n = m + 1;
  Mem<u32> deg(n, 0);
  u32 max_deg = 0;
  for(auto&& [a, b] : edges) {
    ++deg[static_cast<u32>(a)];
    ++deg[static_cast<u32>(b)];
  }
  for(u32 v = 0; v != n; ++v) Chmax(max_deg, deg[v]);
  Mem<u32> off(n + 1, 0);
  for(u32 v = 0; v != n; ++v) off[v + 1] = off[v] + deg[v];
  Mem<u32> cur = off;
  struct Adj {
    u32 to;
    u32 edge_idx;
    bool rev;
    u32 rev_idx;
  };
  Mem<Adj> g(2 * m);
  for(u32 i = 0; auto&& [a, b] : edges) {
    const u32 x = static_cast<u32>(a);
    const u32 y = static_cast<u32>(b);
    const u32 ix = cur[x]++;
    const u32 iy = cur[y]++;
    g[ix] = Adj{y, i, false, iy};
    g[iy] = Adj{x, i, true, ix};
    ++i;
  }
  constexpr u32 npos = 0xffffffffu;
  Mem<u32> parent(n, npos);
  Mem<u32> parent_dir(n, npos);
  Mem<u32> order(n);
  u32 order_sz = 0;
  {
    Mem<u32> st(n);
    u32 sp = 0;
    parent[0] = 0;
    st[sp++] = 0;
    while(sp) {
      const u32 v = st[--sp];
      order[order_sz++] = v;
      for(u32 ei = off[v]; ei != off[v + 1]; ++ei) {
        const u32 to = g[ei].to;
        if(to == parent[v]) continue;
        if(parent[to] != npos) continue;
        parent[to] = v;
        parent_dir[to] = g[ei].rev_idx;
        st[sp++] = to;
      }
    }
  }
  const V init_v = spec.put_vertex(spec.identity(), 0);
  Mem<V> msg(2 * m, init_v);
  for(u32 it = order_sz; it--;) {
    const u32 v = order[it];
    E lower = spec.identity();
    for(u32 ei = off[v]; ei != off[v + 1]; ++ei) {
      const u32 to = g[ei].to;
      if(v != 0 && to == parent[v]) continue;
      lower = spec.merge(lower, spec.put_edge(msg[g[ei].rev_idx], g[ei].edge_idx, g[ei].rev));
    }
    V branch = spec.put_vertex(lower, v);
    if(v != 0) msg[parent_dir[v]] = std::move(branch);
  }
  Mem<E> suffix_buf(max_deg, spec.identity());
  Mem<E> contrib_buf(max_deg, spec.identity());
  Vec<V> res(n, init_v);
  for(u32 it = 0; it != order_sz; ++it) {
    const u32 v = order[it];
    const u32 dv = off[v + 1] - off[v];
    E suffix = spec.identity();
    for(u32 k = dv; k--;) {
      const u32 ei = off[v] + k;
      suffix_buf[k] = suffix;
      contrib_buf[k] = spec.put_edge(msg[g[ei].rev_idx], g[ei].edge_idx, g[ei].rev);
      suffix = spec.merge(contrib_buf[k], suffix);
    }
    E prefix = spec.identity();
    for(u32 k = 0; k != dv; ++k) {
      const u32 ei = off[v] + k;
      E upper = spec.merge(prefix, suffix_buf[k]);
      msg[ei] = spec.put_vertex(upper, v);
      prefix = spec.merge(prefix, contrib_buf[k]);
    }
    res[v] = spec.put_vertex(prefix, v);
  }
  return res;
}
} // namespace gsh
