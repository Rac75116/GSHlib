#pragma once
#include "Memory.hpp"
#include "SparseTable.hpp"
#include <ranges>
namespace gsh {
class LowestCommonAncestor {
public:
  using size_type = u32;
  static constexpr size_type npos = 0xffffffffu;
private:
  size_type n_ = 0;
  size_type root_ = 0;
  Mem<size_type> parent_;
  Mem<size_type> depth_;
  Mem<size_type> first_;
  Mem<size_type> child_start_;
  Mem<size_type> child_;
  SparseTable<sparse_table_specs::RangeMin<u64, 6>> rmq_;
  constexpr void build_children() {
    if(n_ == 0) {
      child_start_.clear();
      child_.clear();
      return;
    }
    Mem<size_type> cnt(n_, 0);
    for(size_type v = 0; v < n_; ++v) {
      if(v == root_) continue;
      const size_type p = parent_[v];
#ifndef NDEBUG
      if(p == npos || p >= n_) throw Exception("LowestCommonAncestor: invalid parent[", v, "] = ", p);
#endif
      ++cnt[p];
    }
    child_start_ = Mem<size_type>(n_ + 1, 0);
    for(size_type i = 0; i < n_; ++i) child_start_[i + 1] = child_start_[i] + cnt[i];
    child_ = Mem<size_type>(n_ - 1);
    Mem<size_type> cur = child_start_;
    for(size_type v = 0; v < n_; ++v) {
      if(v == root_) continue;
      const size_type p = parent_[v];
      child_[cur[p]++] = v;
    }
  }
  constexpr void build_euler_rmq() {
    if(n_ == 0) {
      depth_.clear();
      first_.clear();
      rmq_.clear();
      return;
    }
    depth_ = Mem<size_type>(n_, 0);
    first_ = Mem<size_type>(n_, npos);
    Mem<u64> euler(2 * n_ - 1);
    size_type euler_len = 0;
    Mem<size_type> st(n_);
    Mem<size_type> it(n_);
    size_type sp = 0;
    st[sp] = root_;
    it[sp] = child_start_[root_];
    ++sp;
    first_[root_] = 0;
    euler[euler_len++] = (static_cast<u64>(0u) << 32) | static_cast<u64>(root_);
    while(sp != 0) {
      const size_type v = st[sp - 1];
      size_type cur = it[sp - 1];
      const size_type end = child_start_[v + 1];
      if(cur < end) {
        const size_type to = child_[cur++];
        it[sp - 1] = cur;
        depth_[to] = depth_[v] + 1;
        st[sp] = to;
        it[sp] = child_start_[to];
        ++sp;
        first_[to] = euler_len;
        euler[euler_len++] = (static_cast<u64>(depth_[to]) << 32) | static_cast<u64>(to);
      } else {
        --sp;
        if(sp != 0) {
          const size_type p = st[sp - 1];
          euler[euler_len++] = (static_cast<u64>(depth_[p]) << 32) | static_cast<u64>(p);
        }
      }
    }
    rmq_.assign(euler.data(), euler.data() + euler_len);
  }
public:
  constexpr LowestCommonAncestor() = default;
  template<std::ranges::forward_range R> constexpr explicit LowestCommonAncestor(R&& parent, size_type root = 0) { assign(std::forward<R>(parent), root); }
  template<std::forward_iterator It, std::sentinel_for<It> Sent> constexpr LowestCommonAncestor(It first, Sent last, size_type root = 0) { assign(first, last, root); }
  template<std::ranges::forward_range R> constexpr void assign(R&& parent, size_type root = 0) { assign(std::ranges::begin(parent), std::ranges::end(parent), root); }
  template<std::forward_iterator It, std::sentinel_for<It> Sent> constexpr void assign(It first, Sent last, size_type root = 0) {
    const size_type n = static_cast<size_type>(std::ranges::distance(first, last));
    parent_ = Mem<size_type>(n);
    for(size_type i = 0; i < n; ++i, ++first) parent_[i] = static_cast<size_type>(*first);
    n_ = n;
    root_ = root;
#ifndef NDEBUG
    if(n_ == 0) return;
    if(root_ >= n_) throw Exception("LowestCommonAncestor: root is out of range ( root=", root_, ", n=", n_, " )");
    for(size_type v = 0; v < n_; ++v) {
      if(v == root_) continue;
      const size_type p = parent_[v];
      if(p >= n_) throw Exception("LowestCommonAncestor: parent[", v, "] is out of range ( p=", p, ", n=", n_, " )");
    }
#endif
    build_children();
    build_euler_rmq();
  }
  constexpr void clear() {
    n_ = 0;
    root_ = 0;
    parent_.clear();
    depth_.clear();
    first_.clear();
    child_start_.clear();
    child_.clear();
    rmq_.clear();
  }
  constexpr size_type size() const noexcept { return n_; }
  constexpr bool empty() const noexcept { return n_ == 0; }
  constexpr size_type root() const noexcept { return root_; }
  constexpr size_type parent(size_type v) const {
#ifndef NDEBUG
    if(v >= n_) throw Exception("LowestCommonAncestor::parent: v is out of range ( v=", v, ", n=", n_, " )");
#endif
    return parent_[v];
  }
  constexpr size_type depth(size_type v) const {
#ifndef NDEBUG
    if(v >= n_) throw Exception("LowestCommonAncestor::depth: v is out of range ( v=", v, ", n=", n_, " )");
#endif
    return depth_[v];
  }
  constexpr size_type lca(size_type a, size_type b) const {
#ifndef NDEBUG
    if(a >= n_ || b >= n_) throw Exception("LowestCommonAncestor::lca: vertex is out of range ( a=", a, ", b=", b, ", n=", n_, " )");
    if(first_.empty()) throw Exception("LowestCommonAncestor::lca: not initialized");
#endif
    size_type l = first_[a];
    size_type r = first_[b];
    if(l > r) {
      const size_type tmp = l;
      l = r;
      r = tmp;
    }
    return static_cast<size_type>(rmq_.prod(l, r + 1));
  }
  constexpr size_type dist(size_type a, size_type b) const {
    const size_type c = lca(a, b);
    return depth_[a] + depth_[b] - 2 * depth_[c];
  }
};
}
