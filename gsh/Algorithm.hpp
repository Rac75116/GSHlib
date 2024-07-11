#pragma once
#include <type_traits>         // std::common_type
#include <cstring>             // std::memset
#include <cstdlib>             // std::malloc, std::free
#include <algorithm>           // std::lower_bound
#include <gsh/TypeDef.hpp>     // gsh::itype
#include <gsh/Arr.hpp>         // gsh::Arr
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

template<Range R> constexpr Arr<itype::u32> LongestIncreasingSubsequenceIndex(R&& r) {
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

template<RandomAccessRange R> constexpr Arr<itype::u32> EnumerateLongestCommonPrefixLength(R&& r) {
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

}  // namespace gsh