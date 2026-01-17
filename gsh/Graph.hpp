#pragma once
#include "Arr.hpp"
#include "Exception.hpp"
#include "Heap.hpp"
#include "Range.hpp"
#include "TypeDef.hpp"
#include "UnionFind.hpp"
#include "Vec.hpp"
#include <algorithm>
#include <iterator>
#include <limits>
#include <numeric>
#include <tuple>
#include <type_traits>
#include <utility>
namespace gsh {
template<class W = void> class Edge {
  u32 t = 0;
  W w{};
public:
  constexpr Edge(u32 _t) : t(_t) {}
  constexpr Edge(u32 _t, W _w) : t(_t), w(_w) {}
  constexpr u32 to() const noexcept { return t; }
  constexpr W& weight() noexcept { return w; }
  constexpr const W& weight() const noexcept { return w; }
  constexpr operator u32() const noexcept { return t; }
};
template<> class Edge<void> {
  u32 t = 0;
public:
  constexpr Edge(u32 _t) noexcept : t(_t) {}
  constexpr Edge(u32 _t, u32 _w) : t(_t) {
    if(_w != 1) throw gsh::Exception("gsh::Edge<void>::Edge / The weight is not 1.");
  }
  constexpr u32 to() const noexcept { return t; }
  constexpr u32 weight() const noexcept { return 1; }
  constexpr operator u32() const noexcept { return t; }
};
} // namespace gsh
namespace std {
template<class W> struct tuple_size<gsh::Edge<W>> : integral_constant<size_t, 2> {};
template<std::size_t M, class W> struct tuple_element<M, gsh::Edge<W>> {
  static_assert(M < 2, "std::tuple_element<gsh::Edge> / The index is out of range.");
  using type = std::conditional_t<M == 0, gsh::u32, W>;
};
template<std::size_t M> struct tuple_element<M, gsh::Edge<void>> {
  static_assert(M < 2, "std::tuple_element<gsh::Edge<void>> / The index is out of range.");
  using type = gsh::u32;
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
namespace graph_format {
template<class W> class CRS {
  constexpr static u32 npos = 0xffffffffu;
  Vec<std::pair<Edge<W>, u32>> storage;
  Vec<u32> tail;
  template<bool IsConst> class adjacency_list : public ViewInterface<adjacency_list<IsConst>, Edge<W>> {
    using storage_ptr_type = std::conditional_t<IsConst, typename decltype(storage)::const_iterator, typename decltype(storage)::iterator>;
    storage_ptr_type storage_ptr;
    u32 idx;
    friend class CRS;
    constexpr adjacency_list(storage_ptr_type p, u32 i) : storage_ptr(p), idx(i) {}
  public:
    class iterator {
      storage_ptr_type storage_ptr;
      u32 current_idx;
      constexpr iterator(storage_ptr_type p, u32 i) : storage_ptr(p), current_idx(i) {}
      friend class adjacency_list;
    public:
      using difference_type = i32;
      using value_type = Edge<W>;
      using pointer = std::conditional_t<IsConst, const value_type*, value_type*>;
      using reference = std::conditional_t<IsConst, const value_type&, value_type&>;
      using iterator_category = std::forward_iterator_tag;
      constexpr reference operator*() const noexcept { return storage_ptr[current_idx].first; }
      constexpr pointer operator->() const noexcept { return &(storage_ptr[current_idx].first); }
      constexpr iterator& operator++() {
        current_idx = storage_ptr[current_idx].second;
        return *this;
      }
      constexpr iterator operator++(int) {
        auto copy = *this;
        operator++();
        return copy;
      }
      friend constexpr bool operator==(const iterator& a, const iterator& b) noexcept { return a.current_idx == b.current_idx; }
    };
    class const_iterator {
      using storage_ptr_type = decltype(storage)::const_iterator;
      storage_ptr_type storage_ptr;
      u32 current_idx;
      constexpr const_iterator(storage_ptr_type p, u32 i) : storage_ptr(p), current_idx(i) {}
      friend class adjacency_list;
    public:
      using difference_type = i32;
      using value_type = Edge<W>;
      using pointer = const value_type*;
      using reference = const value_type&;
      using iterator_category = std::forward_iterator_tag;
      constexpr reference operator*() const noexcept { return storage_ptr[current_idx].first; }
      constexpr pointer operator->() const noexcept { return &(storage_ptr[current_idx].first); }
      constexpr const_iterator& operator++() {
        current_idx = storage_ptr[current_idx].second;
        return *this;
      }
      constexpr const_iterator operator++(int) {
        auto copy = *this;
        operator++();
        return copy;
      }
      friend constexpr bool operator==(const const_iterator& a, const const_iterator& b) noexcept { return a.current_idx == b.current_idx; }
    };
    constexpr bool empty() const noexcept { return idx == CRS::npos; }
    constexpr iterator begin() noexcept { return iterator(storage_ptr, idx); }
    constexpr iterator end() noexcept { return iterator(storage_ptr, CRS::npos); }
    constexpr iterator begin() const noexcept { return iterator(storage_ptr, idx); }
    constexpr iterator end() const noexcept { return iterator(storage_ptr, CRS::npos); }
  };
protected:
  constexpr CRS() {}
  constexpr explicit CRS(u32 n) : tail(n, npos) {}
  constexpr u32 vertex_count() const noexcept { return tail.size(); }
  constexpr u32 edge_count() const noexcept { return storage.size(); }
  constexpr void connect(u32 from, u32 to) {
    storage.emplace_back(std::piecewise_construct, std::tuple{to}, std::tuple{tail[from]});
    tail[from] = storage.size() - 1;
  }
  template<class WW = W> constexpr void connect(u32 from, u32 to, const WW& w) requires (!std::is_void_v<WW>) {
    storage.emplace_back(std::piecewise_construct, std::tuple{to, w}, std::tuple{tail[from]});
    tail[from] = storage.size() - 1;
  }
  template<class WW = W> constexpr void connect(u32 from, u32 to, WW&& w) requires (!std::is_void_v<WW>) {
    storage.emplace_back(std::piecewise_construct, std::tuple{to, std::move(w)}, std::tuple{tail[from]});
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
  constexpr u32 outdegree(u32 v) const {
#ifndef NDEBUG
    if(v >= vertex_count()) [[unlikely]]
      throw Exception("gsh::graph_format::CRS::outdegree / The index is out of range. ( v=", v, ", size=", vertex_count(), " )");
#endif
    u32 res = 0;
    for(auto it = adjacency_list<true>(storage.begin(), tail[v]).begin(); it != adjacency_list<true>(storage.begin(), tail[v]).end(); ++it) ++res;
    return res;
  }
};
} // namespace graph_format
namespace internal {
template<class T> constexpr T inf_value() {
  if constexpr(std::numeric_limits<T>::is_specialized) return std::numeric_limits<T>::max();
  else return T{};
}
template<class D, class WT, bool IsWeighted> class GraphInterface;
} // namespace internal
template<class W, template<class> class Format = graph_format::CRS> class DirectedGraph : public Format<W>, public internal::GraphInterface<DirectedGraph<W, Format>, std::conditional_t<std::is_void_v<W>, u32, W>, !std::is_void_v<W>> {
  using base = Format<W>;
public:
  using edge_type = Edge<W>;
  using weight_type = std::conditional_t<std::is_void_v<W>, u32, W>;
  constexpr static bool is_weighted = !std::is_void_v<W>;
  constexpr DirectedGraph() : base() {}
  constexpr explicit DirectedGraph(u32 n) : base(n) {}
  constexpr u32 vertex_count() const noexcept { return base::vertex_count(); }
  constexpr u32 edge_count() const noexcept { return base::edge_count(); }
  constexpr auto operator[](u32 v) { return base::operator[](v); }
  constexpr auto operator[](u32 v) const { return base::operator[](v); }
  constexpr void reserve(u32 m) { base::reserve(m); }
  constexpr void connect(u32 from, u32 to) { base::connect(from, to); }
  constexpr void connect(u32 from, u32 to, const weight_type& w) requires is_weighted { base::connect(from, to, w); }
  constexpr void connect(u32 from, u32 to, weight_type&& w) requires is_weighted { base::connect(from, to, std::move(w)); }
  constexpr u32 outdegree(u32 v) const { return base::outdegree(v); }
};
template<class W, template<class> class Format = graph_format::CRS> class UndirectedGraph : public Format<W>, public internal::GraphInterface<UndirectedGraph<W, Format>, std::conditional_t<std::is_void_v<W>, u32, W>, !std::is_void_v<W>> {
  using base = Format<W>;
public:
  using edge_type = Edge<W>;
  using weight_type = std::conditional_t<std::is_void_v<W>, u32, W>;
  constexpr static bool is_weighted = !std::is_void_v<W>;
  constexpr UndirectedGraph() : base() {}
  constexpr explicit UndirectedGraph(u32 n) : base(n) {}
  constexpr u32 vertex_count() const noexcept { return base::vertex_count(); }
  constexpr u32 edge_count() const noexcept { return base::edge_count() / 2; }
  constexpr auto operator[](u32 v) { return base::operator[](v); }
  constexpr auto operator[](u32 v) const { return base::operator[](v); }
  constexpr void reserve(u32 m) { base::reserve(m * 2); }
  constexpr void connect(u32 a, u32 b) {
    base::connect(a, b);
    base::connect(b, a);
  }
  constexpr void connect(u32 a, u32 b, const weight_type& w) requires is_weighted {
    base::connect(a, b, w);
    base::connect(b, a, w);
  }
  constexpr void connect(u32 a, u32 b, weight_type&& w) requires is_weighted {
    auto tmp = std::move(w);
    base::connect(a, b, tmp);
    base::connect(b, a, std::move(tmp));
  }
  constexpr u32 outdegree(u32 v) const { return base::outdegree(v); }
  constexpr u32 degree(u32 v) const { return base::outdegree(v); }
  constexpr auto to_directed() const {
    u32 n = vertex_count();
    DirectedGraph<W, Format> res(n);
    res.reserve(base::edge_count());
    for(u32 i = 0; i != n; ++i) {
      for(const auto& e : (*this)[i]) {
        if constexpr(!is_weighted) res.connect(i, e.to());
        else res.connect(i, e.to(), e.weight());
      }
    }
    return res;
  }
};
namespace internal {
template<class T> constexpr static bool IsGraphType = false;
template<class W, template<class> class Format> constexpr static bool IsGraphType<DirectedGraph<W, Format>> = true;
template<class W, template<class> class Format> constexpr static bool IsGraphType<UndirectedGraph<W, Format>> = true;
template<class T> concept GraphType = IsGraphType<T>;
template<class D, class WT, bool IsWeighted> class GraphInterface {
  constexpr D& derived() noexcept { return *static_cast<D*>(this); }
  constexpr const D& derived() const noexcept { return *static_cast<const D*>(this); }
  using weight_type = WT;
  constexpr static bool is_weighted = IsWeighted;
public:
  inline static const weight_type inf = inf_value<weight_type>();
  constexpr Vec<u32> indegree() const {
    const u32 n = derived().vertex_count();
    Vec<u32> deg(n, 0);
    for(u32 i = 0; i != n; ++i) {
      for(const auto& e : derived()[i]) ++deg[e.to()];
    }
    return deg;
  }
  constexpr u32 indegree(u32 v) const { return indegree()[v]; }
  constexpr Vec<Vec<u32>> connected_components() const {
    const u32 n = derived().vertex_count();
    UnionFind uf(n);
    for(u32 i = 0; i != n; ++i) {
      for(const auto& e : derived()[i]) uf.merge(i, e.to());
    }
    return uf.groups();
  }
  constexpr u32 count_connected_components() const { return connected_components().size(); }
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
    u32 cnt1 = 0;
    for(u32 i = 0; i != n; ++i) {
      const u32 d = derived().outdegree(i);
      if(d > 2) return false;
      cnt1 += (d == 1);
    }
    if(n == 1) return true;
    if(cnt1 != 2) return false;
    return is_connected_graph();
  }
  constexpr Vec<u32> topological_sort() const {
    const u32 n = derived().vertex_count();
    Vec<u32> indeg = indegree();
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
  template<class Comp> constexpr Vec<u32> topological_sort(const Comp& comp) const {
    const u32 n = derived().vertex_count();
    Vec<u32> indeg = indegree();
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
  constexpr auto longest_path_length_dag() const {
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
        if(dp[to] < cand) dp[to] = cand;
      }
    }
    return dp;
  }
  template<class WTT> struct ShortestPathResult {
    using weight_type = WTT;
    Vec<weight_type> dist;
    Vec<u32> prev;
    constexpr Vec<u32> restore(u32 t) const {
      Vec<u32> path;
      if(t >= prev.size()) return path;
      if(dist[t] == GraphInterface::inf) return path;
      for(u32 cur = t;; cur = prev[cur]) {
        path.push_back(cur);
        if(prev[cur] == cur) break;
      }
      path.reverse();
      return path;
    }
  };
  constexpr auto shortest_path(u32 s) const {
    if constexpr(!is_weighted) return shortest_path_bfs(s);
    else return shortest_path_dijkstra(s);
  }
  constexpr auto shortest_path_bfs(u32 s) const {
    const u32 n = derived().vertex_count();
    ShortestPathResult<weight_type> res{Vec<weight_type>(n, GraphInterface::inf), Vec<u32>(n, 0)};
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
        if(res.dist[to] != GraphInterface::inf) continue;
        res.dist[to] = res.dist[v] + static_cast<weight_type>(1);
        res.prev[to] = v;
        q.push_back(to);
      }
    }
    return res;
  }
  constexpr auto shortest_path_dijkstra(u32 s) const {
    const u32 n = derived().vertex_count();
    ShortestPathResult<weight_type> res{Vec<weight_type>(n, GraphInterface::inf), Vec<u32>(n, 0)};
    struct Node {
      weight_type d;
      u32 v;
    };
    struct Comp {
      constexpr bool operator()(const Node& a, const Node& b) const noexcept { return a.d < b.d; }
    };
    Heap<Node, Comp> pq;
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
        if(res.dist[cur.v] == GraphInterface::inf) continue;
        const weight_type nd = cur.d + w;
        if(nd < res.dist[to]) {
          res.dist[to] = nd;
          res.prev[to] = cur.v;
          pq.push(Node{nd, to});
        }
      }
    }
    return res;
  }
  constexpr auto minimum_spanning_forest_cost() const {
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
    std::sort(es.begin(), es.end(), [](const E& a, const E& b) { return a.w < b.w; });
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
} // namespace gsh
