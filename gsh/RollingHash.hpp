#pragma once
#include "Functional.hpp"
#include "Modint.hpp"
#include "Random.hpp"
#include "TypeDef.hpp"
#include "Util.hpp"
#include "Vec.hpp"
#include <array>
#include <limits>
#include <ranges>
#include <type_traits>
#include <utility>
namespace gsh {
template<u32 BaseCnt = 8, bool RegenerateBasesAtRuntime = false> class RollingHash {
public:
  using value_type = std::array<u64, BaseCnt>;
private:
  value_type hash{};
  constexpr static internal::StaticModint64Impl<(1ull << 61) - 1> mint;
  constexpr static auto GenerateBases() {
    value_type res{};
    Rand32 rnd(Seed());
    for(u32 i = 0; i != BaseCnt; ++i) res[i] = rnd() | 1;
    return res;
  }
  constexpr static auto GenerateBasesInv(const value_type& b) {
    value_type res{};
    for(u32 i = 0; i != BaseCnt; ++i) res[i] = mint.inv(b[i]);
    return res;
  }
  constexpr static auto compile_time_bases = GenerateBases();
  constexpr static auto compile_time_bases_inv = GenerateBasesInv(compile_time_bases);
  static inline value_type runtime_bases = []() {
    if constexpr(RegenerateBasesAtRuntime) return GenerateBases();
    else return compile_time_bases;
  }();
  static inline value_type runtime_bases_inv = []() {
    if constexpr(RegenerateBasesAtRuntime) return GenerateBasesInv(runtime_bases);
    else return compile_time_bases_inv;
  }();
  constexpr static const u64* bases() {
    if(std::is_constant_evaluated()) return compile_time_bases.data();
    else return runtime_bases.data();
  }
  constexpr static const u64* bases_inv() {
    if(std::is_constant_evaluated()) return compile_time_bases_inv.data();
    else return runtime_bases_inv.data();
  }
  constexpr static u64 PowBaseAt(u32 i, u32 e) {
    if(e == 0) return mint.one();
    return mint.pow(bases()[i], e);
  }
  constexpr static u64 PowBaseInvAt(u32 i, u32 e) {
    if(e == 0) return mint.one();
    return mint.pow(bases_inv()[i], e);
  }
  constexpr explicit RollingHash(const value_type& h) : hash(h) {}
  template<u32 BC, bool Regen> friend class RollingHashTable;
public:
  constexpr RollingHash() = default;
  template<std::ranges::forward_range R> requires Hashable<std::ranges::range_value_t<R>> explicit constexpr RollingHash(R&& r) {
    for(auto&& x : r) push_back(std::forward<decltype(x)>(x));
  }
  constexpr const value_type& value() const { return hash; }
  template<Hashable T> constexpr void push(T&& x) {
    const u64 v = mint.build(Hash(x));
    const u64* b = bases();
    for(u32 i = 0; i != BaseCnt; ++i) hash[i] = mint.add(mint.mul(hash[i], b[i]), v);
  }
  constexpr RollingHash& operator<<=(i32 shift) {
    if(shift <= 0) return *this;
    const u32 sh = static_cast<u32>(shift);
    const u64* b = bases();
    for(u32 i = 0; i != BaseCnt; ++i) hash[i] = mint.mul(hash[i], mint.pow(b[i], sh));
    return *this;
  }
  constexpr RollingHash& operator>>=(i32 shift) {
    if(shift <= 0) return *this;
    const u32 sh = static_cast<u32>(shift);
    const u64* bi = bases_inv();
    for(u32 i = 0; i != BaseCnt; ++i) hash[i] = mint.mul(hash[i], mint.pow(bi[i], sh));
    return *this;
  }
  constexpr RollingHash& operator+=(const RollingHash& h) {
    for(u32 i = 0; i != BaseCnt; ++i) hash[i] = mint.add(hash[i], h.hash[i]);
    return *this;
  }
  constexpr RollingHash& operator-=(const RollingHash& h) {
    for(u32 i = 0; i != BaseCnt; ++i) hash[i] = mint.sub(hash[i], h.hash[i]);
    return *this;
  }
  friend constexpr RollingHash operator<<(RollingHash h, i32 shift) {
    h <<= shift;
    return h;
  }
  friend constexpr RollingHash operator>>(RollingHash h, i32 shift) {
    h >>= shift;
    return h;
  }
  friend constexpr RollingHash operator+(RollingHash a, const RollingHash& b) {
    a += b;
    return a;
  }
  friend constexpr RollingHash operator-(RollingHash a, const RollingHash& b) {
    a -= b;
    return a;
  }
  friend constexpr bool operator==(const RollingHash& a, const RollingHash& b) { return a.hash == b.hash; }
};
template<u32 BaseCnt = 8, bool RegenerateBasesAtRuntime = false> class RollingHashTable {
  using RH = RollingHash<BaseCnt, RegenerateBasesAtRuntime>;
  using value_type = typename RH::value_type;
  Vec<value_type> pref_{};
public:
  constexpr RollingHashTable() = default;
  template<std::ranges::forward_range R> explicit constexpr RollingHashTable(R&& r) { build(std::forward<R>(r)); }
  template<std::ranges::forward_range R> requires (std::ranges::sized_range<R> && Hashable<std::ranges::range_value_t<R>>) constexpr void build(R&& r) {
    const u32 n = static_cast<u32>(std::ranges::size(r));
    pref_.resize(n + 1);
    pref_[0] = value_type{};
    const u64* b = RH::bases();
    u32 idx = 0;
    for(auto&& x : r) {
      ++idx;
      const u64 v = RH::mint.build(Hash(x));
      value_type nxt = pref_[idx - 1];
      for(u32 i = 0; i != BaseCnt; ++i) nxt[i] = RH::mint.add(RH::mint.mul(nxt[i], b[i]), v);
      pref_[idx] = nxt;
    }
  }
  constexpr u32 size() const { return pref_.empty() ? 0u : (pref_.size() - 1); }
  constexpr bool empty() const { return size() == 0; }
  constexpr RH get(u32 l, u32 r) const {
    const u32 n = size();
    Assume(l <= r);
    Assume(r <= n);
    const u32 len = r - l;
    value_type res{};
    if(len == 0) return RH(res);
    const u64* b = RH::bases();
    for(u32 i = 0; i != BaseCnt; ++i) {
      const u64 sub = RH::mint.mul(pref_[l][i], RH::mint.pow(b[i], len));
      res[i] = RH::mint.sub(pref_[r][i], sub);
    }
    return RH(res);
  }
  constexpr bool equal(u32 l1, u32 r1, u32 l2, u32 r2) const {
    if((r1 - l1) != (r2 - l2)) return false;
    return get(l1, r1) == get(l2, r2);
  }
  constexpr u32 lcp(u32 i, u32 j, u32 limit = std::numeric_limits<u32>::max()) const {
    const u32 n = size();
    Assume(i <= n);
    Assume(j <= n);
    u32 mx1 = n - i;
    u32 mx2 = n - j;
    u32 mx = mx1 < mx2 ? mx1 : mx2;
    if(limit < mx) mx = limit;
    u32 lo = 0, hi = mx + 1;
    while(hi - lo > 1) {
      const u32 mid = (lo + hi) >> 1;
      if(get(i, i + mid) == get(j, j + mid)) lo = mid;
      else hi = mid;
    }
    return lo;
  }
};
}
