#pragma once
#include "BitVector.hpp"
#include "Exception.hpp"
#include "Heap.hpp"
#include "IndexableDict.hpp"
#include "TypeDef.hpp"
#include "Vec.hpp"
#include <bit>
#include <concepts>
#include <initializer_list>
#include <iterator>
#include <optional>
#include <tuple>
#include <utility>
namespace gsh {
template<class T> requires std::unsigned_integral<T> class WaveletMatrix {
public:
  using value_type = T;
  using size_type = u32;
  using difference_type = i32;
private:
  u32 n_ = 0;
  u32 lg_ = 0;
  Vec<value_type> vals_; // sorted unique values
  Vec<u32> begin_; // begin_[id] = start position of value id in last level order
  Vec<IndexableDict> mat_; // bit vectors for each level (high bit -> low bit)
  Vec<u32> mid_; // mid_[level] = #zeros at that level
  constexpr void check_pos_on_debug(u32 pos) const {
#ifndef NDEBUG
    if(pos >= n_) [[unlikely]]
      throw Exception("gsh::WaveletMatrix / The index is out of range. ( pos=", pos, ", size=", n_, " )");
#else
    Assume(pos < n_);
#endif
  }
  constexpr void check_range_on_debug(u32 l, u32 r) const {
#ifndef NDEBUG
    if(l > r || r > n_) [[unlikely]]
      throw Exception("gsh::WaveletMatrix / Invalid range. ( l=", l, ", r=", r, ", size=", n_, " )");
#else
    Assume(l <= r);
    Assume(r <= n_);
#endif
  }
  constexpr u32 sigma() const noexcept { return static_cast<u32>(vals_.size()); }
  constexpr u32 lower_bound_id(const value_type& x) const noexcept { return vals_.lower_bound_index(x); }
  constexpr u32 id_of_existing(const value_type& x) const noexcept {
    const u32 id = lower_bound_id(x);
    if(id == sigma() || vals_[id] != x) return sigma();
    return id;
  }
  constexpr u32 rank_id(u32 id, u32 pos) const {
    check_pos_on_debug(pos == n_ ? (n_ - 1u) : pos); // keep Assume-friendly shape
    u32 l = 0;
    u32 r = pos;
    if(lg_ == 0) return (id < sigma() ? (r - l) : 0);
    for(u32 level = 0; level != lg_; ++level) {
      const u32 bit = (id >> (lg_ - 1u - level)) & 1u;
      const auto& bv = mat_[level];
      const u32 l1 = bv.rank1(l);
      const u32 r1 = bv.rank1(r);
      if(bit == 0) {
        l -= l1;
        r -= r1;
      } else {
        l = mid_[level] + l1;
        r = mid_[level] + r1;
      }
    }
    return r - l;
  }
  constexpr u32 rank_id_range(u32 id, u32 l, u32 r) const {
    if(l == r) return 0;
    if(lg_ == 0) return (id < sigma() ? (r - l) : 0);
    for(u32 level = 0; level != lg_; ++level) {
      const u32 bit = (id >> (lg_ - 1u - level)) & 1u;
      const auto& bv = mat_[level];
      const u32 l1 = bv.rank1(l);
      const u32 r1 = bv.rank1(r);
      if(bit == 0) {
        l -= l1;
        r -= r1;
      } else {
        l = mid_[level] + l1;
        r = mid_[level] + r1;
      }
    }
    return r - l;
  }
  constexpr u32 range_freq_id(u32 l, u32 r, u32 x, u32 y) const {
    if(l == r || x >= y) return 0;
    if(lg_ == 0) {
      // sigma is 0 or 1
      return (x == 0 && y != 0 && n_ != 0) ? (r - l) : 0;
    }
    const u64 ql = static_cast<u64>(x);
    const u64 qr = static_cast<u64>(y);
    struct Node {
      u32 level;
      u32 l;
      u32 r;
      u64 vl;
      u64 vr;
    };
    Vec<Node> st;
    st.emplace_back(0u, l, r, 0ull, (1ull << lg_));
    u32 ans = 0;
    while(!st.empty()) {
      const Node cur = st.back();
      st.pop_back();
      if(cur.l == cur.r) continue;
      if(qr <= cur.vl || cur.vr <= ql) continue;
      if(ql <= cur.vl && cur.vr <= qr) {
        ans += cur.r - cur.l;
        continue;
      }
      if(cur.level == lg_) continue;
      const u64 vm = (cur.vl + cur.vr) >> 1;
      const u32 level = cur.level;
      const auto& bv = mat_[level];
      const u32 cl1 = bv.rank1(cur.l);
      const u32 cr1 = bv.rank1(cur.r);
      const u32 l0 = cur.l - cl1;
      const u32 r0 = cur.r - cr1;
      const u32 l1 = mid_[level] + cl1;
      const u32 r1 = mid_[level] + cr1;
      // push right then left (stack -> left first)
      st.emplace_back(level + 1u, l1, r1, vm, cur.vr);
      st.emplace_back(level + 1u, l0, r0, cur.vl, vm);
    }
    return ans;
  }
  constexpr bool next_id_impl(u32 level, u32 l, u32 r, u64 vl, u64 vr, u64 ql, u64 qr, u32 x, u32 y, u32& out) const {
    if(l == r) return false;
    if(qr <= vl || vr <= ql) return false;
    if(level == lg_) {
      const u32 id = static_cast<u32>(vl);
      if(id < sigma() && x <= id && id < y) {
        out = id;
        return true;
      }
      return false;
    }
    const u64 vm = (vl + vr) >> 1;
    const auto& bv = mat_[level];
    const u32 l1p = bv.rank1(l);
    const u32 r1p = bv.rank1(r);
    const u32 l0 = l - l1p;
    const u32 r0 = r - r1p;
    const u32 l1 = mid_[level] + l1p;
    const u32 r1 = mid_[level] + r1p;
    if(next_id_impl(level + 1u, l0, r0, vl, vm, ql, qr, x, y, out)) return true;
    return next_id_impl(level + 1u, l1, r1, vm, vr, ql, qr, x, y, out);
  }
  constexpr bool prev_id_impl(u32 level, u32 l, u32 r, u64 vl, u64 vr, u64 ql, u64 qr, u32 x, u32 y, u32& out) const {
    if(l == r) return false;
    if(qr <= vl || vr <= ql) return false;
    if(level == lg_) {
      const u32 id = static_cast<u32>(vl);
      if(id < sigma() && x <= id && id < y) {
        out = id;
        return true;
      }
      return false;
    }
    const u64 vm = (vl + vr) >> 1;
    const auto& bv = mat_[level];
    const u32 l1p = bv.rank1(l);
    const u32 r1p = bv.rank1(r);
    const u32 l0 = l - l1p;
    const u32 r0 = r - r1p;
    const u32 l1 = mid_[level] + l1p;
    const u32 r1 = mid_[level] + r1p;
    if(prev_id_impl(level + 1u, l1, r1, vm, vr, ql, qr, x, y, out)) return true;
    return prev_id_impl(level + 1u, l0, r0, vl, vm, ql, qr, x, y, out);
  }
public:
  constexpr WaveletMatrix() = default;
  template<std::forward_iterator Iter, std::sentinel_for<Iter> Sent> explicit WaveletMatrix(Iter first, Sent last) { assign(first, last); }
  constexpr WaveletMatrix(std::initializer_list<value_type> init) { assign(init.begin(), init.end()); }
  template<std::forward_iterator Iter, std::sentinel_for<Iter> Sent> void assign(Iter first, Sent last) {
    const u32 n = static_cast<u32>(std::ranges::distance(first, last));
    n_ = n;
    vals_.clear();
    begin_.clear();
    mat_.clear();
    mid_.clear();
    if(n_ == 0) {
      lg_ = 0;
      return;
    }
    using Item = std::pair<value_type, u32>;
    Vec<Item> items(n_);
    for(u32 i = 0; i != n_; ++i) {
      items[i].first = *(first++);
      items[i].second = i;
    }
    items.sort({}, [](const Item& a) { return a.first; });
    Vec<u32> ids(n_);
    Vec<u32> freq;
    vals_.clear();
    vals_.reserve(n_);
    freq.reserve(n_);
    // unique + ids + freq
    vals_.push_back(items[0].first);
    freq.push_back(0u);
    u32 cur_id = 0;
    ids[items[0].second] = 0;
    ++freq[0];
    for(u32 i = 1; i != n_; ++i) {
      const value_type v = items[i].first;
      if(v != vals_.back()) {
        vals_.push_back(v);
        freq.push_back(0u);
        ++cur_id;
      }
      ids[items[i].second] = cur_id;
      ++freq[cur_id];
    }
    const u32 sig = sigma();
    lg_ = (sig <= 1) ? 0u : static_cast<u32>(std::bit_width(sig - 1u));
    begin_.resize(sig + 1u);
    begin_[0] = 0;
    for(u32 i = 0; i != sig; ++i) begin_[i + 1u] = begin_[i] + freq[i];
    if(lg_ == 0) return;
    mat_.resize(lg_);
    mid_.resize(lg_);
    Vec<u32> cur(ids);
    Vec<u32> nxt(n_);
    for(u32 level = 0; level != lg_; ++level) {
      const u32 sh = lg_ - 1u - level;
      BitVector bv(n_);
      u64* w = const_cast<u64*>(bv.data());
      u32 zeros = 0;
      for(u32 i = 0; i != n_; ++i) {
        const u32 v = cur[i];
        const bool bit = ((v >> sh) & 1u) != 0;
        if(bit) {
          w[i >> 6] |= (1ULL << (i & 63));
        } else {
          ++zeros;
        }
      }
      u32 p0 = 0;
      u32 p1 = zeros;
      for(u32 i = 0; i != n_; ++i) {
        const u32 v = cur[i];
        const bool bit = ((v >> sh) & 1u) != 0;
        if(!bit) nxt[p0++] = v;
        else nxt[p1++] = v;
      }
      mid_[level] = zeros;
      mat_[level] = IndexableDict(std::move(bv));
      cur.swap(nxt);
    }
  }
  constexpr void clear() {
    n_ = 0;
    lg_ = 0;
    vals_.clear();
    begin_.clear();
    mat_.clear();
    mid_.clear();
  }
  constexpr bool empty() const noexcept { return n_ == 0; }
  constexpr u32 size() const noexcept { return n_; }
  // access(T, i): returns original value at position i (O(b))
  constexpr value_type operator[](u32 i) const {
    check_pos_on_debug(i);
    if(lg_ == 0) return vals_.empty() ? value_type{} : vals_[0];
    u32 pos = i;
    u32 id = 0;
    for(u32 level = 0; level != lg_; ++level) {
      const auto& bv = mat_[level];
      const bool bit = bv[pos];
      const u32 p1 = bv.rank1(pos);
      if(bit) {
        id |= (1u << (lg_ - 1u - level));
        pos = mid_[level] + p1;
      } else {
        pos -= p1;
      }
    }
    return vals_[id];
  }
  // rank(c, pos): occurrences of c in [0, pos) (O(b))
  constexpr u32 rank(const value_type& c, u32 pos) const {
#ifndef NDEBUG
    if(pos > n_) { throw Exception("gsh::WaveletMatrix / The position is out of range. ( pos=", pos, ", size=", n_, " )"); }
#else
    Assume(pos <= n_);
#endif
    const u32 id = id_of_existing(c);
    if(id == sigma()) return 0;
    if(pos == 0) return 0;
    if(pos == n_) return begin_[id + 1u] - begin_[id];
    return rank_id_range(id, 0, pos);
  }
  // rank(c, l, r): occurrences of c in [l, r) (O(b))
  constexpr u32 rank(const value_type& c, u32 l, u32 r) const {
    check_range_on_debug(l, r);
    const u32 id = id_of_existing(c);
    if(id == sigma()) return 0;
    return rank_id_range(id, l, r);
  }
  // quantile(l, r, k): k-th smallest in [l, r) (0-indexed) (O(b))
  constexpr value_type quantile(u32 l, u32 r, u32 k) const {
    check_range_on_debug(l, r);
#ifndef NDEBUG
    if(k >= r - l) [[unlikely]]
      throw Exception("gsh::WaveletMatrix / quantile: k is out of range. ( k=", k, ", len=", (r - l), " )");
#endif
    if(lg_ == 0) return vals_.empty() ? value_type{} : vals_[0];
    u32 id = 0;
    for(u32 level = 0; level != lg_; ++level) {
      const auto& bv = mat_[level];
      const u32 l1 = bv.rank1(l);
      const u32 r1 = bv.rank1(r);
      const u32 l0 = l - l1;
      const u32 r0 = r - r1;
      const u32 zeros = r0 - l0;
      if(k < zeros) {
        l = l0;
        r = r0;
      } else {
        k -= zeros;
        id |= (1u << (lg_ - 1u - level));
        l = mid_[level] + l1;
        r = mid_[level] + r1;
      }
    }
    return vals_[id];
  }
  // range_freq(l, r, x, y): count of values v with x <= v < y in [l, r) (O(b))
  constexpr u32 range_freq(u32 l, u32 r, const value_type& x, const value_type& y) const {
    check_range_on_debug(l, r);
    const u32 xid = lower_bound_id(x);
    const u32 yid = lower_bound_id(y);
    if(xid >= yid) return 0;
    return range_freq_id(l, r, xid, yid);
  }
  // range_list(l, r, x, y): enumerate (value, freq) for x <= v < y in [l, r) (O(b + m), m<=N)
  constexpr Vec<std::pair<value_type, u32>> range_list(u32 l, u32 r, const value_type& x, const value_type& y) const {
    check_range_on_debug(l, r);
    Vec<std::pair<value_type, u32>> out;
    if(l == r || sigma() == 0) return out;
    const u32 xid = lower_bound_id(x);
    const u32 yid = lower_bound_id(y);
    if(xid >= yid) return out;
    if(lg_ == 0) {
      out.emplace_back(vals_[0], r - l);
      return out;
    }
    const u64 ql = static_cast<u64>(xid);
    const u64 qr = static_cast<u64>(yid);
    struct Node {
      u32 level;
      u32 l;
      u32 r;
      u64 vl;
      u64 vr;
    };
    Vec<Node> st;
    st.emplace_back(0u, l, r, 0ull, (1ull << lg_));
    while(!st.empty()) {
      const Node cur = st.back();
      st.pop_back();
      if(cur.l == cur.r) continue;
      if(qr <= cur.vl || cur.vr <= ql) continue;
      if(cur.level == lg_) {
        const u32 id = static_cast<u32>(cur.vl);
        if(id < sigma() && xid <= id && id < yid) out.emplace_back(vals_[id], cur.r - cur.l);
        continue;
      }
      const u64 vm = (cur.vl + cur.vr) >> 1;
      const u32 level = cur.level;
      const auto& bv = mat_[level];
      const u32 cl1 = bv.rank1(cur.l);
      const u32 cr1 = bv.rank1(cur.r);
      const u32 l0 = cur.l - cl1;
      const u32 r0 = cur.r - cr1;
      const u32 l1 = mid_[level] + cl1;
      const u32 r1 = mid_[level] + cr1;
      // right then left (stack -> left first)
      st.emplace_back(level + 1u, l1, r1, vm, cur.vr);
      st.emplace_back(level + 1u, l0, r0, cur.vl, vm);
    }
    return out;
  }
  // next_value(l, r, x): min v with x <= v in [l, r) (O(b))
  constexpr std::optional<value_type> next_value(u32 l, u32 r, const value_type& x) const {
    check_range_on_debug(l, r);
    if(l == r || sigma() == 0) return std::nullopt;
    const u32 xid = lower_bound_id(x);
    if(xid >= sigma()) return std::nullopt;
    if(lg_ == 0) return vals_[0];
    u32 id = 0;
    const u64 ql = static_cast<u64>(xid);
    const u64 qr = static_cast<u64>(sigma());
    if(!next_id_impl(0u, l, r, 0ull, (1ull << lg_), ql, qr, xid, sigma(), id)) return std::nullopt;
    return vals_[id];
  }
  // prev_value(l, r, x): max v with v < x in [l, r) (O(b))
  constexpr std::optional<value_type> prev_value(u32 l, u32 r, const value_type& x) const {
    check_range_on_debug(l, r);
    if(l == r || sigma() == 0) return std::nullopt;
    const u32 yid = lower_bound_id(x);
    if(yid == 0) return std::nullopt;
    if(lg_ == 0) return vals_[0];
    u32 id = 0;
    const u64 ql = 0ull;
    const u64 qr = static_cast<u64>(yid);
    if(!prev_id_impl(0u, l, r, 0ull, (1ull << lg_), ql, qr, 0u, yid, id)) return std::nullopt;
    return vals_[id];
  }
  // topk(l, r, k): return (value, freq) in descending freq (O((b + k) log N))
  constexpr Vec<std::pair<value_type, u32>> topk(u32 l, u32 r, u32 k) const {
    check_range_on_debug(l, r);
    Vec<std::pair<value_type, u32>> out;
    if(l == r || k == 0 || sigma() == 0) return out;
    if(lg_ == 0) {
      out.emplace_back(vals_[0], r - l);
      return out;
    }
    struct Node {
      u32 cnt;
      u32 level;
      u32 l;
      u32 r;
      u32 id_prefix;
    };
    struct LessCnt {
      constexpr bool operator()(const Node& a, const Node& b) const noexcept { return a.cnt < b.cnt; }
    };
    Heap<Node, LessCnt> pq;
    pq.emplace(r - l, 0u, l, r, 0u);
    while(!pq.empty() && out.size() < k) {
      const Node cur = pq.max();
      pq.pop_max();
      if(cur.cnt == 0) continue;
      if(cur.level == lg_) {
        const u32 id = cur.id_prefix;
        if(id < sigma()) out.emplace_back(vals_[id], cur.cnt);
        continue;
      }
      const u32 level = cur.level;
      const auto& bv = mat_[level];
      const u32 cl1 = bv.rank1(cur.l);
      const u32 cr1 = bv.rank1(cur.r);
      const u32 l0 = cur.l - cl1;
      const u32 r0 = cur.r - cr1;
      const u32 l1 = mid_[level] + cl1;
      const u32 r1 = mid_[level] + cr1;
      const u32 c0 = r0 - l0;
      const u32 c1 = r1 - l1;
      const u32 bit = 1u << (lg_ - 1u - level);
      if(c0) pq.emplace(c0, level + 1u, l0, r0, cur.id_prefix);
      if(c1) pq.emplace(c1, level + 1u, l1, r1, cur.id_prefix | bit);
    }
    return out;
  }
  // sum(l, r): sum of values in [l, r) (wraps in u64 if overflow) (O(b + m), m<=N)
  constexpr u64 sum(u32 l, u32 r) const {
    check_range_on_debug(l, r);
    if(l == r || sigma() == 0) return 0;
    if(lg_ == 0) return static_cast<u64>(vals_[0]) * static_cast<u64>(r - l);
    struct Node {
      u32 level;
      u32 l;
      u32 r;
      u64 vl;
      u64 vr;
    };
    Vec<Node> st;
    st.emplace_back(0u, l, r, 0ull, (1ull << lg_));
    u64 ans = 0;
    while(!st.empty()) {
      const Node cur = st.back();
      st.pop_back();
      if(cur.l == cur.r) continue;
      if(cur.level == lg_) {
        const u32 id = static_cast<u32>(cur.vl);
        if(id < sigma()) ans += static_cast<u64>(vals_[id]) * static_cast<u64>(cur.r - cur.l);
        continue;
      }
      const u64 vm = (cur.vl + cur.vr) >> 1;
      const u32 level = cur.level;
      const auto& bv = mat_[level];
      const u32 cl1 = bv.rank1(cur.l);
      const u32 cr1 = bv.rank1(cur.r);
      const u32 l0 = cur.l - cl1;
      const u32 r0 = cur.r - cr1;
      const u32 l1 = mid_[level] + cl1;
      const u32 r1 = mid_[level] + cr1;
      // right then left
      st.emplace_back(level + 1u, l1, r1, vm, cur.vr);
      st.emplace_back(level + 1u, l0, r0, cur.vl, vm);
    }
    return ans;
  }
  // intersect([l1, r1), [l2, r2)): return (value, freq1, freq2) (O(b + m), m<=N)
  constexpr Vec<std::tuple<value_type, u32, u32>> intersect(u32 l1, u32 r1, u32 l2, u32 r2) const {
    check_range_on_debug(l1, r1);
    check_range_on_debug(l2, r2);
    Vec<std::tuple<value_type, u32, u32>> out;
    if(l1 == r1 || l2 == r2 || sigma() == 0) return out;
    if(lg_ == 0) {
      out.emplace_back(vals_[0], r1 - l1, r2 - l2);
      return out;
    }
    struct Node {
      u32 level;
      u32 l1;
      u32 r1;
      u32 l2;
      u32 r2;
      u32 id_prefix;
    };
    Vec<Node> st;
    st.emplace_back(0u, l1, r1, l2, r2, 0u);
    while(!st.empty()) {
      const Node cur = st.back();
      st.pop_back();
      if(cur.l1 == cur.r1 || cur.l2 == cur.r2) continue;
      if(cur.level == lg_) {
        const u32 id = cur.id_prefix;
        if(id < sigma()) out.emplace_back(vals_[id], cur.r1 - cur.l1, cur.r2 - cur.l2);
        continue;
      }
      const u32 level = cur.level;
      const u32 bit = 1u << (lg_ - 1u - level);
      const auto& bv = mat_[level];
      const u32 a_l1 = bv.rank1(cur.l1);
      const u32 a_r1 = bv.rank1(cur.r1);
      const u32 b_l1 = bv.rank1(cur.l2);
      const u32 b_r1 = bv.rank1(cur.r2);
      const u32 a00 = cur.l1 - a_l1;
      const u32 a01 = cur.r1 - a_r1;
      const u32 a10 = mid_[level] + a_l1;
      const u32 a11 = mid_[level] + a_r1;
      const u32 b00 = cur.l2 - b_l1;
      const u32 b01 = cur.r2 - b_r1;
      const u32 b10 = mid_[level] + b_l1;
      const u32 b11 = mid_[level] + b_r1;
      // right then left (so left processed first)
      if((a11 - a10) && (b11 - b10)) st.emplace_back(level + 1u, a10, a11, b10, b11, cur.id_prefix | bit);
      if((a01 - a00) && (b01 - b00)) st.emplace_back(level + 1u, a00, a01, b00, b01, cur.id_prefix);
    }
    return out;
  }
};
}
