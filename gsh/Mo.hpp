#pragma once
#include "TypeDef.hpp"
#include "Vec.hpp"
#include <cmath>
#include <functional>
namespace gsh {
class Mo {
  struct rg {
    u32 l, r;
    constexpr rg(u32 a, u32 b) : l(a), r(b) {}
  };
  Vec<rg> qu;
  f64 coef = 1.05;
public:
  constexpr Mo() {}
  constexpr void reserve(u32 q) { qu.reserve(q); }
  constexpr void query(u32 l, u32 r) { qu.emplace_back(l, r); }
  constexpr void set_coef(f64 c) { coef = c; }
  template<class F1, class F2, class F3> void run(F1&& add, F2&& del, F3&& slv) const { run(add, add, del, del, slv); }
  template<class F1, class F2, class F3, class F4, class F5> void run(F1&& addl, F2&& addr, F3&& dell, F4&& delr, F5&& slv) const {
    const u32 Q = qu.size();
    u32 N = 0;
    for(u32 i = 0; i != Q; ++i) N = N < qu[i].r ? qu[i].r : N;
    u32 width = coef * std::sqrt(static_cast<f64>(3ull * N * N) / (2 * Q));
    width += width == 0;
    Vec<u32> cnt(N + 1), buf(Q), block(Q), idx(Q);
    for(u32 i = 0; i != Q; ++i) ++cnt[qu[i].r];
    for(u32 i = 0; i != N; ++i) cnt[i + 1] += cnt[i];
    for(u32 i = 0; i != Q; ++i) buf[--cnt[qu[i].r]] = i;
    cnt.assign(N / width + 2, 0);
    for(u32 i = 0; i != Q; ++i) block[i] = qu[i].l / width;
    for(u32 i = 0; i != Q; ++i) ++cnt[block[i]];
    for(u32 i = 0; i != cnt.size() - 1; ++i) cnt[i + 1] += cnt[i];
    for(u32 i = 0; i != Q; ++i) idx[--cnt[block[buf[i]]]] = buf[i];
    for(u32 i = 0; i < cnt.size() - 1; i += 2) {
      const u32 l = cnt[i], r = cnt[i + 1];
      for(u32 j = 0; j != (r - l) / 2; ++j) {
        const u32 t = idx[l + j];
        idx[l + j] = idx[r - j - 1], idx[r - j - 1] = t;
      }
    }
    u32 nl = 0, nr = 0;
    for(u32 i : idx) {
      while(nl > qu[i].l) std::invoke(addl, --nl);
      while(nr < qu[i].r) std::invoke(addr, nr++);
      while(nl < qu[i].l) std::invoke(dell, nl++);
      while(nr > qu[i].r) std::invoke(delr, --nr);
      std::invoke(slv, i);
    }
  }
};
}
