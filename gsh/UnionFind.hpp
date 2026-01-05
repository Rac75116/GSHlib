#pragma once
#include "Exception.hpp"
#include "Functional.hpp"
#include "TypeDef.hpp"
#include "Vec.hpp"
namespace gsh {
namespace internal {
template<class D> class UnionFindImpl {
  D& derived() noexcept { return static_cast<D&>(*this); }
  const D& derived() const noexcept { return static_cast<const D&>(*this); }
public:
  constexpr u32 size() const noexcept { return derived().parent.size(); }
  [[nodiscard]] constexpr bool empty() const noexcept { return derived().parent.empty(); }
  constexpr u32 leader(u32 n) {
#ifndef NDEBUG
    if(n >= derived().parent.size()) throw gsh::Exception("gsh::internal::UnionFindImpl::leader / The index is out of range. ( n=", n, ", size=", derived().parent.size(), " )");
#endif
    return derived().root(n);
  }
  constexpr bool is_leader(u32 n) const {
#ifndef NDEBUG
    if(n >= derived().parent.size()) throw gsh::Exception("gsh::internal::UnionFindImpl::is_leader / The index is out of range. ( n=", n, ", size=", derived().parent.size(), " )");
#endif
    return derived().parent[n] < 0;
  }
  constexpr bool same(u32 a, u32 b) {
#ifndef NDEBUG
    if(a >= derived().parent.size() || b >= derived().parent.size()) throw gsh::Exception("gsh::UnionFind::same / The index is out of range. ( a=", a, ", b=", b, ", size=", derived().parent.size(), " )");
#endif
    return derived().root(a) == derived().root(b);
  }
protected:
  GSH_INTERNAL_INLINE constexpr u32 merge_impl(u32 ar, u32 br) noexcept {
    const i32 sa = derived().parent[ar], sb = derived().parent[br];
    const i32 ss = sa + sb;
    const i32 tmp1 = sa < sb ? ar : br;
    const i32 tmp2 = sa < sb ? br : ar;
    --derived().cnt;
    derived().parent[tmp1] = ss;
    derived().parent[tmp2] = tmp1;
    return tmp1;
  }
public:
  constexpr u32 group_size(u32 n) {
#ifndef NDEBUG
    if(n >= derived().parent.size()) throw gsh::Exception("gsh::internal::UnionFindImpl::group_size(u32) / The index is out of range. ( n=", n, ", size=", derived().parent.size(), " )");
#endif
    return -derived().parent[derived().root(n)];
  }
  constexpr u32 max_group_size() const noexcept {
    i32 res = 1;
    for(u32 i = 0; i != size(); ++i) { res = -derived().parent[i] < res ? res : -derived().parent[i]; }
    return res;
  }
  constexpr u32 count_groups() const noexcept { return derived().cnt; }
  constexpr Vec<u32> extract(u32 n) {
#ifndef NDEBUG
    if(n >= derived().parent.size()) throw gsh::Exception("gsh::internal::UnionFindImpl::extract / The index is out of range. ( n=", n, ", size=", derived().parent.size(), " )");
#endif
    const i32 nr = derived().root(n);
    u32 ccnt = 0;
    for(u32 i = 0; i != derived().parent.size(); ++i) ccnt += derived().root(i) == nr;
    Vec<u32> res(ccnt);
    for(u32 i = 0, j = 0; i != derived().parent.size(); ++i)
      if(i == static_cast<u32>(nr) || derived().parent[i] == nr) res[j++] = i;
    return res;
  }
  constexpr Vec<Vec<u32>> groups() {
    Vec<u32> key(derived().parent.size());
    u32 cnt = 0;
    for(u32 i = 0; i != derived().parent.size(); ++i) {
      if(derived().parent[i] < 0) key[i] = cnt++;
    }
    Vec<u32> cnt2(cnt);
    for(u32 i = 0; i != derived().parent.size(); ++i) ++cnt2[key[derived().root(i)]];
    Vec<Vec<u32>> res(cnt);
    for(u32 i = 0; i != cnt; ++i) {
      res[i].resize(cnt2[i]);
      cnt2[i] = 0;
    }
    for(u32 i = 0; i != derived().parent.size(); ++i) {
      const u32 idx = key[derived().parent[i] < 0 ? i : derived().parent[i]];
      res[idx][cnt2[idx]++] = i;
    }
    return res;
  }
};
} // namespace internal
class UnionFind : public internal::UnionFindImpl<UnionFind> {
  friend class internal::UnionFindImpl<UnionFind>;
  Vec<i32> parent;
  u32 cnt = 0;
  constexpr i32 rootimpl(i32 n) noexcept {
    if(parent[n] < 0) return n;
    i32 r = rootimpl(parent[n]);
    parent[n] = r;
    return r;
  }
  GSH_INTERNAL_INLINE constexpr i32 root(i32 n) noexcept {
    if(parent[n] < 0) return n;
    i32 m = parent[n];
    if(parent[m] < 0) return parent[n] = m, m;
    i32 r = rootimpl(parent[m]);
    return parent[n] = r, parent[m] = r, r;
  }
public:
  using size_type = u32;
  constexpr UnionFind() noexcept {}
  constexpr explicit UnionFind(u32 n) : parent(n, -1), cnt(n) {}
  constexpr void reset() noexcept {
    cnt = parent.size();
    for(u32 i = 0; i != cnt; ++i) parent[i] = -1;
  }
  constexpr void resize(u32 n) {
    if(n < parent.size()) throw gsh::Exception("gsh::UnionFind::resize / It cannot be smaller than it is now.");
    cnt += n - parent.size();
    parent.resize(n, -1);
  }
  constexpr u32 merge(const u32 a, const u32 b) {
#ifndef NDEBUG
    if(a >= parent.size() || b >= parent.size()) throw gsh::Exception("gsh::UnionFind::merge / The index is out of range. ( a=", a, ", b=", b, ", size=", parent.size(), " )");
#endif
    const i32 ar = root(a), br = root(b);
    if(ar == br) return ar;
    return this->merge_impl(ar, br);
  }
  constexpr bool merge_same(const u32 a, const u32 b) {
#ifndef NDEBUG
    if(a >= parent.size() || b >= parent.size()) throw gsh::Exception("gsh::UnionFind::merge_same / The index is out of range. ( a=", a, ", b=", b, ", size=", parent.size(), " )");
#endif
    const i32 ar = root(a), br = root(b);
    if(ar == br) return true;
    this->merge_impl(ar, br);
    return false;
  }
};
class RollbackUnionFind : public internal::UnionFindImpl<RollbackUnionFind> {
  friend class internal::UnionFindImpl<RollbackUnionFind>;
  Vec<i32> parent;
  u32 cnt = 0;
  struct change {
    u32 a, b;
    i32 c, d;
    bool merged;
    constexpr change(u32 A, u32 B, i32 C, i32 D, bool M) : a(A), b(B), c(C), d(D), merged(M) {}
  };
  Vec<change> history;
  constexpr i32 root(i32 n) const noexcept {
    while(parent[n] >= 0) n = parent[n];
    return n;
  }
public:
  using size_type = u32;
  constexpr RollbackUnionFind() noexcept {}
  constexpr explicit RollbackUnionFind(u32 n) : parent(n, -1), cnt(n) {}
  constexpr void reset() noexcept {
    cnt = parent.size();
    history.clear();
    for(u32 i = 0; i != parent.size(); ++i) parent[i] = -1;
  }
  constexpr void reserve(u32 q) { history.reserve(history.size() + q); }
  constexpr void resize(u32 n) {
    if(n < parent.size()) throw gsh::Exception("gsh::RollbackUnionFind::resize / It cannot be smaller than it is now.");
    cnt += n - parent.size();
    parent.resize(n, -1);
  }
  constexpr u32 merge(const u32 a, const u32 b) {
#ifndef NDEBUG
    if(a >= parent.size() || b >= parent.size()) throw gsh::Exception("gsh::RollbackUnionFind::merge / The index is out of range. ( a=", a, ", b=", b, ", size=", parent.size(), " )");
#endif
    const i32 ar = root(a), br = root(b);
    history.emplace_back(ar, br, parent[ar], parent[br], ar != br);
    if(ar == br) return ar;
    return merge_impl(ar, br);
  }
  constexpr bool merge_same(const u32 a, const u32 b) {
#ifndef NDEBUG
    if(a >= parent.size() || b >= parent.size()) throw gsh::Exception("gsh::RollbackUnionFind::merge_same / The index is out of range. ( a=", a, ", b=", b, ", size=", parent.size(), " )");
#endif
    const i32 ar = root(a), br = root(b);
    history.emplace_back(ar, br, parent[ar], parent[br], ar != br);
    if(ar == br) return true;
    merge_impl(ar, br);
    return false;
  }
  constexpr void undo() {
#ifndef NDEBUG
    if(history.empty()) throw Exception("gsh::RollbackUnionFind::undo / The history is empty.");
#endif
    auto [a, b, c, d, e] = history.back();
    history.pop_back();
    parent[a] = c, parent[b] = d;
    cnt += e;
  }
  class state {
    friend class RollbackUnionFind;
    u32 s;
    state() = delete;
    constexpr state(u32 n) noexcept : s(n) {}
  public:
    constexpr state(const state&) noexcept = default;
    constexpr state& operator=(const state&) noexcept = default;
    constexpr state next(i32 n) const noexcept { return state{s + n}; }
    constexpr state prev(i32 n) const noexcept { return state{s - n}; }
    constexpr void advance(i32 n) noexcept { s += n; }
  };
  constexpr state current() const noexcept { return state{history.size()}; }
  constexpr void rollback(state st) {
    while(st.s < history.size()) {
      auto [a, b, c, d, e] = history.back();
      history.pop_back();
      parent[a] = c, parent[b] = d;
      cnt += e;
    }
  }
};
template<class T = i64, class F = Plus, class I = Negate> class PotentializedUnionFind : public internal::UnionFindImpl<PotentializedUnionFind<T, F, I>> {
  friend class internal::UnionFindImpl<PotentializedUnionFind<T, F, I>>;
  Vec<i32> parent;
  Vec<T> diff;
  u32 cnt = 0;
  [[no_unique_address]] F func{};
  [[no_unique_address]] I inv{};
  T el{};
  constexpr i32 root(i32 n) noexcept {
    if(parent[n] < 0) return n;
    const i32 r = root(parent[n]);
    diff[n] = std::invoke(func, diff[parent[n]], diff[n]);
    return parent[n] = r;
  }
public:
  using value_type = T;
  using size_type = u32;
  constexpr PotentializedUnionFind() noexcept(std::is_nothrow_default_constructible_v<F> && std::is_nothrow_default_constructible_v<I> && std::is_nothrow_default_constructible_v<T>) {}
  constexpr explicit PotentializedUnionFind(F f, I i = I(), const T& e = T()) : func(f), inv(i), el(e) {}
  constexpr explicit PotentializedUnionFind(u32 n, F f = F(), I i = I(), const T& e = T()) : parent(n, -1), diff(n, e), cnt(n), func(f), inv(i), el(e) {}
  constexpr void reset() {
    cnt = parent.size();
    for(u32 i = 0; i != parent.size(); ++i) parent[i] = -1;
    diff.assign(parent.size(), el);
  }
  constexpr void resize(u32 n) {
    if(n < parent.size()) throw gsh::Exception("gsh::PotentializedUnionFind::resize / It cannot be smaller than it is now.");
    cnt += n - parent.size();
    parent.resize(n, -1);
    diff.resize(n, el);
  }
  constexpr const T& operator[](u32 n) { return potential(n); }
  constexpr const T& potential(u32 n) {
#ifndef NDEBUG
    if(n >= parent.size()) throw gsh::Exception("gsh::PotentializedUnionFind::potential / The index is out of range. ( n=", n, ", size=", parent.size(), " )");
#endif
    root(n);
    return diff[n];
  }
  // A[a] = func(A[b], result)
  constexpr T potential(u32 a, u32 b) {
#ifndef NDEBUG
    if(a >= parent.size() || b >= parent.size()) throw gsh::Exception("gsh::PotentializedUnionFind::potential / The index is out of range. ( a=", a, ", b=", b, ", size=", parent.size(), " )");
#endif
    root(a);
    root(b);
    return std::invoke(func, std::invoke(inv, diff[b]), diff[a]);
  }
  // A[a] = func(A[b], w) return leader(a)
  constexpr u32 merge(const u32 a, const u32 b, const T& w) {
#ifndef NDEBUG
    if(a >= parent.size() || b >= parent.size()) throw gsh::Exception("gsh::PotentializedUnionFind::merge / The index is out of range. ( a=", a, ", b=", b, ", size=", parent.size(), " )");
#endif
    const i32 ar = root(a), br = root(b);
    if(ar == br) return ar;
    const i32 sa = parent[ar], sb = parent[br];
    const bool f = sa < sb;
    const i32 tmp1 = f ? ar : br, tmp2 = f ? br : ar;
    parent[tmp1] += parent[tmp2];
    parent[tmp2] = tmp1;
    diff[tmp2] = std::invoke(func, diff[f ? a : b], std::invoke(inv, std::invoke(func, diff[f ? b : a], f ? w : std::invoke(inv, w))));
    --cnt;
    return tmp1;
  }
  // A[a] = func(A[b], w) return same(a, b)
  constexpr bool merge_same(const u32 a, const u32 b, const T& w) {
#ifndef NDEBUG
    if(a >= parent.size() || b >= parent.size()) throw gsh::Exception("gsh::PotentializedUnionFind::merge_same / The index is out of range. ( a=", a, ", b=", b, ", size=", parent.size(), " )");
#endif
    const i32 ar = root(a), br = root(b);
    if(ar == br) return true;
    const i32 sa = parent[ar], sb = parent[br];
    const bool f = sa < sb;
    const i32 tmp1 = f ? ar : br, tmp2 = f ? br : ar;
    parent[tmp1] += parent[tmp2];
    parent[tmp2] = tmp1;
    diff[tmp2] = std::invoke(func, diff[f ? a : b], std::invoke(inv, std::invoke(func, diff[f ? b : a], f ? w : std::invoke(inv, w))));
    --cnt;
    return false;
  }
  // A[a] = func(A[b], w)
  constexpr bool merge_valid(const u32 a, const u32 b, const T& w) {
#ifndef NDEBUG
    if(a >= parent.size() || b >= parent.size()) throw gsh::Exception("gsh::PotentializedUnionFind::merge_valid / The index is out of range. ( a=", a, ", b=", b, ", size=", parent.size(), " )");
#endif
    const i32 ar = root(a), br = root(b);
    if(ar == br) return diff[a] == std::invoke(func, diff[b], w);
    const i32 sa = parent[ar], sb = parent[br];
    const bool f = sa < sb;
    const i32 tmp1 = f ? ar : br, tmp2 = f ? br : ar;
    parent[tmp1] += parent[tmp2];
    parent[tmp2] = tmp1;
    diff[tmp2] = std::invoke(func, diff[f ? a : b], std::invoke(inv, std::invoke(func, diff[f ? b : a], f ? w : std::invoke(inv, w))));
    --cnt;
    return true;
  }
};
template<class T, class F> class MonoidalUnionFind : public internal::UnionFindImpl<MonoidalUnionFind<T, F>> {
  friend class internal::UnionFindImpl<MonoidalUnionFind<T, F>>;
  Vec<i32> parent;
  Vec<T> monoid;
  [[no_unique_address]] F func;
  u32 cnt = 0;
  constexpr i32 root(i32 n) noexcept {
    if(parent[n] < 0) return n;
    return parent[n] = root(parent[n]);
  }
public:
  using value_type = T;
  using size_type = u32;
  constexpr MonoidalUnionFind() noexcept {}
  constexpr explicit MonoidalUnionFind(F f) : func(std::move(f)) {}
  constexpr explicit MonoidalUnionFind(u32 n, F f = F()) : parent(n, -1), monoid(n), func(std::move(f)), cnt(n) {}
  constexpr MonoidalUnionFind(u32 n, F f, const T& init) : parent(n, -1), monoid(n, init), func(std::move(f)), cnt(n) {}
  constexpr ~MonoidalUnionFind() {
    for(u32 i = 0; i != parent.size(); ++i) {
      if(parent[i] < 0) std::destroy_at(&monoid[i]);
    }
    monoid.reset();
  }
  constexpr void reset() {
    cnt = parent.size();
    for(u32 i = 0; i != parent.size(); ++i) {
      if(parent[i] < 0) std::destroy_at(&monoid[i]);
      parent[i] = -1;
      std::construct_at(&monoid[i]);
    }
  }
  constexpr void reset(const T& init) {
    cnt = parent.size();
    for(u32 i = 0; i != parent.size(); ++i) {
      if(parent[i] < 0) std::destroy_at(&monoid[i]);
      parent[i] = -1;
      std::construct_at(&monoid[i], init);
    }
  }
  constexpr void resize(u32 n) {
    if(n < parent.size()) throw gsh::Exception("gsh::MonoidalUnionFind::resize / It cannot be smaller than it is now.");
    cnt += n - parent.size();
    parent.resize(n, -1);
    monoid.resize(n);
  }
  constexpr void resize(u32 n, const T& value) {
    if(n < parent.size()) throw gsh::Exception("gsh::MonoidalUnionFind::resize / It cannot be smaller than it is now.");
    cnt += n - parent.size();
    parent.resize(n, -1);
    monoid.resize(n, value);
  }
  constexpr T& operator[](u32 n) {
#ifndef NDEBUG
    if(n >= parent.size()) throw gsh::Exception("gsh::MonoidalUnionFind::operator[] / The index is out of range. ( n=", n, ", size=", parent.size(), " )");
#endif
    return monoid[root(n)];
  }
  constexpr u32 merge(const u32 a, const u32 b) {
#ifndef NDEBUG
    if(a >= parent.size() || b >= parent.size()) throw gsh::Exception("gsh::MonoidalUnionFind::merge / The index is out of range. ( a=", a, ", b=", b, ", size=", parent.size(), " )");
#endif
    const i32 ar = root(a), br = root(b);
    if(ar == br) return ar;
    const i32 sa = parent[ar], sb = parent[br];
    const bool f = sa < sb;
    const i32 tmp1 = f ? ar : br, tmp2 = f ? br : ar;
    parent[tmp1] += parent[tmp2];
    parent[tmp2] = tmp1;
    monoid[tmp1] = std::invoke(func, std::move(monoid[tmp1]), std::move(monoid[tmp2]));
    std::destroy_at(&monoid[tmp2]);
    --cnt;
    return tmp1;
  }
  constexpr bool merge_same(const u32 a, const u32 b) {
#ifndef NDEBUG
    if(a >= parent.size() || b >= parent.size()) throw gsh::Exception("gsh::MonoidalUnionFind::merge / The index is out of range. ( a=", a, ", b=", b, ", size=", parent.size(), " )");
#endif
    const i32 ar = root(a), br = root(b);
    if(ar == br) return true;
    const i32 sa = parent[ar], sb = parent[br];
    const bool f = sa < sb;
    const i32 tmp1 = f ? ar : br, tmp2 = f ? br : ar;
    parent[tmp1] += parent[tmp2];
    parent[tmp2] = tmp1;
    monoid[tmp1] = std::invoke(func, std::move(monoid[tmp1]), std::move(monoid[tmp2]));
    std::destroy_at(&monoid[tmp2]);
    --cnt;
    return false;
  }
};
} // namespace gsh
