#pragma once
#include "Memory.hpp"
#include "Vec.hpp"
#include <concepts>
#include <ranges>
namespace gsh {
class FunctionalGraph {
public:
  using size_type = u32;
  static constexpr size_type npos = 0xffffffffu;
private:
  static constexpr u32 log_ = 64;
  size_type n_ = 0;
  Mem<size_type> to_;
  Mem<size_type> comp_;
  Mem<size_type> entry_;
  Mem<size_type> depth_;
  Mem<size_type> cycle_pos_;
  Mem<size_type> cycle_len_;
  Mem<size_type> up_;
  constexpr size_type up_at(const u32 k, const size_type v) const noexcept { return up_[k * n_ + v]; }
  constexpr size_type& up_at(const u32 k, const size_type v) noexcept { return up_[k * n_ + v]; }
  template<class E> static constexpr size_type edge_to(const E& e) {
    if constexpr(requires { e.to(); }) return static_cast<size_type>(e.to());
    else return static_cast<size_type>(e);
  }
  template<u32 K> constexpr size_type jump_impl(size_type v, const u64 step) const {
    if constexpr(K == log_) {
      return v;
    } else {
      if(step & (1ull << K)) v = up_at(K, v);
      return jump_impl<K + 1>(v, step);
    }
  }
  constexpr void build_tables() {
    if(n_ == 0) {
      comp_.clear();
      entry_.clear();
      depth_.clear();
      cycle_pos_.clear();
      cycle_len_.clear();
      up_.clear();
      return;
    }
    Mem<size_type> indeg(n_, 0);
    for(size_type v = 0; v < n_; ++v) ++indeg[to_[v]];
    // 0: cycle, 1: removed
    Mem<u8> removed(n_, 0);
    Vec<size_type> q;
    q.reserve(n_);
    for(size_type v = 0; v < n_; ++v)
      if(indeg[v] == 0) q.push_back(v);
    Vec<size_type> order;
    order.reserve(n_);
    for(size_type head = 0; head < q.size(); ++head) {
      const size_type v = q[head];
      removed[v] = 1;
      order.push_back(v);
      const size_type u = to_[v];
      const size_type nxt = indeg[u] - 1;
      indeg[u] = nxt;
      if(nxt == 0) q.push_back(u);
    }
    comp_ = Mem<size_type>(n_, npos);
    entry_ = Mem<size_type>(n_, npos);
    depth_ = Mem<size_type>(n_, 0);
    cycle_pos_ = Mem<size_type>(n_, npos);
    Vec<size_type> cycle_len_vec;
    // assign cycle nodes
    size_type comp_cnt = 0;
    for(size_type v = 0; v < n_; ++v) {
      if(removed[v]) continue;
      if(comp_[v] != npos) continue;
      Vec<size_type> cyc;
      size_type cur = v;
      do {
        cyc.push_back(cur);
        cur = to_[cur];
      } while(cur != v);
      const size_type len = cyc.size();
      for(size_type i = 0; i < len; ++i) {
        const size_type x = cyc[i];
        comp_[x] = comp_cnt;
        entry_[x] = x;
        depth_[x] = 0;
        cycle_pos_[x] = i;
      }
      cycle_len_vec.push_back(len);
      ++comp_cnt;
    }
    cycle_len_ = Mem<size_type>(comp_cnt);
    for(size_type i = 0; i < comp_cnt; ++i) cycle_len_[i] = cycle_len_vec[i];
    // assign non-cycle nodes in reverse topological order
    for(size_type idx = order.size(); idx != 0; --idx) {
      const size_type v = order[idx - 1];
      const size_type u = to_[v];
      comp_[v] = comp_[u];
      entry_[v] = entry_[u];
      depth_[v] = depth_[u] + 1;
    }
    // doubling table
    up_ = Mem<size_type>(n_ * log_);
    for(size_type v = 0; v < n_; ++v) up_at(0, v) = to_[v];
    for(u32 k = 1; k < log_; ++k) {
      for(size_type v = 0; v < n_; ++v) { up_at(k, v) = up_at(k - 1, up_at(k - 1, v)); }
    }
  }
public:
  constexpr FunctionalGraph() = default;
  template<std::ranges::forward_range R> constexpr explicit FunctionalGraph(R&& successor) { assign_successor(std::forward<R>(successor)); }
  template<std::forward_iterator It, std::sentinel_for<It> Sent> constexpr FunctionalGraph(It first, Sent last) { assign_successor(first, last); }
  // successor[v] = next vertex
  template<std::ranges::forward_range R> requires std::convertible_to<std::ranges::range_reference_t<R>, size_type> constexpr void assign_successor(R&& successor) { assign_successor(std::ranges::begin(successor), std::ranges::end(successor)); }
  template<std::forward_iterator It, std::sentinel_for<It> Sent> requires std::convertible_to<std::iter_reference_t<It>, size_type> constexpr void assign_successor(It first, Sent last) {
    const size_type n = static_cast<size_type>(std::ranges::distance(first, last));
    n_ = n;
    to_ = Mem<size_type>(n_);
    for(size_type i = 0; i < n_; ++i, ++first) to_[i] = static_cast<size_type>(*first);
#ifndef NDEBUG
    for(size_type v = 0; v < n_; ++v) {
      const size_type u = to_[v];
      if(u >= n_) throw Exception("FunctionalGraph: successor[", v, "] is out of range ( to=", u, ", n=", n_, " )");
    }
#endif
    build_tables();
  }
  // out_edges[v] is a range with exactly one outgoing edge
  template<std::ranges::forward_range G> requires (!std::convertible_to<std::ranges::range_reference_t<G>, size_type>) && std::ranges::range<std::ranges::range_reference_t<G>> constexpr void assign_out_edges(G&& out_edges) {
    const size_type n = static_cast<size_type>(std::ranges::distance(std::ranges::begin(out_edges), std::ranges::end(out_edges)));
    n_ = n;
    to_ = Mem<size_type>(n_);
    size_type v = 0;
    for(auto&& edges : out_edges) {
      auto it = std::ranges::begin(edges);
      auto ed = std::ranges::end(edges);
#ifndef NDEBUG
      if(it == ed) throw Exception("FunctionalGraph: out_edges[", v, "] is empty");
#endif
      to_[v] = edge_to(*it);
#ifndef NDEBUG
      {
        auto it2 = it;
        ++it2;
        if(it2 != ed) throw Exception("FunctionalGraph: out_edges[", v, "] has multiple outgoing edges");
      }
      if(to_[v] >= n_) throw Exception("FunctionalGraph: out_edges[", v, "] points out of range ( to=", to_[v], ", n=", n_, " )");
#endif
      ++v;
    }
    build_tables();
  }
  constexpr void clear() {
    n_ = 0;
    to_.clear();
    comp_.clear();
    entry_.clear();
    depth_.clear();
    cycle_pos_.clear();
    cycle_len_.clear();
    up_.clear();
  }
  constexpr size_type size() const noexcept { return n_; }
  constexpr bool empty() const noexcept { return n_ == 0; }
  constexpr size_type next(const size_type v) const {
#ifndef NDEBUG
    if(v >= n_) throw Exception("FunctionalGraph::next: v is out of range ( v=", v, ", n=", n_, " )");
    if(to_.empty()) throw Exception("FunctionalGraph::next: not initialized");
#endif
    return to_[v];
  }
  // vertex after `step` transitions
  constexpr size_type jump(const size_type v, const u64 step) const {
#ifndef NDEBUG
    if(v >= n_) throw Exception("FunctionalGraph::jump: v is out of range ( v=", v, ", n=", n_, " )");
    if(up_.empty()) throw Exception("FunctionalGraph::jump: not initialized");
#endif
    return jump_impl<0>(v, step);
  }
  constexpr bool on_cycle(const size_type v) const {
#ifndef NDEBUG
    if(v >= n_) throw Exception("FunctionalGraph::on_cycle: v is out of range ( v=", v, ", n=", n_, " )");
    if(depth_.empty()) throw Exception("FunctionalGraph::on_cycle: not initialized");
#endif
    return depth_[v] == 0;
  }
  constexpr size_type component_id(const size_type v) const {
#ifndef NDEBUG
    if(v >= n_) throw Exception("FunctionalGraph::component_id: v is out of range ( v=", v, ", n=", n_, " )");
    if(comp_.empty()) throw Exception("FunctionalGraph::component_id: not initialized");
#endif
    return comp_[v];
  }
  constexpr size_type cycle_entry(const size_type v) const {
#ifndef NDEBUG
    if(v >= n_) throw Exception("FunctionalGraph::cycle_entry: v is out of range ( v=", v, ", n=", n_, " )");
    if(entry_.empty()) throw Exception("FunctionalGraph::cycle_entry: not initialized");
#endif
    return entry_[v];
  }
  constexpr size_type depth_to_cycle(const size_type v) const {
#ifndef NDEBUG
    if(v >= n_) throw Exception("FunctionalGraph::depth_to_cycle: v is out of range ( v=", v, ", n=", n_, " )");
    if(depth_.empty()) throw Exception("FunctionalGraph::depth_to_cycle: not initialized");
#endif
    return depth_[v];
  }
  constexpr size_type cycle_length(const size_type v) const {
    const size_type cid = component_id(v);
#ifndef NDEBUG
    if(cid == npos) throw Exception("FunctionalGraph::cycle_length: invalid component id");
    if(cycle_len_.empty()) throw Exception("FunctionalGraph::cycle_length: not initialized");
#endif
    return cycle_len_[cid];
  }
  // directed distance along successor edges; returns npos if unreachable
  constexpr size_type dist(const size_type a, const size_type b) const {
#ifndef NDEBUG
    if(a >= n_ || b >= n_) throw Exception("FunctionalGraph::dist: vertex is out of range ( a=", a, ", b=", b, ", n=", n_, " )");
    if(depth_.empty()) throw Exception("FunctionalGraph::dist: not initialized");
#endif
    if(comp_[a] != comp_[b]) return npos;
    const size_type da = depth_[a];
    const size_type db = depth_[b];
    if(da >= db) {
      const size_type x = jump(a, static_cast<u64>(da - db));
      if(x == b) return da - db;
    }
    if(db != 0) return npos;
    const size_type e = entry_[a];
    const size_type cid = comp_[a];
    const size_type len = cycle_len_[cid];
    const size_type pe = cycle_pos_[e];
    const size_type pb = cycle_pos_[b];
    const size_type cyc = (pb + len - pe) % len;
    return da + cyc;
  }
  // count visits of vertex m in the sequence f^0(v), f^1(v), ..., f^step(v)
  constexpr u64 visit_count_inclusive(const size_type v, const u64 step, const size_type m) const {
#ifndef NDEBUG
    if(v >= n_ || m >= n_) throw Exception("FunctionalGraph::visit_count_inclusive: vertex is out of range ( v=", v, ", m=", m, ", n=", n_, " )");
    if(depth_.empty()) throw Exception("FunctionalGraph::visit_count_inclusive: not initialized");
#endif
    const size_type d = dist(v, m);
    if(d == npos) return 0;
    if(static_cast<u64>(d) > step) return 0;
    if(depth_[m] != 0) return 1;
    const size_type len = cycle_len_[comp_[m]];
    return 1 + (step - static_cast<u64>(d)) / static_cast<u64>(len);
  }
  // count visits of vertex m in the sequence f^1(v), f^2(v), ..., f^step(v)
  constexpr u64 visit_count_exclusive(const size_type v, const u64 step, const size_type m) const {
    if(step == 0) return 0;
    const u64 all = visit_count_inclusive(v, step, m);
    if(v == m) return all - 1;
    return all;
  }
};
}
