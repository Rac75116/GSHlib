#pragma once
#include <type_traits>         // std::common_type
#include <cstring>             // std::memset
#include <cstdlib>             // std::malloc, std::free
#include <algorithm>           // std::lower_bound
#include <cmath>               // std::sqrt
#include <gsh/TypeDef.hpp>     // gsh::itype
#include <gsh/Arr.hpp>         // gsh::Arr
#include <gsh/Vec.hpp>         // gsh::Vec
#include <gsh/Range.hpp>       // gsh::Range
#include <gsh/Functional.hpp>  // gsh::Less, gsh::Greater

namespace gsh {

template<class T, class U> constexpr std::common_type_t<T, U> Min(const T& a, const U& b) {
    return a < b ? a : b;
}
template<class T, class... Args> constexpr auto Min(const T& x, const Args&... args) {
    return Min(x, Min(args...));
}
template<class T, class U> constexpr std::common_type_t<T, U> Max(const T& a, const U& b) {
    return a < b ? b : a;
}
template<class T, class... Args> constexpr auto Max(const T& x, const Args&... args) {
    return Max(x, Max(args...));
}
template<class T, class U> constexpr bool Chmin(T& a, const U& b) {
    const bool f = b < a;
    a = f ? b : a;
    return f;
}
template<class T, class... Args> constexpr bool Chmin(T& a, const Args&... b) {
    return Chmin(a, Min(b...));
}
template<class T, class U> constexpr bool Chmax(T& a, const U& b) {
    const bool f = a < b;
    a = f ? b : a;
    return f;
}
template<class T, class... Args> constexpr bool Chmax(T& a, const Args&... b) {
    return Chmax(a, Max(b...));
}

namespace internal {
    template<class T, bool Rev = false, class Proj = Identity> void SortUnsigned32(T* const p, const itype::u32 n, Proj proj = {}) {
        static itype::u32 cnt1[1 << 16], cnt2[1 << 16];
        std::memset(cnt1, 0, sizeof(cnt1));
        std::memset(cnt2, 0, sizeof(cnt2));
        for (itype::u32 i = 0; i != n; ++i) {
            auto tmp = proj(p[i]);
            const itype::u16 a = tmp & 0xffff;
            const itype::u16 b = tmp >> 16 & 0xffff;
            ++cnt1[a];
            ++cnt2[b];
        }
        for (itype::u32 i = 0; i != (1 << 16) - 1; ++i) {
            cnt1[i + 1] += cnt1[i];
            cnt2[i + 1] += cnt2[i];
        }
        T* const tmp = reinterpret_cast<T*>(std::malloc(sizeof(T) * n));
        if constexpr (Rev) {
            for (itype::u32 i = 0; i != n; ++i) tmp[--cnt1[proj(p[i]) & 0xffff]] = std::move(p[i]);
            for (itype::u32 i = 0; i != n; ++i) p[--cnt2[proj(tmp[i]) >> 16 & 0xffff]] = std::move(tmp[i]);
        } else {
            for (itype::u32 i = n; i != 0;) --i, tmp[--cnt1[proj(p[i]) & 0xffff]] = std::move(p[i]);
            for (itype::u32 i = n; i != 0;) --i, p[--cnt2[proj(tmp[i]) >> 16 & 0xffff]] = std::move(tmp[i]);
        }
        std::free(tmp);
    }
    template<class T, bool Rev = false, class Proj = Identity> void SortUnsigned64(T* const p, const itype::u32 n, Proj proj = {}) {
        static itype::u32 cnt1[1 << 16], cnt2[1 << 16], cnt3[1 << 16], cnt4[1 << 16];
        std::memset(cnt1, 0, sizeof(cnt1));
        std::memset(cnt2, 0, sizeof(cnt2));
        std::memset(cnt3, 0, sizeof(cnt3));
        std::memset(cnt4, 0, sizeof(cnt4));
        for (itype::u32 i = 0; i != n; ++i) {
            auto tmp = proj(p[i]);
            const itype::u16 a = tmp & 0xffff;
            const itype::u16 b = tmp >> 16 & 0xffff;
            const itype::u16 c = tmp >> 32 & 0xffff;
            const itype::u16 d = tmp >> 48 & 0xffff;
            ++cnt1[a];
            ++cnt2[b];
            ++cnt3[c];
            ++cnt4[d];
        }
        for (itype::u32 i = 0; i != (1 << 16) - 1; ++i) {
            cnt1[i + 1] += cnt1[i];
            cnt2[i + 1] += cnt2[i];
            cnt3[i + 1] += cnt3[i];
            cnt4[i + 1] += cnt4[i];
        }
        T* const tmp = reinterpret_cast<T*>(std::malloc(sizeof(T) * n));
        if constexpr (Rev) {
            for (itype::u32 i = 0; i != n; ++i) tmp[--cnt1[proj(p[i]) & 0xffff]] = std::move(p[i]);
            for (itype::u32 i = 0; i != n; ++i) p[--cnt2[proj(tmp[i]) >> 16 & 0xffff]] = std::move(tmp[i]);
            for (itype::u32 i = 0; i != n; ++i) tmp[--cnt3[proj(p[i]) >> 32 & 0xffff]] = std::move(p[i]);
            for (itype::u32 i = 0; i != n; ++i) p[--cnt4[proj(tmp[i]) >> 48 & 0xffff]] = std::move(tmp[i]);
        } else {
            for (itype::u32 i = n; i != 0;) --i, tmp[--cnt1[proj(p[i]) & 0xffff]] = std::move(p[i]);
            for (itype::u32 i = n; i != 0;) --i, p[--cnt2[proj(tmp[i]) >> 16 & 0xffff]] = std::move(tmp[i]);
            for (itype::u32 i = n; i != 0;) --i, tmp[--cnt3[proj(p[i]) >> 32 & 0xffff]] = std::move(p[i]);
            for (itype::u32 i = n; i != 0;) --i, p[--cnt4[proj(tmp[i]) >> 48 & 0xffff]] = std::move(tmp[i]);
        }
        std::free(tmp);
    }
}  // namespace internal
/*
template<Range R, class Comp = Less, class Proj = Identity>
    requires std::sortable<std::ranges::iterator_t<R>, Comp, Proj>
constexpr void Sort(R&& r, Comp comp = {}, Proj proj = {}) {
    using traits = RangeTraits<R>;
    const itype::u32 n = traits::size(r);
    if constexpr (!traits::pointer_obtainable) {
        //Arr<decltype(proj()) tmp(n);
        //Sort(tmp, comp, proj);
        //for (itype::u32 i = 0; auto&& x : r) x = std::move(tmp[i]);
    } else {
        if constexpr (std::is_same_v<typename traits::value_type, itype::u32>) {
        } else if constexpr (std::is_same_v<typename traits::value_type, itype::u64>) {
        }
    }
}
*/

template<Range R> constexpr Arr<itype::u32> LongestIncreasingSubsequence(R&& r) {
    using T = typename RangeTraits<R>::value_type;
    Arr<T> dp(RangeTraits<R>::size(r));
    Arr<itype::u32> idx(dp.size());
    itype::u32 i = 0;
    T *begin = dp.data(), *last = dp.data();
    for (const T& x : r) {
        T* loc = std::lower_bound(begin, last, x);
        idx[i++] = loc - begin;
        last += (loc == last);
        *loc = x;
    }
    itype::u32 cnt = last - begin - 1;
    Arr<itype::u32> res(last - begin);
    for (itype::u32 i = dp.size(); i != 0;)
        if (idx[--i] == cnt) res[cnt--] = i;
    return res;
}

template<RandomAccessRange R> constexpr Arr<itype::u32> LongestCommonPrefix(R&& r) {
    using traits = RangeTraits<R>;
    const itype::u32 n = traits::size(r);
    Arr<itype::u32> res(n);
    if (n == 0) return res;
    res[0] = n;
    const auto itr = traits::begin(r);
    itype::u32 i = 1, j = 0;
    while (i != n) {
        while (i + j < n && *std::next(itr, j) == *std::next(itr, i + j)) ++j;
        res[i] = j;
        if (j == 0) {
            ++i;
            continue;
        }
        itype::u32 k = 1;
        while (k < j && k + res[k] < j) ++k;
        const auto a = res.data() + i + 1;
        const auto b = res.data() + 1;
        for (itype::u32 l = 0; l != k - 1; ++l) a[l] = b[l];
        i += k;
        j -= k;
    }
    return res;
}

template<class T = itype::u64, Rangeof<itype::u32> R> constexpr T CountDistinctSubsequences(R&& r) {
    using traits = RangeTraits<R>;
    const itype::u32 n = traits::size(r);
    if (n == 0) return 0;
    Arr<itype::u64> s(n);
    for (itype::u32 i = 0; itype::u32 x : r) {
        s[i] = static_cast<itype::u64>(i) << 32 | x;
        ++i;
    }
    internal::SortUnsigned32(s.data(), n);
    Arr<itype::u32> rank(n);
    rank[s[0] >> 32] = 0;
    itype::u32 cnt = 0, end = s[0];
    for (itype::u32 i = 1; i != n; ++i) {
        cnt += end != static_cast<itype::u32>(s[i]);
        rank[s[i] >> 32] = cnt;
        end = s[i];
    }
    Arr<itype::u32> last(cnt + 1, n);
    Arr<T> dp(n + 1);
    dp[0] = 1;
    const T m = 2;
    for (itype::u32 i = 0; i != n; ++i) {
        dp[i + 1] = m * dp[i] - dp[last[rank[i]]];
        last[rank[i]] = i;
    }
    return dp[n] - dp[0];
}

template<Range R> constexpr auto Majority(R&& r) {
    using traits = RangeTraits<R>;
    itype::u32 c = 0;
    itype::u32 len = 0;
    auto i = traits::begin(r), j = traits::end(r), k = j;
    for (; i != j; ++i) {
        ++len;
        if (c == 0) k = i, c = 1;
        else c += static_cast<itype::u32>(static_cast<bool>(*i == *k)) * 2 - 1;
    }
    c = 0;
    for (i = traits::begin(r); i != j; ++i) c += static_cast<bool>(*i == *k);
    return (2 * c >= len ? k : j);
}

class Mo {
    struct rg {
        itype::u32 l, r;
        constexpr rg(itype::u32 a, itype::u32 b) : l(a), r(b) {}
    };
    Vec<rg> qu;
public:
    constexpr Mo() {}
    constexpr void reserve(itype::u32 q) { qu.reserve(q); }
    constexpr void query(itype::u32 l, itype::u32 r) { qu.emplace_back(l, r); }
    template<class F1, class F2, class F3> void run(F1&& add, F2&& del, F3&& slv) const { solve(add, add, del, del, slv); }
    template<class F1, class F2, class F3, class F4, class F5> void run(F1&& addl, F2&& addr, F3&& dell, F4&& delr, F5&& slv) const {
        const itype::u32 Q = qu.size();
        itype::u32 N = 0;
        for (itype::u32 i = 0; i != Q; ++i) N = N < qu[i].r ? qu[i].r : N;
        itype::u32 width = 1.1 * std::sqrt(static_cast<ftype::f64>(3ull * N * N) / (2 * Q));
        width += width == 0;
        Arr<itype::u32> cnt(N + 1), buf(Q), block(Q), idx(Q);
        for (itype::u32 i = 0; i != Q; ++i) ++cnt[qu[i].r];
        for (itype::u32 i = 0; i != N; ++i) cnt[i + 1] += cnt[i];
        for (itype::u32 i = 0; i != Q; ++i) buf[--cnt[qu[i].r]] = i;
        cnt.assign(N / width + 2, 0);
        for (itype::u32 i = 0; i != Q; ++i) block[i] = qu[i].l / width;
        for (itype::u32 i = 0; i != Q; ++i) ++cnt[block[i]];
        for (itype::u32 i = 0; i != cnt.size() - 1; ++i) cnt[i + 1] += cnt[i];
        for (itype::u32 i = 0; i != Q; ++i) idx[--cnt[block[buf[i]]]] = buf[i];
        for (itype::u32 i = 0; i < cnt.size() - 1; i += 2) {
            const itype::u32 l = cnt[i], r = cnt[i + 1];
            for (itype::u32 j = 0; j != (r - l) / 2; ++j) {
                const itype::u32 t = idx[l + j];
                idx[l + j] = idx[r - j - 1], idx[r - j - 1] = t;
            }
        }
        itype::u32 nl = 0, nr = 0;
        for (itype::u32 i : idx) {
            while (nl > qu[i].l) addl(--nl);
            while (nr < qu[i].r) addr(nr++);
            while (nl < qu[i].l) dell(nl++);
            while (nr > qu[i].r) delr(--nr);
            slv(i);
        }
        /*
        Arr<itype::u32> idx(Q);
        for (itype::u32 i = 0; i != Q; ++i) idx[i] = i;
        {
            auto hilbertorder = [maxn = (std::bit_floor(N) << 1) - 1](itype::u32 x, itype::u32 y) {
                itype::u64 rx, ry, d = 0;
                for (itype::u64 s = (maxn + 1) >> 1; s; s >>= 1) {
                    rx = (x & s) != 0, ry = (y & s) != 0;
                    d += s * s * ((rx * 3) ^ ry);
                    if (ry) continue;
                    if (rx) {
                        x = maxn - x;
                        y = maxn - y;
                    }
                    itype::u32 t = x;
                    x = y, y = t;
                }
                return d;
            };
            Arr<itype::u64> eval(Q);
            for (itype::u32 i = 0; i != Q; ++i) eval[i] = hilbertorder(qu[i].l, qu[i].r);
            std::sort(idx.begin(), idx.end(), [&](itype::u32 a, itype::u32 b) { return eval[a] < eval[b]; });
        }
        itype::u32 nl = 0, nr = 0;
        for (itype::u32 i : idx) {
            while (nl > qu[i].l) addl(--nl);
            while (nr < qu[i].r) addr(nr++);
            while (nl < qu[i].l) dell(nl++);
            while (nr > qu[i].r) delr(--nr);
            slv(i);
        }
        */
    }
};

}  // namespace gsh