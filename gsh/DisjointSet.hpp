#pragma once
#include <gsh/TypeDef.hpp>    // gsh::itype
#include <gsh/Arr.hpp>        // gsh::Arr
#include <gsh/Exception.hpp>  // gsh::Exception

namespace gsh {

class DisjointSet {
    Arr<itype::i32> parent;
    itype::u32 cnt = 0;
private:
    constexpr itype::i32 root(itype::i32 n) noexcept {
        if (parent.at_unchecked(n) < 0) return n;
        return parent.at_unchecked(n) = root(parent.at_unchecked(n));
    }
public:
    using size_type = itype::u32;
    constexpr DisjointSet() {}
    constexpr explicit DisjointSet(size_type n) : parent(n, -1), cnt(n) {}
    constexpr DisjointSet(const DisjointSet&) = default;
    constexpr DisjointSet(DisjointSet&&) = default;
    constexpr DisjointSet& operator=(const DisjointSet&) = default;
    constexpr DisjointSet& operator=(DisjointSet&&) = default;
    constexpr void reset() {
        for (itype::u32 i = 0; i != size(); ++i) parent.at_unchecked(i) = -1;
    }
    constexpr size_type size() const noexcept { return parent.size(); }
    [[nodiscard]] constexpr bool empty() const noexcept { return parent.empty(); }
    constexpr void resize(size_type n) {
        if (n < size()) throw gsh::Exception("gsh::DisjointSet::resize / It cannot be smaller than it is now.");
        cnt += n - size();
        parent.resize(n, -1);
    }
    constexpr size_type leader(size_type n) {
#ifndef NDEBUG
        if (n >= size()) throw gsh::Exception("gsh::DisjointSet::leader / The index is out of range. ( n=", n, ", size=", size(), " )");
#endif
        return root(n);
    }
    constexpr bool is_leader(size_type n) const {
#ifndef NDEBUG
        if (n >= size()) throw gsh::Exception("gsh::DisjointSet::is_leader / The index is out of range. ( n=", n, ", size=", size(), " )");
#endif
        return parent.at_unchecked(n) < 0;
    }
    constexpr bool same(size_type a, size_type b) {
#ifndef NDEBUG
        if (a >= size() || b >= size()) throw gsh::Exception("gsh::DisjointSet::same / The index is out of range. ( a=", a, ", b=", b, ", size=", size(), " )");
#endif
        return root(a) == root(b);
    }
    constexpr bool merge(const size_type a, const size_type b) {
#ifndef NDEBUG
        if (a >= size() || b >= size()) throw gsh::Exception("gsh::DisjointSet::merge / The index is out of range. ( a=", a, ", b=", b, ", size=", size(), " )");
#endif
        const itype::i32 ar = root(a), br = root(b);
        if (ar == br) return false;
        const itype::i32 sa = parent.at_unchecked(ar), sb = parent.at_unchecked(br);
        const itype::i32 tmp1 = sa < sb ? ar : br, tmp2 = sa < sb ? br : ar;
        parent.at_unchecked(tmp1) += parent.at_unchecked(tmp2);
        parent.at_unchecked(tmp2) = tmp1;
        --cnt;
        return true;
    }
    constexpr size_type size(size_type n) {
#ifndef NDEBUG
        if (n >= size()) throw gsh::Exception("gsh::DisjointSet::size(size_type) / The index is out of range. ( n=", n, ", size=", size(), " )");
#endif
        return -parent.at_unchecked(root(n));
    }
    constexpr size_type count_groups() const noexcept { return cnt; }
    constexpr Arr<size_type> extract(size_type n) {
#ifndef NDEBUG
        if (n >= size()) throw gsh::Exception("gsh::DisjointSet::extract / The index is out of range. ( n=", n, ", size=", size(), " )");
#endif
        const itype::i32 nr = root(n);
        itype::u32 ccnt = 0;
        for (itype::u32 i = 0; i != size(); ++i) ccnt += root(i) == nr;
        Arr<itype::u32> res(ccnt);
        for (itype::u32 i = 0, j = 0; i != size(); ++i)
            if (i == static_cast<itype::u32>(nr) || parent.at_unchecked(i) == nr) res[j++] = i;
        return res;
    }
    constexpr Arr<Arr<size_type>> groups() {
        Arr<itype::u32> key(size());
        itype::u32 cnt = 0;
        for (itype::u32 i = 0; i != size(); ++i) {
            if (parent.at_unchecked(i) < 0) key.at_unchecked(i) = cnt++;
        }
        Arr<itype::u32> cnt2(cnt);
        for (itype::u32 i = 0; i != size(); ++i) ++cnt2.at_unchecked(key.at_unchecked(root(i)));
        Arr<Arr<itype::u32>> res(cnt);
        for (itype::u32 i = 0; i != cnt; ++i) {
            res.at_unchecked(i).resize(cnt2.at_unchecked(i));
            cnt2.at_unchecked(i) = 0;
        }
        for (itype::u32 i = 0; i != size(); ++i) {
            const itype::u32 idx = key.at_unchecked(parent.at_unchecked(i) < 0 ? i : parent.at_unchecked(i));
            res.at_unchecked(idx).at_unchecked(cnt2.at_unchecked(idx)++) = i;
        }
        return res;
    }
};

}  // namespace gsh