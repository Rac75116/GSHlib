#pragma once
#include "TypeDef.hpp"     // gsh::itype
#include "Arr.hpp"         // gsh::Arr
#include "Vec.hpp"         // gsh::Vec
#include "Exception.hpp"   // gsh::Exception
#include "Functional.hpp"  // gsh::Plus, gsh::Negate

namespace gsh {

namespace internal {
    template<class D> class UnionFindImpl {
        D& derived() noexcept { return static_cast<D&>(*this); }
        const D& derived() const noexcept { return static_cast<const D&>(*this); }
    public:
        constexpr itype::u32 size() const noexcept { return derived().parent.size(); }
        [[nodiscard]] constexpr bool empty() const noexcept { return derived().parent.empty(); }
        constexpr itype::u32 leader(itype::u32 n) {
#ifndef NDEBUG
            if (n >= derived().parent.size()) throw gsh::Exception("gsh::internal::UnionFindImpl::leader / The index is out of range. ( n=", n, ", size=", derived().parent.size(), " )");
#endif
            return derived().root(n);
        }
        constexpr bool is_leader(itype::u32 n) const {
#ifndef NDEBUG
            if (n >= derived().parent.size()) throw gsh::Exception("gsh::internal::UnionFindImpl::is_leader / The index is out of range. ( n=", n, ", size=", derived().parent.size(), " )");
#endif
            return derived().parent[n] < 0;
        }
        constexpr bool same(itype::u32 a, itype::u32 b) {
#ifndef NDEBUG
            if (a >= derived().parent.size() || b >= derived().parent.size()) throw gsh::Exception("gsh::UnionFind::same / The index is out of range. ( a=", a, ", b=", b, ", size=", derived().parent.size(), " )");
#endif
            return derived().root(a) == derived().root(b);
        }
    protected:
        GSH_INTERNAL_INLINE constexpr itype::u32 merge_impl(itype::u32 ar, itype::u32 br) noexcept {
            const itype::i32 sa = derived().parent[ar], sb = derived().parent[br];
            const itype::i32 tmp1 = sa < sb ? ar : br, tmp2 = sa < sb ? br : ar;
            derived().parent[tmp1] += derived().parent[tmp2];
            derived().parent[tmp2] = tmp1;
            --derived().cnt;
            return tmp1;
        }
    public:
        constexpr itype::u32 group_size(itype::u32 n) {
#ifndef NDEBUG
            if (n >= derived().parent.size()) throw gsh::Exception("gsh::internal::UnionFindImpl::group_size(itype::u32) / The index is out of range. ( n=", n, ", size=", derived().parent.size(), " )");
#endif
            return -derived().parent[derived().root(n)];
        }
        constexpr itype::u32 max_group_size() const noexcept {
            itype::i32 res = 1;
            for (itype::u32 i = 0; i != size(); ++i) {
                res = -derived().parent[i] < res ? res : -derived().parent[i];
            }
            return res;
        }
        constexpr itype::u32 count_groups() const noexcept { return derived().cnt; }
        constexpr Arr<itype::u32> extract(itype::u32 n) {
#ifndef NDEBUG
            if (n >= derived().parent.size()) throw gsh::Exception("gsh::internal::UnionFindImpl::extract / The index is out of range. ( n=", n, ", size=", derived().parent.size(), " )");
#endif
            const itype::i32 nr = derived().root(n);
            itype::u32 ccnt = 0;
            for (itype::u32 i = 0; i != derived().parent.size(); ++i) ccnt += derived().root(i) == nr;
            Arr<itype::u32> res(ccnt);
            for (itype::u32 i = 0, j = 0; i != derived().parent.size(); ++i)
                if (i == static_cast<itype::u32>(nr) || derived().parent[i] == nr) res[j++] = i;
            return res;
        }
        constexpr Arr<Arr<itype::u32>> groups() {
            Arr<itype::u32> key(derived().parent.size());
            itype::u32 cnt = 0;
            for (itype::u32 i = 0; i != derived().parent.size(); ++i) {
                if (derived().parent[i] < 0) key[i] = cnt++;
            }
            Arr<itype::u32> cnt2(cnt);
            for (itype::u32 i = 0; i != derived().parent.size(); ++i) ++cnt2[key[derived().root(i)]];
            Arr<Arr<itype::u32>> res(cnt);
            for (itype::u32 i = 0; i != cnt; ++i) {
                res[i].resize(cnt2[i]);
                cnt2[i] = 0;
            }
            for (itype::u32 i = 0; i != derived().parent.size(); ++i) {
                const itype::u32 idx = key[derived().parent[i] < 0 ? i : derived().parent[i]];
                res[idx][cnt2[idx]++] = i;
            }
            return res;
        }
    };
}  // namespace internal

class UnionFind : public internal::UnionFindImpl<UnionFind> {
    friend class internal::UnionFindImpl<UnionFind>;
    Arr<itype::i32> parent;
    itype::u32 cnt = 0;
    constexpr itype::i32 root(itype::i32 n) noexcept {
        if (parent[n] < 0) return n;
        return parent[n] = root(parent[n]);
    }
public:
    using size_type = itype::u32;
    constexpr UnionFind() noexcept {}
    constexpr explicit UnionFind(itype::u32 n) : parent(n, -1), cnt(n) {}
    constexpr void reset() noexcept {
        cnt = parent.size();
        for (itype::u32 i = 0; i != cnt; ++i) parent[i] = -1;
    }
    constexpr void resize(itype::u32 n) {
        if (n < size()) throw gsh::Exception("gsh::UnionFind::resize / It cannot be smaller than it is now.");
        cnt += n - size();
        parent.resize(n, -1);
    }
    constexpr itype::u32 merge(const itype::u32 a, const itype::u32 b) {
#ifndef NDEBUG
        if (a >= parent.size() || b >= parent.size()) throw gsh::Exception("gsh::UnionFind::merge / The index is out of range. ( a=", a, ", b=", b, ", size=", parent.size(), " )");
#endif
        const itype::i32 ar = root(a), br = root(b);
        if (ar == br) return ar;
        return merge_impl(ar, br);
    }
    constexpr bool merge_same(const itype::u32 a, const itype::u32 b) {
#ifndef NDEBUG
        if (a >= size() || b >= size()) throw gsh::Exception("gsh::UnionFind::merge_same / The index is out of range. ( a=", a, ", b=", b, ", size=", size(), " )");
#endif
        const itype::i32 ar = root(a), br = root(b);
        if (ar == br) return true;
        merge_impl(ar, br);
        return false;
    }
};

class RollbackUnionFind : public internal::UnionFindImpl<RollbackUnionFind> {
    friend class internal::UnionFindImpl<RollbackUnionFind>;
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
    constexpr RollbackUnionFind() noexcept {}
    constexpr explicit RollbackUnionFind(itype::u32 n) : parent(n, -1), cnt(n) {}
    constexpr void reset() noexcept {
        cnt = parent.size();
        history.clear();
        for (itype::u32 i = 0; i != parent.size(); ++i) parent[i] = -1;
    }
    constexpr void reserve(itype::u32 q) { history.reserve(history.size() + q); }
    constexpr void resize(itype::u32 n) {
        if (n < parent.size()) throw gsh::Exception("gsh::RollbackUnionFind::resize / It cannot be smaller than it is now.");
        cnt += n - parent.size();
        parent.resize(n, -1);
    }
    constexpr itype::u32 merge(const itype::u32 a, const itype::u32 b) {
#ifndef NDEBUG
        if (a >= parent.size() || b >= parent.size()) throw gsh::Exception("gsh::RollbackUnionFind::merge / The index is out of range. ( a=", a, ", b=", b, ", size=", parent.size(), " )");
#endif
        const itype::i32 ar = root(a), br = root(b);
        history.emplace_back(ar, br, parent[ar], parent[br], ar != br);
        if (ar == br) return ar;
        return merge_impl(ar, br);
    }
    constexpr bool merge_same(const itype::u32 a, const itype::u32 b) {
#ifndef NDEBUG
        if (a >= parent.size() || b >= parent.size()) throw gsh::Exception("gsh::RollbackUnionFind::merge_same / The index is out of range. ( a=", a, ", b=", b, ", size=", parent.size(), " )");
#endif
        const itype::i32 ar = root(a), br = root(b);
        history.emplace_back(ar, br, parent[ar], parent[br], ar != br);
        if (ar == br) return true;
        merge_impl(ar, br);
        return false;
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
    class state {
        friend class RollbackUnionFind;
        itype::u32 s;
        state() = delete;
        constexpr state(itype::u32 n) noexcept : s(n) {}
    public:
        constexpr state(const state&) noexcept = default;
        constexpr state& operator=(const state&) noexcept = default;
        constexpr state next(itype::i32 n) const noexcept { return state{ s + n }; }
        constexpr state prev(itype::i32 n) const noexcept { return state{ s - n }; }
        constexpr void advance(itype::i32 n) noexcept { s += n; }
    };
    constexpr state current() const noexcept { return state{ history.size() }; }
    constexpr void rollback(state st) {
        while (st.s < history.size()) {
            auto [a, b, c, d, e] = history.back();
            history.pop_back();
            parent[a] = c, parent[b] = d;
            cnt += e;
        }
    }
};

template<class T = itype::i64, class F = Plus, class I = Negate> class PotentializedUnionFind : public internal::UnionFindImpl<PotentializedUnionFind<T, F, I>> {
    friend class internal::UnionFindImpl<PotentializedUnionFind<T, F, I>>;
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
    using value_type = T;
    using size_type = itype::u32;
    constexpr PotentializedUnionFind() noexcept(std::is_nothrow_default_constructible_v<F> && std::is_nothrow_default_constructible_v<I> && std::is_nothrow_default_constructible_v<T>) {}
    constexpr explicit PotentializedUnionFind(F f, I i = I(), const T& e = T()) : func(f), inv(i), el(e) {}
    constexpr explicit PotentializedUnionFind(itype::u32 n, F f = F(), I i = I(), const T& e = T()) : parent(n, -1), diff(n, e), cnt(n), func(f), inv(i), el(e) {}
    constexpr void reset() {
        cnt = parent.size();
        for (itype::u32 i = 0; i != parent.size(); ++i) parent[i] = -1;
        diff.assign(parent.size(), el);
    }
    constexpr void resize(itype::u32 n) {
        if (n < parent.size()) throw gsh::Exception("gsh::PotentializedUnionFind::resize / It cannot be smaller than it is now.");
        cnt += n - parent.size();
        parent.resize(n, -1);
        diff.resize(n, el);
    }
    constexpr const T& operator[](itype::u32 n) { return potential(n); }
    constexpr const T& potential(itype::u32 n) {
#ifndef NDEBUG
        if (n >= parent.size()) throw gsh::Exception("gsh::PotentializedUnionFind::potential / The index is out of range. ( n=", n, ", size=", parent.size(), " )");
#endif
        root(n);
        return diff[n];
    }
    // A[a] = func(A[b], result)
    constexpr T potential(itype::u32 a, itype::u32 b) {
#ifndef NDEBUG
        if (a >= parent.size() || b >= parent.size()) throw gsh::Exception("gsh::PotentializedUnionFind::potential / The index is out of range. ( a=", a, ", b=", b, ", size=", parent.size(), " )");
#endif
        root(a);
        root(b);
        return Invoke(func, Invoke(inv, diff[b]), diff[a]);
    }
    // A[a] = func(A[b], w) return leader(a)
    constexpr itype::u32 merge(const itype::u32 a, const itype::u32 b, const T& w) {
#ifndef NDEBUG
        if (a >= parent.size() || b >= parent.size()) throw gsh::Exception("gsh::PotentializedUnionFind::merge / The index is out of range. ( a=", a, ", b=", b, ", size=", parent.size(), " )");
#endif
        const itype::i32 ar = root(a), br = root(b);
        if (ar == br) return ar;
        const itype::i32 sa = parent[ar], sb = parent[br];
        const bool f = sa < sb;
        const itype::i32 tmp1 = f ? ar : br, tmp2 = f ? br : ar;
        parent[tmp1] += parent[tmp2];
        parent[tmp2] = tmp1;
        diff[tmp2] = Invoke(func, diff[f ? a : b], Invoke(inv, Invoke(func, diff[f ? b : a], f ? w : Invoke(inv, w))));
        --cnt;
        return tmp1;
    }
    // A[a] = func(A[b], w) return same(a, b)
    constexpr bool merge_same(const itype::u32 a, const itype::u32 b, const T& w) {
#ifndef NDEBUG
        if (a >= parent.size() || b >= parent.size()) throw gsh::Exception("gsh::PotentializedUnionFind::merge_same / The index is out of range. ( a=", a, ", b=", b, ", size=", parent.size(), " )");
#endif
        const itype::i32 ar = root(a), br = root(b);
        if (ar == br) return true;
        const itype::i32 sa = parent[ar], sb = parent[br];
        const bool f = sa < sb;
        const itype::i32 tmp1 = f ? ar : br, tmp2 = f ? br : ar;
        parent[tmp1] += parent[tmp2];
        parent[tmp2] = tmp1;
        diff[tmp2] = Invoke(func, diff[f ? a : b], Invoke(inv, Invoke(func, diff[f ? b : a], f ? w : Invoke(inv, w))));
        --cnt;
        return false;
    }
    // A[a] = func(A[b], w)
    constexpr bool merge_valid(const itype::u32 a, const itype::u32 b, const T& w) {
#ifndef NDEBUG
        if (a >= parent.size() || b >= parent.size()) throw gsh::Exception("gsh::PotentializedUnionFind::merge_valid / The index is out of range. ( a=", a, ", b=", b, ", size=", parent.size(), " )");
#endif
        const itype::i32 ar = root(a), br = root(b);
        if (ar == br) return diff[a] == Invoke(func, diff[b], w);
        const itype::i32 sa = parent[ar], sb = parent[br];
        const bool f = sa < sb;
        const itype::i32 tmp1 = f ? ar : br, tmp2 = f ? br : ar;
        parent[tmp1] += parent[tmp2];
        parent[tmp2] = tmp1;
        diff[tmp2] = Invoke(func, diff[f ? a : b], Invoke(inv, Invoke(func, diff[f ? b : a], f ? w : Invoke(inv, w))));
        --cnt;
        return true;
    }
};

template<class T, class F> class MonoidalUnionFind : public internal::UnionFindImpl<MonoidalUnionFind<T, F>> {
    friend class internal::UnionFindImpl<MonoidalUnionFind<T, F>>;
    Arr<itype::i32> parent;
    Arr<T> monoid;
    [[no_unique_address]] F func;
    itype::u32 cnt = 0;
    constexpr itype::i32 root(itype::i32 n) noexcept {
        if (parent[n] < 0) return n;
        return parent[n] = root(parent[n]);
    }
public:
    using value_type = T;
    using size_type = itype::u32;
    constexpr MonoidalUnionFind() noexcept {}
    constexpr explicit MonoidalUnionFind(F f) : func(std::move(f)) {}
    constexpr explicit MonoidalUnionFind(itype::u32 n, F f = F()) : parent(n, -1), monoid(n), func(std::move(f)), cnt(n) {}
    constexpr MonoidalUnionFind(itype::u32 n, F f, const T& init) : parent(n, -1), monoid(n, init), func(std::move(f)), cnt(n) {}
    constexpr ~MonoidalUnionFind() {
        for (itype::u32 i = 0; i != parent.size(); ++i) {
            if (parent[i] < 0) std::destroy_at(&monoid[i]);
        }
        monoid.reset();
    }
    constexpr void reset() {
        cnt = parent.size();
        for (itype::u32 i = 0; i != parent.size(); ++i) {
            if (parent[i] < 0) std::destroy_at(&monoid[i]);
            parent[i] = -1;
            std::construct_at(&monoid[i]);
        }
    }
    constexpr void reset(const T& init) {
        cnt = parent.size();
        for (itype::u32 i = 0; i != parent.size(); ++i) {
            if (parent[i] < 0) std::destroy_at(&monoid[i]);
            parent[i] = -1;
            std::construct_at(&monoid[i], init);
        }
    }
    constexpr void resize(itype::u32 n) {
        if (n < parent.size()) throw gsh::Exception("gsh::MonoidalUnionFind::resize / It cannot be smaller than it is now.");
        cnt += n - parent.size();
        parent.resize(n, -1);
        monoid.resize(n);
    }
    constexpr void resize(itype::u32 n, const T& value) {
        if (n < parent.size()) throw gsh::Exception("gsh::MonoidalUnionFind::resize / It cannot be smaller than it is now.");
        cnt += n - parent.size();
        parent.resize(n, -1);
        monoid.resize(n, value);
    }
    constexpr T& operator[](itype::u32 n) {
#ifndef NDEBUG
        if (n >= parent.size()) throw gsh::Exception("gsh::MonoidalUnionFind::operator[] / The index is out of range. ( n=", n, ", size=", parent.size(), " )");
#endif
        return monoid[root(n)];
    }
    constexpr itype::u32 merge(const itype::u32 a, const itype::u32 b) {
#ifndef NDEBUG
        if (a >= parent.size() || b >= parent.size()) throw gsh::Exception("gsh::MonoidalUnionFind::merge / The index is out of range. ( a=", a, ", b=", b, ", size=", parent.size(), " )");
#endif
        const itype::i32 ar = root(a), br = root(b);
        if (ar == br) return ar;
        const itype::i32 sa = parent[ar], sb = parent[br];
        const bool f = sa < sb;
        const itype::i32 tmp1 = f ? ar : br, tmp2 = f ? br : ar;
        parent[tmp1] += parent[tmp2];
        parent[tmp2] = tmp1;
        monoid[tmp1] = Invoke(func, std::move(monoid[tmp1]), std::move(monoid[tmp2]));
        std::destroy_at(&monoid[tmp2]);
        --cnt;
        return tmp1;
    }
    constexpr bool merge_same(const itype::u32 a, const itype::u32 b) {
#ifndef NDEBUG
        if (a >= parent.size() || b >= parent.size()) throw gsh::Exception("gsh::MonoidalUnionFind::merge / The index is out of range. ( a=", a, ", b=", b, ", size=", parent.size(), " )");
#endif
        const itype::i32 ar = root(a), br = root(b);
        if (ar == br) return true;
        const itype::i32 sa = parent[ar], sb = parent[br];
        const bool f = sa < sb;
        const itype::i32 tmp1 = f ? ar : br, tmp2 = f ? br : ar;
        parent[tmp1] += parent[tmp2];
        parent[tmp2] = tmp1;
        monoid[tmp1] = Invoke(func, std::move(monoid[tmp1]), std::move(monoid[tmp2]));
        std::destroy_at(&monoid[tmp2]);
        --cnt;
        return false;
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
