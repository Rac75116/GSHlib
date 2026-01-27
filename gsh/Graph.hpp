#pragma once
#include "Algorithm.hpp"
#include "Exception.hpp"
#include "Heap.hpp"
#include "Memory.hpp"
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
template<std::size_t M, class W> decltype(auto) get(const Edge<W>& e) {
  static_assert(M <= 1, "gsh::get(gsh::Edge) / The index is out of range.");
  if constexpr(M == 0) return e.to();
  else return e.weight();
}
template<std::size_t M, class W> decltype(auto) get(Edge<W>& e) {
  static_assert(M <= 1, "gsh::get(gsh::Edge) / The index is out of range.");
  if constexpr(M == 0) return e.to();
  else return e.weight();
}
template<std::size_t M, class W> decltype(auto) get(Edge<W>&& e) {
  static_assert(M <= 1, "gsh::get(gsh::Edge) / The index is out of range.");
  if constexpr(M == 0) return e.to();
  else return std::move(e.weight());
}
}
namespace std {
template<class W> struct tuple_size<gsh::Edge<W>> : integral_constant<std::size_t, 2> {};
template<class W> struct tuple_size<const gsh::Edge<W>> : integral_constant<std::size_t, 2> {};
template<std::size_t M, class W> requires (M < 2) struct tuple_element<M, gsh::Edge<W>> {
  using type = std::conditional_t<M == 0, gsh::u32, W>;
};
template<std::size_t M, class W> requires (M < 2) struct tuple_element<M, const gsh::Edge<W>> {
  using type = std::conditional_t<M == 0, const gsh::u32, const W>;
};
}
namespace gsh {
namespace internal {
template<class W, bool IsConst> class AdjacencyList : public ViewInterface<AdjacencyList<W, IsConst>, Edge<W>> {
  constexpr static u32 npos = 0xffffffffu;
  using storage_type = Vec<std::pair<Edge<W>, u32>>;
  using storage_ptr_type = std::conditional_t<IsConst, typename storage_type::const_iterator, typename storage_type::iterator>;
  storage_ptr_type storage_ptr;
  u32 idx;
  template<class W2> friend class CRS;
  constexpr AdjacencyList(storage_ptr_type p, u32 i) : storage_ptr(p), idx(i) {}
  class sentinel_impl {};
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
    friend constexpr bool operator==(const iterator_impl& a, const sentinel_impl&) noexcept { return a.current_idx == npos; }
  };
public:
  using iterator = iterator_impl<IsConst>;
  using sentinel = sentinel_impl;
  using const_iterator = iterator_impl<true>;
  using const_sentinel = sentinel_impl;
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
  constexpr sentinel end() noexcept { return {}; };
  constexpr const_iterator begin() const noexcept { return const_iterator(storage_ptr, idx); }
  constexpr const_sentinel end() const noexcept { return {}; }
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
template<class WTT> class ShortestPathResult {
  template<class D, class W> friend class GraphInterface;
  constexpr static u32 npos = 0xffffffff;
  using weight_type = WTT;
  Vec<weight_type> dist_;
  Vec<u32> prev_;
  constexpr ShortestPathResult(const weight_type& inf, u32 n) : dist_(n, inf), prev_(n, npos), inf(inf) {}
public:
  const weight_type inf;
  constexpr ShortestPathResult(const ShortestPathResult&) = default;
  constexpr ShortestPathResult(ShortestPathResult&&) = default;
  constexpr const weight_type& dist(u32 t) const { return dist_[t]; }
  constexpr u32 prev(u32 t) const { return prev_[t]; }
  constexpr bool is_reachable(u32 t) const { return dist_[t] != inf; }
  constexpr Vec<u32> path(u32 t) const {
    u32 len = 0;
    for(u32 cur = t; cur != npos; cur = prev_[cur]) ++len;
    Vec<u32> path(len);
    for(u32 cur = t, i = 0; cur != npos; cur = prev_[cur]) path[i++] = cur;
    path.reverse();
    return path;
  }
};
class ConnectedComponents {
  u32 n_ = 0;
  u32 comp_cnt_ = 0;
  Mem<u32> comp_id_, comp_start_, comp_size_, comp_list_;
public:
  constexpr ConnectedComponents() = default;
  constexpr ConnectedComponents(u32 n, u32 comp_cnt, Mem<u32>&& comp_id, Mem<u32>&& comp_start, Mem<u32>&& comp_size, Mem<u32>&& comp_list) : n_(n), comp_cnt_(comp_cnt), comp_id_(std::move(comp_id)), comp_start_(std::move(comp_start)), comp_size_(std::move(comp_size)), comp_list_(std::move(comp_list)) {}
  constexpr u32 vertex_count() const noexcept { return n_; }
  constexpr u32 size() const noexcept { return comp_cnt_; }
  constexpr u32 id(u32 v) const noexcept { return comp_id_[v]; }
  constexpr auto operator[](u32 cid) const noexcept {
    auto p = comp_list_.data() + comp_start_[cid];
    return Subrange(p, p + comp_size_[cid]);
  }
  constexpr Vec<Vec<u32>> groups() const {
    Vec<Vec<u32>> res(comp_cnt_);
    for(u32 cid = 0; cid != comp_cnt_; ++cid) res[cid].assign_range((*this)[cid]);
    return res;
  }
};
template<class D, class W> class GraphInterface {
  constexpr D& derived() noexcept { return *static_cast<D*>(this); }
  constexpr const D& derived() const noexcept { return *static_cast<const D*>(this); }
public:
  using edge_type = Edge<W>;
  using weight_type = typename edge_type::weight_type;
  constexpr static bool is_weighted = edge_type::is_weighted;
  template<class W2> constexpr static W2 default_inf() { return std::numeric_limits<W2>::max(); }
  constexpr auto shortest_path_bfs(u32 s, u32 t = 0xffffffff) const {
    const u32 n = derived().vertex_count();
    constexpr u32 inf = std::numeric_limits<u32>::max();
    ShortestPathResult<u32> res(inf, n);
    Vec<u32> q(n);
    u32 head = 0;
    u32 tail = 0;
    res.dist_[s] = 0;
    q[tail++] = s;
    while(head != tail) {
      const u32 v = q[head++];
      for(const auto& e : derived()[v]) {
        const u32 to = e.to();
        if(res.dist_[to] != inf) continue;
        res.dist_[to] = res.dist_[v] + 1u;
        res.prev_[to] = v;
        q[tail++] = to;
        if(to == t) {
          tail = head;
          break;
        }
      }
    }
    return res;
  }
  constexpr auto shortest_path_01bfs(u32 s, u32 t = 0xffffffff) const {
    const u32 n = derived().vertex_count();
    constexpr u32 inf = std::numeric_limits<u32>::max();
    ShortestPathResult<u32> res(inf, n);
    Vec<u8> used(n, 0);
    Vec<u32> dq(2 * n);
    u32 front = n;
    u32 back = n;
    res.dist_[s] = 0;
    dq[back++] = s;
    while(front != back) {
      const u32 v = dq[front++];
      if(used[v]) continue;
      if(v == t) break;
      used[v] = 1;
      for(const auto& e : derived()[v]) {
        const u32 to = e.to();
        const bool w = static_cast<bool>(e.weight());
        const u32 nd = res.dist_[v] + w;
        if(nd < res.dist_[to]) {
          res.dist_[to] = nd;
          res.prev_[to] = v;
          if(w) dq[back++] = to;
          else dq[--front] = to;
        }
      }
    }
    return res;
  }
  template<class W2 = weight_type> constexpr auto shortest_path_dijkstra(u32 s, u32 t = 0xffffffff) const {
    const u32 n = derived().vertex_count();
    const auto inf = std::numeric_limits<W2>::max();
    ShortestPathResult<W2> res(inf, n);
    Heap<std::pair<W2, u32>, decltype([](const auto& a, const auto& b) { return a.first < b.first; })> pq;
    pq.reserve(derived().edge_count());
    res.dist_[s] = W2{};
    pq.emplace(W2{}, s);
    while(!pq.empty()) {
      auto [d, v] = pq.min();
      pq.pop_min();
      if(v == t) break;
      if(d != res.dist_[v]) continue;
      for(const auto& e : derived()[v]) {
        const u32 to = e.to();
        const W2 w = static_cast<W2>(e.weight());
        const W2 nd = d + w;
        if(nd < res.dist_[to]) {
          res.dist_[to] = nd;
          res.prev_[to] = v;
          pq.emplace(nd, to);
        }
      }
    }
    return res;
  }
  template<class W2 = weight_type> constexpr ShortestPathResult<W2> shortest_path_bellman_ford(u32 s) const {
    const auto& g = derived();
    const u32 n = g.vertex_count();
    static_assert(std::numeric_limits<W2>::is_signed, "gsh::GraphInterface::shortest_path_bellman_ford / W2 must be signed.");
    const W2 inf = std::numeric_limits<W2>::max();
    const W2 neg_inf = std::numeric_limits<W2>::lowest();
    constexpr u32 npos = 0xffffffffu;
    ShortestPathResult<W2> res(inf, n);
    if(n == 0) return res;
    auto& dist = res.dist_;
    auto& prev = res.prev_;
    dist[s] = W2{};
    Vec<u8> inq(n, 0);
    Vec<u32> push_cnt(n, 0);
    Vec<u8> neg(n, 0);
    const u32 cap = n + 1;
    Vec<u32> q(cap);
    u32 head = 0;
    u32 tail = 0;
    u32 sz = 0;
    auto pop_front = [&]() constexpr {
      const u32 v = q[head];
      head = (head + 1 == cap) ? 0 : (head + 1);
      --sz;
      return v;
    };
    auto push_back = [&](u32 v) constexpr {
      q[tail] = v;
      tail = (tail + 1 == cap) ? 0 : (tail + 1);
      ++sz;
    };
    auto push_front = [&](u32 v) constexpr {
      head = (head == 0) ? (cap - 1) : (head - 1);
      q[head] = v;
      ++sz;
    };
    auto front = [&]() constexpr { return q[head]; };
    Vec<u32> negq(n);
    u32 nhead = 0;
    u32 ntail = 0;
    auto neg_push = [&](u32 v) constexpr { negq[ntail++] = v; };
    auto drain_neg = [&]() constexpr {
      while(nhead != ntail) {
        const u32 v = negq[nhead++];
        for(const auto& e : g[v]) {
          const u32 to = e.to();
          if(neg[to]) continue;
          neg[to] = 1;
          dist[to] = neg_inf;
          prev[to] = v;
          neg_push(to);
        }
      }
    };
    push_back(s);
    inq[s] = 1;
    push_cnt[s] = 1;
    Vec<u8> in_cycle(n, 0);
    Vec<u32> cycle_nodes;
    cycle_nodes.reserve(4);
    while(sz != 0) {
      const u32 v = pop_front();
      inq[v] = 0;
      if(neg[v]) continue;
      if(dist[v] == inf || dist[v] == neg_inf) continue;
      for(const auto& e : g[v]) {
        const u32 to = e.to();
        if(neg[to]) continue;
        const W2 w = static_cast<W2>(e.weight());
        const W2 nd = dist[v] + w;
        if(nd < dist[to]) {
          dist[to] = nd;
          prev[to] = v;
          if(!inq[to]) {
            if(++push_cnt[to] >= n) [[unlikely]] {
              u32 x = to;
              for(u32 i = 0; i != n; ++i) {
                const u32 p = prev[x];
                if(p == npos) break;
                x = p;
              }
              if(prev[x] != npos) {
                u32 cur = x;
                while(!in_cycle[cur]) {
                  in_cycle[cur] = 1;
                  cycle_nodes.push_back(cur);
                  const u32 p = prev[cur];
                  if(p == npos) break;
                  cur = p;
                }
              }
              if(!cycle_nodes.empty()) {
                for(u32 u : cycle_nodes) {
                  if(neg[u]) continue;
                  neg[u] = 1;
                  dist[u] = neg_inf;
                  neg_push(u);
                }
              } else {
                neg[to] = 1;
                dist[to] = neg_inf;
                prev[to] = to;
                neg_push(to);
              }
              drain_neg();
            } else {
              if(sz != 0 && nd < dist[front()]) push_front(to);
              else push_back(to);
              inq[to] = 1;
            }
          }
        }
      }
    }
    if(ntail == 0) return res;
    if(!cycle_nodes.empty()) {
      for(u32 u : cycle_nodes) {
        const u32 p = prev[u];
        if(p == npos || !in_cycle[p]) prev[u] = u;
      }
    }
    drain_neg();
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
  constexpr bool is_dag() const { return derived().vertex_count() == 0 || !topological_sort().empty(); }
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
    if(res.size() != n) return {};
    return res;
  }
  template<class Comp = Less> constexpr Vec<u32> minimum_topological_sort(Comp comp = Comp()) const {
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
    if(res.size() != n) return {};
    return res;
  }
  constexpr ConnectedComponents strongly_connected_components() const {
    const u32 n = derived().vertex_count();
    const u32 m = derived().edge_count();
    if(n == 0) return {};
    constexpr u32 npos = 0xffffffffu;
    Mem<u32> head(n + 1), to(m), dfn(n, 0), low(n, 0), parent(n, npos), stk(n);
    Mem<u8> in_stk(n, 0);
    Mem<u32> comp_id(n), comp_list(n), comp_start(n), comp_size(n);
    Mem<std::pair<u32, u32>> dfs_buf(n);
    head[0] = std::ranges::size(derived()[0]);
    for(u32 u = 1; u != n; ++u) head[u] = head[u - 1] + std::ranges::size(derived()[u]);
    head[n] = head[n - 1];
    for(u32 u = 0; u != n; ++u) {
      for(const auto& e : derived()[u]) {
        const u32 v = e.to();
        to[--head[u]] = v;
      }
    }
    u32 write_ptr = n;
    u32 sp = 0;
    u32 comp_cnt = 0;
    std::pair<u32, u32>* dfs;
    u32 timer = 0;
    for(u32 s = 0; s != n; ++s) {
      if(dfn[s]) continue;
      parent[s] = npos;
      dfs = dfs_buf.data();
      *(dfs++) = {s, head[s]};
      dfn[s] = low[s] = ++timer;
      stk[sp++] = s;
      in_stk[s] = 1;
      while(dfs != dfs_buf.data()) {
        u32 u = (dfs - 1)->first;
        u32& ei = (dfs - 1)->second;
        u32 end = head[u + 1];
        if(ei < end) {
          u32 v = to[ei++];
          u32 vdfn = dfn[v];
          if(vdfn == 0) {
            parent[v] = u;
            dfs->first = v;
            dfs->second = head[v];
            ++dfs;
            dfn[v] = low[v] = ++timer;
            stk[sp++] = v;
            in_stk[v] = 1;
          } else if(in_stk[v]) {
            if(vdfn < low[u]) low[u] = vdfn;
          }
          continue;
        }
        --dfs;
        u32 p = parent[u];
        u32 lowu = low[u];
        if(p != npos && lowu < low[p]) low[p] = lowu;
        if(lowu == dfn[u]) {
          u32 sz = 0;
          while(true) {
            const u32 v = stk[--sp];
            in_stk[v] = 0;
            comp_id[v] = comp_cnt;
            comp_list[--write_ptr] = v;
            ++sz;
            if(v == u) break;
          }
          comp_size[comp_cnt] = sz;
          comp_start[comp_cnt] = write_ptr;
          ++comp_cnt;
        }
      }
    }
    for(u32 v = 0; v != n; ++v) comp_id[v] = (comp_cnt - 1) - comp_id[v];
    std::ranges::reverse(comp_size.data(), comp_size.data() + comp_cnt);
    std::ranges::reverse(comp_start.data(), comp_start.data() + comp_cnt);
    return {n, comp_cnt, std::move(comp_id), std::move(comp_start), std::move(comp_size), std::move(comp_list)};
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
    Vec<i8> col(n, -1);
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
            return {};
          }
        }
      }
    }
    Vec<bool> res(n);
    for(u32 i = 0; i != n; ++i) res[i] = static_cast<bool>(col[i]);
    return res;
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
