#pragma once
#include "Algorithm.hpp"
#include "Exception.hpp"
#include "Heap.hpp"
#include "UnionFind.hpp"
#include <algorithm>
#include <iterator>
#include <limits>
#include <numeric>
#include <tuple>
#include <type_traits>
#include <utility>
namespace gsh {
template<class W = std::monostate> class Edge {
  u32 t = 0;
  [[no_unique_address]] W w{};
public:
  constexpr static bool is_weighted = !std::is_same_v<W, std::monostate>;
  using weight_type = std::conditional_t<is_weighted, W, u32>;
  constexpr Edge(u32 _t) : t(_t) {}
  constexpr Edge(u32 _t, W _w) : t(_t), w(_w) {}
  constexpr u32 to() const noexcept { return t; }
  constexpr W& weight() noexcept requires is_weighted { return w; }
  constexpr const W& weight() const noexcept requires is_weighted { return w; }
  constexpr u32 weight() const noexcept requires (!is_weighted) { return 1; }
  constexpr operator u32() const noexcept { return t; }
};
} // namespace gsh
namespace std {
template<class W> struct tuple_size<gsh::Edge<W>> : integral_constant<size_t, 2> {};
template<std::size_t M, class W> struct tuple_element<M, gsh::Edge<W>> {
  static_assert(M < 2, "std::tuple_element<gsh::Edge> / The index is out of range.");
  using type = std::conditional_t<M == 0, gsh::u32, W>;
};
} // namespace std
namespace gsh {
template<std::size_t M, class W> auto get(const Edge<W>& e) {
  static_assert(M <= 1, "gsh::get(gsh::Edge) / The index is out of range.");
  if constexpr(M == 0) return e.to();
  else return e.weight();
}
template<std::size_t M, class W> auto get(Edge<W>& e) {
  static_assert(M <= 1, "gsh::get(gsh::Edge) / The index is out of range.");
  if constexpr(M == 0) return e.to();
  else return e.weight();
}
template<std::size_t M, class W> auto get(Edge<W>&& e) {
  static_assert(M <= 1, "gsh::get(gsh::Edge) / The index is out of range.");
  if constexpr(M == 0) return e.to();
  else return std::move(e.weight());
}
struct ConnectedComponents {
  Vec<Vec<u32>> vertex;
  Vec<u32> aff;
  constexpr u32 size() const noexcept { return aff.size(); }
  constexpr bool empty() const noexcept { return aff.empty(); }
  constexpr u32 count_groups() const noexcept { return vertex.size(); }
  constexpr u32 affiliation(u32 v) const noexcept { return aff[v]; }
  constexpr const Vec<u32>& affiliation() const noexcept { return aff; }
  constexpr const Vec<Vec<u32>>& groups() const noexcept { return vertex; }
  constexpr const Vec<u32>& extract(u32 v) const { return vertex[aff[v]]; }
  constexpr u32 leader(u32 v) const { return vertex[aff[v]][0]; }
  constexpr bool same(u32 a, u32 b) const noexcept { return aff[a] == aff[b]; }
};
template<class WTT> struct ShortestPathResult {
  using weight_type = WTT;
  weight_type inf;
  Vec<weight_type> dist;
  Vec<u32> prev;
  constexpr Vec<u32> restore(u32 t) const {
    Vec<u32> path;
    if(t >= prev.size()) return path;
    if(dist[t] == inf) return path;
    for(u32 cur = t;; cur = prev[cur]) {
      path.push_back(cur);
      if(prev[cur] == cur) break;
    }
    path.reverse();
    return path;
  }
};
namespace internal {
template<class W, bool IsConst> class AdjacencyList : public ViewInterface<AdjacencyList<W, IsConst>, Edge<W>> {
  constexpr static u32 npos = 0xffffffffu;
  using storage_type = Vec<std::pair<Edge<W>, u32>>;
  using storage_ptr_type = std::conditional_t<IsConst, typename storage_type::const_iterator, typename storage_type::iterator>;
  storage_ptr_type storage_ptr;
  u32 idx;
  template<class W2> friend class CRS;
  constexpr AdjacencyList(storage_ptr_type p, u32 i) : storage_ptr(p), idx(i) {}
  template<bool IterIsConst> class iterator_impl {
    storage_ptr_type storage_ptr;
    u32 current_idx;
    constexpr iterator_impl(storage_ptr_type p, u32 i) : storage_ptr(p), current_idx(i) {}
    friend class AdjacencyList;
  public:
    using difference_type = i32;
    using value_type = Edge<W>;
    using pointer = std::conditional_t<IterIsConst, const value_type*, value_type*>;
    using reference = std::conditional_t<IterIsConst, const value_type&, value_type&>;
    using iterator_category = std::forward_iterator_tag;
    constexpr iterator_impl() : storage_ptr(nullptr), current_idx(-1) {}
    constexpr reference operator*() const noexcept { return storage_ptr[current_idx].first; }
    constexpr pointer operator->() const noexcept { return &(storage_ptr[current_idx].first); }
    constexpr iterator_impl& operator++() {
      current_idx = storage_ptr[current_idx].second;
      return *this;
    }
    constexpr iterator_impl operator++(int) {
      auto copy = *this;
      operator++();
      return copy;
    }
    friend constexpr bool operator==(const iterator_impl& a, const iterator_impl& b) noexcept { return a.current_idx == b.current_idx; }
  };
public:
  using iterator = iterator_impl<IsConst>;
  using const_iterator = iterator_impl<true>;
  constexpr u32 size() const noexcept {
    u32 current_idx = idx;
    u32 cnt = 0;
    while(current_idx != npos) {
      current_idx = storage_ptr[current_idx].second;
      ++cnt;
    }
    return cnt;
  }
  constexpr bool empty() const noexcept { return idx == npos; }
  constexpr iterator begin() noexcept { return iterator(storage_ptr, idx); }
  constexpr iterator end() noexcept { return iterator(storage_ptr, npos); }
  constexpr const_iterator begin() const noexcept { return const_iterator(storage_ptr, idx); }
  constexpr const_iterator end() const noexcept { return const_iterator(storage_ptr, npos); }
};
template<class W> class CRS {
  constexpr static u32 npos = 0xffffffffu;
  Vec<std::pair<Edge<W>, u32>> storage;
  Vec<u32> tail;
  constexpr static bool is_weighted = Edge<W>::is_weighted;
  template<bool IsConst> using adjacency_list = AdjacencyList<W, IsConst>;
public:
  constexpr CRS() {}
  constexpr explicit CRS(u32 n) : tail(n, npos) {}
  constexpr u32 vertex_count() const noexcept { return tail.size(); }
  constexpr u32 edge_count() const noexcept { return storage.size(); }
  constexpr void connect(u32 from, u32 to) {
    storage.emplace_back(std::piecewise_construct, std::tuple{to}, std::tuple{tail[from]});
    tail[from] = storage.size() - 1;
  }
  constexpr void connect(u32 from, u32 to, const W& w) {
    storage.emplace_back(std::piecewise_construct, std::tuple{to, w}, std::tuple{tail[from]});
    tail[from] = storage.size() - 1;
  }
  constexpr auto operator[](u32 v) {
#ifndef NDEBUG
    if(v >= vertex_count()) [[unlikely]]
      throw Exception("gsh::graph_format::CRS::operator[] / The index is out of range. ( v=", v, ", size=", vertex_count(), " )");
#endif
    return adjacency_list<false>(storage.begin(), tail[v]);
  }
  constexpr auto operator[](u32 v) const {
#ifndef NDEBUG
    if(v >= vertex_count()) [[unlikely]]
      throw Exception("gsh::graph_format::CRS::operator[] const / The index is out of range. ( v=", v, ", size=", vertex_count(), " )");
#endif
    return adjacency_list<true>(storage.begin(), tail[v]);
  }
  constexpr void reserve(u32 m) { storage.reserve(m); }
};
template<class D, class W> class GraphInterface {
  constexpr D& derived() noexcept { return *static_cast<D*>(this); }
  constexpr const D& derived() const noexcept { return *static_cast<const D*>(this); }
public:
  using edge_type = Edge<W>;
  using weight_type = typename edge_type::weight_type;
  constexpr static bool is_weighted = edge_type::is_weighted;
  constexpr static weight_type default_inf() { return std::numeric_limits<weight_type>::max(); }
  template<class Comp = Less> constexpr auto shortest_path(u32 s, const weight_type& inf = default_inf(), const Comp& comp = Comp()) const {
    if constexpr(!is_weighted) return shortest_path_bfs(s, inf);
    else return shortest_path_dijkstra(s, inf, comp);
  }
  constexpr auto shortest_path_bfs(u32 s, const weight_type& inf = default_inf()) const {
    const u32 n = derived().vertex_count();
    ShortestPathResult<weight_type> res{inf, Vec<weight_type>(n, inf), Vec<u32>(n, 0)};
    Vec<u32> q;
    q.reserve(n);
    u32 head = 0;
    res.dist[s] = weight_type{};
    res.prev[s] = s;
    q.push_back(s);
    while(head < q.size()) {
      const u32 v = q[head++];
      for(const auto& e : derived()[v]) {
        const u32 to = e.to();
        if(res.dist[to] != inf) continue;
        res.dist[to] = res.dist[v] + static_cast<weight_type>(1);
        res.prev[to] = v;
        q.push_back(to);
      }
    }
    return res;
  }
  template<class Comp = Less> constexpr auto shortest_path_dijkstra(u32 s, const weight_type& inf = default_inf(), const Comp& comp = Comp()) const {
    const u32 n = derived().vertex_count();
    ShortestPathResult<weight_type> res{inf, Vec<weight_type>(n, inf), Vec<u32>(n, 0)};
    struct Node {
      weight_type d;
      u32 v;
    };
    struct NodeComp {
      Comp comp;
      constexpr bool operator()(const Node& a, const Node& b) const noexcept(noexcept(std::invoke(comp, a.d, b.d))) { return std::invoke(comp, a.d, b.d); }
    };
    Heap<Node, NodeComp> pq(NodeComp{comp});
    pq.reserve(derived().edge_count());
    res.dist[s] = weight_type{};
    res.prev[s] = s;
    pq.push(Node{weight_type{}, s});
    while(!pq.empty()) {
      const auto cur = pq.min();
      pq.pop_min();
      if(cur.d != res.dist[cur.v]) continue;
      for(const auto& e : derived()[cur.v]) {
        const u32 to = e.to();
        const weight_type w = e.weight();
        if(res.dist[cur.v] == inf) continue;
        const weight_type nd = cur.d + w;
        if(std::invoke(comp, nd, res.dist[to])) {
          res.dist[to] = nd;
          res.prev[to] = cur.v;
          pq.push(Node{nd, to});
        }
      }
    }
    return res;
  }
};
template<class D, class W> class DirectedGraphInterface : public GraphInterface<D, W> {
  constexpr D& derived() noexcept { return *static_cast<D*>(this); }
  constexpr const D& derived() const noexcept { return *static_cast<const D*>(this); }
public:
  using edge_type = Edge<W>;
  using weight_type = typename edge_type::weight_type;
  constexpr static bool is_weighted = edge_type::is_weighted;
  constexpr Vec<u32> indegree() const {
    const u32 n = derived().vertex_count();
    Vec<u32> deg(n, 0);
    for(u32 i = 0; i != n; ++i) {
      for(const auto& e : derived()[i]) ++deg[e.to()];
    }
    return deg;
  }
  constexpr Vec<u32> outdegree() const {
    const u32 n = derived().vertex_count();
    Vec<u32> deg(n);
    for(u32 i = 0; i != n; ++i) deg[i] = derived()[i].size();
    return deg;
  }
  constexpr Vec<u32> topological_sort() const {
    const u32 n = derived().vertex_count();
    Vec<u32> indeg = derived().indegree();
    Vec<u32> st;
    st.reserve(n);
    for(u32 i = 0; i != n; ++i)
      if(indeg[i] == 0) st.push_back(i);
    Vec<u32> res;
    res.reserve(n);
    while(!st.empty()) {
      const u32 v = st.back();
      st.pop_back();
      res.push_back(v);
      for(const auto& e : derived()[v]) {
        const u32 to = e.to();
        if(--indeg[to] == 0) st.push_back(to);
      }
    }
    return res;
  }
  template<class Comp = Less> constexpr Vec<u32> topological_sort(Comp comp = Comp()) const {
    const u32 n = derived().vertex_count();
    Vec<u32> indeg = derived().indegree();
    Heap<u32, Comp> heap(comp);
    for(u32 i = 0; i != n; ++i)
      if(indeg[i] == 0) heap.push(i);
    Vec<u32> res;
    res.reserve(n);
    while(!heap.empty()) {
      const u32 v = heap.min();
      heap.pop_min();
      res.push_back(v);
      for(const auto& e : derived()[v]) {
        const u32 to = e.to();
        if(--indeg[to] == 0) heap.push(to);
      }
    }
    return res;
  }
  template<class Comp = Less> constexpr auto longest_path_length_dag(const Comp& comp = Comp()) const {
    const u32 n = derived().vertex_count();
    Vec<weight_type> dp(n, weight_type{});
    auto ord = topological_sort();
    for(u32 idx = 0; idx != ord.size(); ++idx) {
      const u32 v = ord[idx];
      for(const auto& e : derived()[v]) {
        const u32 to = e.to();
        const weight_type w = [&]() {
          if constexpr(is_weighted) return e.weight();
          else return static_cast<weight_type>(1);
        }();
        const weight_type cand = dp[v] + w;
        if(std::invoke(comp, dp[to], cand)) dp[to] = cand;
      }
    }
    return dp;
  }
  constexpr ConnectedComponents strongly_connected_components_decomposition() const {
    const u32 n = derived().vertex_count();
    Vec<Vec<u32>> g(n), rg(n);
    for(u32 v = 0; v != n; ++v) {
      for(const auto& e : derived()[v]) {
        const u32 to = e.to();
        g[v].push_back(to);
        rg[to].push_back(v);
      }
    }
    Vec<u8> seen(n, 0);
    Vec<u32> order;
    order.reserve(n);
    Vec<std::pair<u32, u32>> st;
    st.reserve(n);
    for(u32 s = 0; s != n; ++s) {
      if(seen[s]) continue;
      st.clear();
      st.push_back({s, 0});
      seen[s] = 1;
      while(!st.empty()) {
        auto& top = st.back();
        const u32 v = top.first;
        u32& i = top.second;
        if(i < g[v].size()) {
          const u32 to = g[v][i++];
          if(!seen[to]) {
            seen[to] = 1;
            st.push_back({to, 0});
          }
        } else {
          order.push_back(v);
          st.pop_back();
        }
      }
    }
    ConnectedComponents res;
    res.aff.assign(n, 0);
    Vec<u8> used(n, 0);
    Vec<u32> stack;
    stack.reserve(n);
    for(u32 k = order.size(); k != 0; --k) {
      const u32 s = order[k - 1];
      if(used[s]) continue;
      res.vertex.emplace_back();
      const u32 gid = res.vertex.size() - 1;
      stack.clear();
      stack.push_back(s);
      used[s] = 1;
      while(!stack.empty()) {
        const u32 v = stack.back();
        stack.pop_back();
        res.aff[v] = gid;
        res.vertex[gid].push_back(v);
        for(u32 to : rg[v]) {
          if(used[to]) continue;
          used[to] = 1;
          stack.push_back(to);
        }
      }
    }
    return res;
  }
};
template<class D, class W> class UndirectedGraphInterface : public GraphInterface<D, W> {
  constexpr D& derived() noexcept { return *static_cast<D*>(this); }
  constexpr const D& derived() const noexcept { return *static_cast<const D*>(this); }
public:
  using edge_type = Edge<W>;
  using weight_type = typename edge_type::weight_type;
  constexpr static bool is_weighted = edge_type::is_weighted;
  constexpr Vec<u32> degree() const {
    const u32 n = derived().vertex_count();
    Vec<u32> deg(n);
    for(u32 i = 0; i != n; ++i) deg[i] = derived()[i].size();
    return deg;
  }
  constexpr Vec<Vec<u32>> connected_components() const {
    const u32 n = derived().vertex_count();
    UnionFind uf(n);
    for(u32 i = 0; i != n; ++i) {
      for(const auto& e : derived()[i])
        if(i < e.to()) uf.merge(i, e.to());
    }
    return uf.groups();
  }
  constexpr u32 count_connected_components() const {
    const u32 n = derived().vertex_count();
    UnionFind uf(n);
    for(u32 i = 0; i != n; ++i) {
      for(const auto& e : derived()[i])
        if(i < e.to()) uf.merge(i, e.to());
    }
    return uf.count_groups();
  }
  constexpr bool is_connected_graph() const { return derived().vertex_count() == 0 ? true : (count_connected_components() == 1); }
  constexpr bool is_tree() const {
    const u32 n = derived().vertex_count();
    if(n == 0) return true;
    if(derived().edge_count() + 1 != n) return false;
    return is_connected_graph();
  }
  constexpr bool is_path_graph() const {
    const u32 n = derived().vertex_count();
    if(n == 0) return true;
    if(derived().edge_count() + 1 != n) return false;
    auto deg = derived().degree();
    u32 cnt1 = 0;
    for(u32 i = 0; i != n; ++i) {
      const u32 d = deg[i];
      if(d > 2) return false;
      cnt1 += (d == 1);
    }
    if(n == 1) return true;
    if(cnt1 != 2) return false;
    return is_connected_graph();
  }
  constexpr ConnectedComponents connected_components_decomposition() const {
    const u32 n = derived().vertex_count();
    ConnectedComponents res;
    res.vertex = connected_components();
    res.aff.assign(n, 0);
    for(u32 gid = 0; gid != res.vertex.size(); ++gid) {
      for(u32 v : res.vertex[gid]) res.aff[v] = gid;
    }
    return res;
  }
  template<class Comp = Less> constexpr auto minimum_spanning_forest_cost(const Comp& comp = Comp()) const {
    const u32 n = derived().vertex_count();
    struct E {
      weight_type w;
      u32 a;
      u32 b;
    };
    Vec<E> es;
    for(u32 i = 0; i != n; ++i) {
      for(const auto& e : derived()[i]) {
        const u32 j = e.to();
        if(i < j) {
          const weight_type w = [&]() {
            if constexpr(is_weighted) return e.weight();
            else return static_cast<weight_type>(1);
          }();
          es.push_back(E{w, i, j});
        }
      }
    }
    es.sort([&](const E& a, const E& b) { return std::invoke(comp, a.w, b.w); });
    UnionFind uf(n);
    weight_type res{};
    for(const auto& e : es) {
      if(!uf.merge_same(e.a, e.b)) res = res + e.w;
    }
    return res;
  }
  constexpr Vec<bool> bipartite_graph_coloring() const {
    const u32 n = derived().vertex_count();
    Vec<i32> col(n, -1);
    Vec<u32> q;
    q.reserve(n);
    for(u32 s = 0; s != n; ++s) {
      if(col[s] != -1) continue;
      u32 head = 0;
      q.clear();
      q.push_back(s);
      col[s] = 0;
      while(head < q.size()) {
        const u32 v = q[head++];
        for(const auto& e : derived()[v]) {
          const u32 to = e.to();
          if(col[to] == -1) {
            col[to] = col[v] ^ 1;
            q.push_back(to);
          } else if(col[to] == col[v]) {
            throw Exception("gsh::GraphInterface::bipartite_graph_coloring / The graph is not bipartite.");
          }
        }
      }
    }
    Vec<bool> res(n);
    for(u32 i = 0; i != n; ++i) res[i] = static_cast<bool>(col[i]);
    return res;
  }
  constexpr bool is_bipartite_graph() const {
    const u32 n = derived().vertex_count();
    Vec<i32> col(n, -1);
    Vec<u32> q;
    q.reserve(n);
    for(u32 s = 0; s != n; ++s) {
      if(col[s] != -1) continue;
      u32 head = 0;
      q.clear();
      q.push_back(s);
      col[s] = 0;
      while(head < q.size()) {
        const u32 v = q[head++];
        for(const auto& e : derived()[v]) {
          const u32 to = e.to();
          if(col[to] == -1) {
            col[to] = col[v] ^ 1;
            q.push_back(to);
          } else if(col[to] == col[v]) {
            return false;
          }
        }
      }
    }
    return true;
  }
};
}
template<class W = std::monostate> class DirectedGraph : public internal::CRS<W>, public internal::DirectedGraphInterface<DirectedGraph<W>, W> {
  using base = internal::CRS<W>;
public:
  constexpr DirectedGraph() = default;
  constexpr DirectedGraph(u32 n) : base(n) {}
};
template<class W = std::monostate> class UndirectedGraph : public internal::CRS<W>, public internal::UndirectedGraphInterface<UndirectedGraph<W>, W> {
  using base = internal::CRS<W>;
public:
  constexpr UndirectedGraph() = default;
  constexpr UndirectedGraph(u32 n) : base(n) {}
  constexpr void connect(u32 a, u32 b) {
    base::connect(a, b);
    base::connect(b, a);
  }
  constexpr void connect(u32 a, u32 b, const W& w) {
    base::connect(a, b, w);
    base::connect(b, a, w);
  }
  constexpr u32 edge_count() const noexcept { return base::edge_count() / 2; }
  constexpr void reserve(u32 m) { base::reserve(2 * m); }
};
} // namespace gsh
namespace std::ranges { template<class W, bool IsConst> inline constexpr bool enable_borrowed_range<gsh::internal::AdjacencyList<W, IsConst>> = true; }
