#pragma once
#include <gsh/TypeDef.hpp>     // gsh::itype
#include <gsh/Arr.hpp>         // gsh::Arr
#include <gsh/Vec.hpp>         // gsh::Vec
#include <gsh/Exception.hpp>   // gsh::Exception
#include <gsh/Functional.hpp>  // gsh::Plus, gsh::Negate

namespace gsh {

class UnionFind {
    Arr<itype::i32> parent;
    itype::u32 cnt = 0;
    constexpr itype::i32 root(itype::i32 n) noexcept {
        if (parent[n] < 0) return n;
        return parent[n] = root(parent[n]);
    }
public:
    using size_type = itype::u32;
    constexpr UnionFind() {}
    constexpr explicit UnionFind(itype::u32 n) : parent(n, -1), cnt(n) {}
    constexpr void reset() {
        cnt = size();
        for (itype::u32 i = 0; i != size(); ++i) parent[i] = -1;
    }
    constexpr itype::u32 size() const noexcept { return parent.size(); }
    [[nodiscard]] constexpr bool empty() const noexcept { return parent.empty(); }
    constexpr void resize(itype::u32 n) {
        if (n < size()) throw gsh::Exception("gsh::UnionFind::resize / It cannot be smaller than it is now.");
        cnt += n - size();
        parent.resize(n, -1);
    }
    constexpr itype::u32 leader(itype::u32 n) {
#ifndef NDEBUG
        if (n >= size()) throw gsh::Exception("gsh::UnionFind::leader / The index is out of range. ( n=", n, ", size=", size(), " )");
#endif
        return root(n);
    }
    constexpr bool is_leader(itype::u32 n) const {
#ifndef NDEBUG
        if (n >= size()) throw gsh::Exception("gsh::UnionFind::is_leader / The index is out of range. ( n=", n, ", size=", size(), " )");
#endif
        return parent[n] < 0;
    }
    constexpr bool same(itype::u32 a, itype::u32 b) {
#ifndef NDEBUG
        if (a >= size() || b >= size()) throw gsh::Exception("gsh::UnionFind::same / The index is out of range. ( a=", a, ", b=", b, ", size=", size(), " )");
#endif
        return root(a) == root(b);
    }
    constexpr itype::u32 merge(const itype::u32 a, const itype::u32 b) {
#ifndef NDEBUG
        if (a >= size() || b >= size()) throw gsh::Exception("gsh::UnionFind::merge / The index is out of range. ( a=", a, ", b=", b, ", size=", size(), " )");
#endif
        const itype::i32 ar = root(a), br = root(b);
        if (ar == br) return ar;
        const itype::i32 sa = parent[ar], sb = parent[br];
        const itype::i32 tmp1 = sa < sb ? ar : br, tmp2 = sa < sb ? br : ar;
        parent[tmp1] += parent[tmp2];
        parent[tmp2] = tmp1;
        --cnt;
        return tmp1;
    }
    constexpr bool merge_same(const itype::u32 a, const itype::u32 b) {
#ifndef NDEBUG
        if (a >= size() || b >= size()) throw gsh::Exception("gsh::UnionFind::merge_same / The index is out of range. ( a=", a, ", b=", b, ", size=", size(), " )");
#endif
        const itype::i32 ar = root(a), br = root(b);
        if (ar == br) return true;
        const itype::i32 sa = parent[ar], sb = parent[br];
        const itype::i32 tmp1 = sa < sb ? ar : br, tmp2 = sa < sb ? br : ar;
        parent[tmp1] += parent[tmp2];
        parent[tmp2] = tmp1;
        --cnt;
        return false;
    }
    constexpr itype::u32 size(itype::u32 n) {
#ifndef NDEBUG
        if (n >= size()) throw gsh::Exception("gsh::UnionFind::size(itype::u32) / The index is out of range. ( n=", n, ", size=", size(), " )");
#endif
        return -parent[root(n)];
    }
    constexpr itype::u32 max_group_size() const noexcept {
        itype::i32 res = 1;
        for (itype::u32 i = 0; i != size(); ++i) {
            res = -parent[i] < res ? res : -parent[i];
        }
        return res;
    }
    constexpr itype::u32 count_groups() const noexcept { return cnt; }
    constexpr Arr<itype::u32> extract(itype::u32 n) {
#ifndef NDEBUG
        if (n >= size()) throw gsh::Exception("gsh::UnionFind::extract / The index is out of range. ( n=", n, ", size=", size(), " )");
#endif
        const itype::i32 nr = root(n);
        itype::u32 ccnt = 0;
        for (itype::u32 i = 0; i != size(); ++i) ccnt += root(i) == nr;
        Arr<itype::u32> res(ccnt);
        for (itype::u32 i = 0, j = 0; i != size(); ++i)
            if (i == static_cast<itype::u32>(nr) || parent[i] == nr) res[j++] = i;
        return res;
    }
    constexpr Arr<Arr<itype::u32>> groups() {
        Arr<itype::u32> key(size());
        itype::u32 cnt = 0;
        for (itype::u32 i = 0; i != size(); ++i) {
            if (parent[i] < 0) key[i] = cnt++;
        }
        Arr<itype::u32> cnt2(cnt);
        for (itype::u32 i = 0; i != size(); ++i) ++cnt2[key[root(i)]];
        Arr<Arr<itype::u32>> res(cnt);
        for (itype::u32 i = 0; i != cnt; ++i) {
            res[i].resize(cnt2[i]);
            cnt2[i] = 0;
        }
        for (itype::u32 i = 0; i != size(); ++i) {
            const itype::u32 idx = key[parent[i] < 0 ? i : parent[i]];
            res[idx][cnt2[idx]++] = i;
        }
        return res;
    }
};

class RollbackUnionFind {
    Arr<itype::i32> parent;
    itype::u32 cnt = 0;
    struct change {
        itype::u32 a, b;
        itype::i32 c, d;
        bool merged;
        constexpr change(itype::u32 A, itype::u32 B, itype::i32 C, itype::i32 D, bool M) : a(A), b(B), c(C), d(D), merged(M) {}
    };
    Vec<change> history;
    constexpr itype::i32 root(itype::i32 n) const noexcept {
        while (parent[n] >= 0) n = parent[n];
        return n;
    }
public:
    using size_type = itype::u32;
    constexpr RollbackUnionFind() {}
    constexpr explicit RollbackUnionFind(itype::u32 n) : parent(n, -1), cnt(n) {}
    constexpr void reset() {
        cnt = size();
        history.clear();
        for (itype::u32 i = 0; i != size(); ++i) parent[i] = -1;
    }
    constexpr void reserve(itype::u32 q) { history.reserve(history.size() + q); }
    constexpr itype::u32 size() const noexcept { return parent.size(); }
    [[nodiscard]] constexpr bool empty() const noexcept { return parent.empty(); }
    constexpr void resize(itype::u32 n) {
        if (n < size()) throw gsh::Exception("gsh::RollbackUnionFind::resize / It cannot be smaller than it is now.");
        cnt += n - size();
        parent.resize(n, -1);
    }
    constexpr itype::u32 leader(itype::u32 n) const {
#ifndef NDEBUG
        if (n >= size()) throw gsh::Exception("gsh::RollbackUnionFind::leader / The index is out of range. ( n=", n, ", size=", size(), " )");
#endif
        return root(n);
    }
    constexpr bool is_leader(itype::u32 n) const {
#ifndef NDEBUG
        if (n >= size()) throw gsh::Exception("gsh::RollbackUnionFind::is_leader / The index is out of range. ( n=", n, ", size=", size(), " )");
#endif
        return parent[n] < 0;
    }
    constexpr bool same(itype::u32 a, itype::u32 b) const {
#ifndef NDEBUG
        if (a >= size() || b >= size()) throw gsh::Exception("gsh::RollbackUnionFind::same / The index is out of range. ( a=", a, ", b=", b, ", size=", size(), " )");
#endif
        return root(a) == root(b);
    }
    constexpr itype::u32 merge(const itype::u32 a, const itype::u32 b) {
#ifndef NDEBUG
        if (a >= size() || b >= size()) throw gsh::Exception("gsh::RollbackUnionFind::merge / The index is out of range. ( a=", a, ", b=", b, ", size=", size(), " )");
#endif
        const itype::i32 ar = root(a), br = root(b);
        const itype::i32 sa = parent[ar], sb = parent[br];
        history.emplace_back(ar, br, sa, sb, ar != br);
        if (ar == br) return ar;
        const itype::i32 tmp1 = sa < sb ? ar : br, tmp2 = sa < sb ? br : ar;
        parent[tmp1] += parent[tmp2];
        parent[tmp2] = tmp1;
        --cnt;
        return tmp1;
    }
    constexpr bool merge_same(const itype::u32 a, const itype::u32 b) {
#ifndef NDEBUG
        if (a >= size() || b >= size()) throw gsh::Exception("gsh::RollbackUnionFind::merge_same / The index is out of range. ( a=", a, ", b=", b, ", size=", size(), " )");
#endif
        const itype::i32 ar = root(a), br = root(b);
        const itype::i32 sa = parent[ar], sb = parent[br];
        history.emplace_back(ar, br, sa, sb, ar != br);
        if (ar == br) return true;
        const itype::i32 tmp1 = sa < sb ? ar : br, tmp2 = sa < sb ? br : ar;
        parent[tmp1] += parent[tmp2];
        parent[tmp2] = tmp1;
        --cnt;
        return false;
    }
    constexpr itype::u32 size(itype::u32 n) const {
#ifndef NDEBUG
        if (n >= size()) throw gsh::Exception("gsh::RollbackUnionFind::size(itype::u32) / The index is out of range. ( n=", n, ", size=", size(), " )");
#endif
        return -parent[root(n)];
    }
    constexpr itype::u32 max_group_size() const noexcept {
        itype::i32 res = 1;
        for (itype::u32 i = 0; i != size(); ++i) {
            res = -parent[i] < res ? res : -parent[i];
        }
        return res;
    }
    constexpr itype::u32 count_groups() const noexcept { return cnt; }
    constexpr Arr<itype::u32> extract(itype::u32 n) const {
#ifndef NDEBUG
        if (n >= size()) throw gsh::Exception("gsh::RollbackUnionFind::extract / The index is out of range. ( n=", n, ", size=", size(), " )");
#endif
        const itype::i32 nr = root(n);
        itype::u32 ccnt = 0;
        for (itype::u32 i = 0; i != size(); ++i) ccnt += root(i) == nr;
        Arr<itype::u32> res(ccnt);
        for (itype::u32 i = 0, j = 0; i != size(); ++i)
            if (i == static_cast<itype::u32>(nr) || parent[i] == nr) res[j++] = i;
        return res;
    }
    constexpr Arr<Arr<itype::u32>> groups() const {
        Arr<itype::u32> key(size());
        itype::u32 cnt = 0;
        for (itype::u32 i = 0; i != size(); ++i) {
            if (parent[i] < 0) key[i] = cnt++;
        }
        Arr<itype::u32> cnt2(cnt);
        for (itype::u32 i = 0; i != size(); ++i) ++cnt2[key[root(i)]];
        Arr<Arr<itype::u32>> res(cnt);
        for (itype::u32 i = 0; i != cnt; ++i) {
            res[i].resize(cnt2[i]);
            cnt2[i] = 0;
        }
        for (itype::u32 i = 0; i != size(); ++i) {
            const itype::u32 idx = key[parent[i] < 0 ? i : parent[i]];
            res[idx][cnt2[idx]++] = i;
        }
        return res;
    }
    constexpr void undo() {
#ifndef NDEBUG
        if (history.empty()) throw Exception("gsh::RollbackUnionFind::undo / The history is empty.");
#endif
        auto [a, b, c, d, e] = history.back();
        history.pop_back();
        parent[a] = c, parent[b] = d;
        cnt += e;
    }
    constexpr itype::u32 get_state() const { return history.size(); }
    constexpr void rollback(itype::u32 s) {
        while (s < history.size()) undo();
    }
};

template<class T = itype::i64, class F = Plus, class I = Negate> class PotentializedUnionFind {
    Arr<itype::i32> parent;
    Arr<T> diff;
    itype::u32 cnt = 0;
    [[no_unique_address]] F func{};
    [[no_unique_address]] I inv{};
    T el{};
    constexpr itype::i32 root(itype::i32 n) noexcept {
        if (parent[n] < 0) return n;
        const itype::i32 r = root(parent[n]);
        diff[n] = Invoke(func, diff[parent[n]], diff[n]);
        return parent[n] = r;
    }
public:
    using size_type = itype::u32;
    constexpr PotentializedUnionFind() {}
    constexpr PotentializedUnionFind(F f, I i = I(), const T& e = T()) : el(e), func(f), inv(i) {}
    constexpr explicit PotentializedUnionFind(itype::u32 n, F f = F(), I i = I(), const T& e = T()) : parent(n, -1), diff(n, e), cnt(n), func(f), inv(i), el(e) {}
    constexpr void reset() {
        cnt = size();
        for (itype::u32 i = 0; i != size(); ++i) parent[i] = -1;
        diff = Arr<T>(size(), el);
    }
    constexpr itype::u32 size() const noexcept { return parent.size(); }
    [[nodiscard]] constexpr bool empty() const noexcept { return parent.empty(); }
    constexpr void resize(itype::u32 n) {
        if (n < size()) throw gsh::Exception("gsh::PotentializedUnionFind::resize / It cannot be smaller than it is now.");
        cnt += n - size();
        parent.resize(n, -1);
        diff.resize(n, el);
    }
    constexpr itype::u32 leader(itype::u32 n) {
#ifndef NDEBUG
        if (n >= size()) throw gsh::Exception("gsh::PotentializedUnionFind::leader / The index is out of range. ( n=", n, ", size=", size(), " )");
#endif
        return root(n);
    }
    constexpr bool is_leader(itype::u32 n) const {
#ifndef NDEBUG
        if (n >= size()) throw gsh::Exception("gsh::PotentializedUnionFind::is_leader / The index is out of range. ( n=", n, ", size=", size(), " )");
#endif
        return parent[n] < 0;
    }
    constexpr bool same(itype::u32 a, itype::u32 b) {
#ifndef NDEBUG
        if (a >= size() || b >= size()) throw gsh::Exception("gsh::PotentializedUnionFind::same / The index is out of range. ( a=", a, ", b=", b, ", size=", size(), " )");
#endif
        return root(a) == root(b);
    }
    constexpr const T& potential(itype::u32 n) {
#ifndef NDEBUG
        if (n >= size()) throw gsh::Exception("gsh::PotentializedUnionFind::potential / The index is out of range. ( n=", n, ", size=", size(), " )");
#endif
        root(n);
        return diff[n];
    }
    // A[a] = func(A[b], result)
    constexpr T potential(itype::u32 a, itype::u32 b) {
#ifndef NDEBUG
        if (a >= size() || b >= size()) throw gsh::Exception("gsh::PotentializedUnionFind::potential / The index is out of range. ( a=", a, ", b=", b, ", size=", size(), " )");
#endif
        root(a);
        root(b);
        return Invoke(func, Invoke(inv, diff[b]), diff[a]);
    }
    // A[a] = func(A[b], w) return leader(a)
    constexpr itype::u32 merge(const itype::u32 a, const itype::u32 b, const T& w) {
#ifndef NDEBUG
        if (a >= size() || b >= size()) throw gsh::Exception("gsh::PotentializedUnionFind::merge / The index is out of range. ( a=", a, ", b=", b, ", size=", size(), " )");
#endif
        const itype::i32 ar = root(a), br = root(b);
        if (ar == br) return ar;
        const itype::i32 sa = parent[ar], sb = parent[br];
        const bool f = sa < sb;
        const itype::i32 tmp1 = f ? ar : br, tmp2 = f ? br : ar;
        parent[tmp1] += parent[tmp2];
        parent[tmp2] = tmp1;
        diff[tmp2] = func(diff[f ? a : b], Invoke(inv, Invoke(func, diff[f ? b : a], f ? w : Invoke(inv, w))));
        --cnt;
        return tmp1;
    }
    // A[a] = func(A[b], w) return same(a, b)
    constexpr bool merge_same(const itype::u32 a, const itype::u32 b, const T& w) {
#ifndef NDEBUG
        if (a >= size() || b >= size()) throw gsh::Exception("gsh::PotentializedUnionFind::merge_same / The index is out of range. ( a=", a, ", b=", b, ", size=", size(), " )");
#endif
        const itype::i32 ar = root(a), br = root(b);
        if (ar == br) return true;
        const itype::i32 sa = parent[ar], sb = parent[br];
        const bool f = sa < sb;
        const itype::i32 tmp1 = f ? ar : br, tmp2 = f ? br : ar;
        parent[tmp1] += parent[tmp2];
        parent[tmp2] = tmp1;
        diff[tmp2] = func(diff[f ? a : b], Invoke(inv, Invoke(func, diff[f ? b : a], f ? w : Invoke(inv, w))));
        --cnt;
        return false;
    }
    // A[a] = func(A[b], w)
    constexpr bool merge_valid(const itype::u32 a, const itype::u32 b, const T& w) {
#ifndef NDEBUG
        if (a >= size() || b >= size()) throw gsh::Exception("gsh::PotentializedUnionFind::merge_valid / The index is out of range. ( a=", a, ", b=", b, ", size=", size(), " )");
#endif
        const itype::i32 ar = root(a), br = root(b);
        if (ar == br) return diff[a] == Invoke(func, diff[b], w);
        const itype::i32 sa = parent[ar], sb = parent[br];
        const bool f = sa < sb;
        const itype::i32 tmp1 = f ? ar : br, tmp2 = f ? br : ar;
        parent[tmp1] += parent[tmp2];
        parent[tmp2] = tmp1;
        diff[tmp2] = func(diff[f ? a : b], Invoke(inv, Invoke(func, diff[f ? b : a], f ? w : Invoke(inv, w))));
        --cnt;
        return true;
    }
    constexpr itype::u32 size(itype::u32 n) {
#ifndef NDEBUG
        if (n >= size()) throw gsh::Exception("gsh::PotentializedUnionFind::size(itype::u32) / The index is out of range. ( n=", n, ", size=", size(), " )");
#endif
        return -parent[root(n)];
    }
    constexpr itype::u32 max_group_size() const noexcept {
        itype::i32 res = 1;
        for (itype::u32 i = 0; i != size(); ++i) {
            res = -parent[i] < res ? res : -parent[i];
        }
        return res;
    }
    constexpr itype::u32 count_groups() const noexcept { return cnt; }
    constexpr Arr<itype::u32> extract(itype::u32 n) {
#ifndef NDEBUG
        if (n >= size()) throw gsh::Exception("gsh::PotentializedUnionFind::extract / The index is out of range. ( n=", n, ", size=", size(), " )");
#endif
        const itype::i32 nr = root(n);
        itype::u32 ccnt = 0;
        for (itype::u32 i = 0; i != size(); ++i) ccnt += root(i) == nr;
        Arr<itype::u32> res(ccnt);
        for (itype::u32 i = 0, j = 0; i != size(); ++i)
            if (i == static_cast<itype::u32>(nr) || parent[i] == nr) res[j++] = i;
        return res;
    }
    constexpr Arr<Arr<itype::u32>> groups() {
        Arr<itype::u32> key(size());
        itype::u32 cnt = 0;
        for (itype::u32 i = 0; i != size(); ++i) {
            if (parent[i] < 0) key[i] = cnt++;
        }
        Arr<itype::u32> cnt2(cnt);
        for (itype::u32 i = 0; i != size(); ++i) ++cnt2[key[root(i)]];
        Arr<Arr<itype::u32>> res(cnt);
        for (itype::u32 i = 0; i != cnt; ++i) {
            res[i].resize(cnt2[i]);
            cnt2[i] = 0;
        }
        for (itype::u32 i = 0; i != size(); ++i) {
            const itype::u32 idx = key[parent[i] < 0 ? i : parent[i]];
            res[idx][cnt2[idx]++] = i;
        }
        return res;
    }
};

/*
class OfflinePersistentUnionfind {
    itype::u32 n = 0;
    struct query {
        bool t;
        itype::u32 k, u, v;
    };
    Vec<query> g;
public:
    constexpr OfflinePersistentUnionfind() {}
    constexpr OfflinePersistentUnionfind(itype::u32 m) : n(m) {}
    constexpr itype::u32 size() const noexcept { return n; }
    constexpr void same(itype::i32 k, itype::u32 u, itype::u32 v) { g.emplace_back(true, k + 1, u, v); }
    constexpr void merge(itype::i32 k, itype::u32 u, itype::u32 v) { g.emplace_back(false, k + 1, u, v); }
    constexpr void reserve(itype::u32 q) { g.reserve(q); }
    constexpr Arr<bool> solve() const {
        struct solver {
            Arr<itype::u32> cnt;
            Arr<query> gr;
            RollbackUnionFind ds;
            Arr<itype::u8> res;
            Arr<bool> res2;
            constexpr void dfs(itype::u32 idx) {
                auto [t, k, u, v] = gr[idx];
                if (t) {
                    res[k - 1] = ds.same(u, v) + 1;
                } else {
                    ds.merge(u, v);
                    for (itype::u32 i = cnt[k], j = cnt[k + 1]; i != j; ++i) dfs(i);
                    ds.undo();
                }
            }
            constexpr solver(itype::u32 n, const Vec<query>& g) : cnt(g.size() + 2), gr(g.size() + 1), ds(n + 1), res(g.size()) {
                const itype::u32 q = g.size();
                itype::u32 same_cnt = 0;
                for (itype::u32 i = 0; i != q; ++i) {
                    ++cnt[g[i].k];
                    same_cnt += g[i].t;
                }
                for (itype::u32 i = 0; i != q; ++i) cnt[i + 1] += cnt[i];
                cnt[q + 1] = cnt[q];
                for (itype::u32 i = q; i != 0; --i) {
                    itype::u32 idx = --cnt[g[i - 1].k];
                    (gr[idx] = g[i - 1]).k = i;
                }
                gr[q] = { false, 0, n, n };
                ds.reserve(q - same_cnt + 1);
                dfs(q);
                res2.resize(same_cnt);
                itype::u32 s = 0;
                for (itype::u32 i = 0; i != q; ++i) {
                    bool f = res[i] != 0;
                    res2[s] = (f ? res[i] - 1 : res2[s]);
                    s += f;
                }
            }
        } sl(n, g);
        return std::move(sl.res2);
    }
};
*/

}  // namespace gsh
