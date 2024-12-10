#pragma once
#include <type_traits>             // std::is_arithmetic_v
#include <bit>                     // std::bit_cast
#include <cmath>                   // std::hypot, std::sqrt
#include "TypeDef.hpp"             // gsh::itype
#include "Range.hpp"               // gsh::Rangeof
#include "Arr.hpp"                 // gsh::Arr
#include "Range.hpp"               // gsh::Rangeof, gsh::RangeTraits
#include "Algorithm.hpp"           // gsh::internal::SortUnsigned64
#include "Random.hpp"              // gsh::RandBuffer32
#include "Int128.hpp"              // gsh::internal::Mulu128, gsh::internal::Divu128
#include "internal/Operation.hpp"  // gsh::internal::ArithmeticInterface

namespace gsh {

template<class T>
    requires std::is_arithmetic_v<T>
class Point2 : public internal::ArithmeticInterface<Point2<T>> {
public:
    T x{}, y{};
    constexpr Point2() {}
    constexpr Point2(const T& a, const T& b) : x(a), y(b) {}
    using value_type = T;
    constexpr Point2& operator+=(const Point2& p) noexcept {
        x += p.x, y += p.y;
        return *this;
    }
    constexpr Point2& operator-=(const Point2& p) noexcept {
        x -= p.x, y -= p.y;
        return *this;
    }
    friend constexpr bool operator==(const Point2& a, const Point2& b) { return a.x == b.x && a.y == b.y; }
};
template<class T = ftype::f64, class U> constexpr T Norm(const Point2<U>& a) {
    return std::hypot(static_cast<T>(a.x), static_cast<T>(a.y));
}
template<class T> constexpr T NormSquare(const Point2<T>& a) {
    return a.x * a.x + a.y * a.y;
}
template<class T> constexpr T Dot(const Point2<T>& a, const Point2<T>& b) {
    return a.x * b.x + a.y * b.y;
}
template<class T> constexpr T Cross(const Point2<T>& a, const Point2<T>& b) {
    return a.x * b.y + a.y * b.x;
}
template<class T, class U> constexpr T NormSquare(const Point2<U>& a) {
    return static_cast<T>(a.x) * static_cast<T>(a.x) + static_cast<T>(a.y) * static_cast<T>(a.y);
}
template<class T, class U> constexpr T Dot(const Point2<U>& a, const Point2<U>& b) {
    return static_cast<T>(a.x) * static_cast<T>(b.x) + static_cast<T>(a.y) * static_cast<T>(b.y);
}
template<class T, class U> constexpr T Cross(const Point2<U>& a, const Point2<U>& b) {
    return static_cast<T>(a.x) * static_cast<T>(b.y) - static_cast<T>(a.y) * static_cast<T>(b.x);
}

template<class T>
    requires std::is_arithmetic_v<T>
class Point3 : public internal::ArithmeticInterface<Point3<T>> {
public:
    T x{}, y{}, z{};
    constexpr Point3() {}
    constexpr Point3(const T& a, const T& b, const T& c) : x(a), y(b), z(c) {}
    using value_type = T;
    constexpr Point3& operator+=(const Point3& p) {
        x += p.x, y += p.y, z += p.z;
        return *this;
    }
    constexpr Point3& operator-=(const Point3& p) {
        x -= p.x, y -= p.y, z -= p.z;
        return *this;
    }
    friend constexpr bool operator==(const Point3& a, const Point3& b) { return a.x == b.x && a.y == b.y && a.z == b.z; }
};
template<class T = ftype::f64, class U> constexpr T Norm(const Point3<U>& a) {
    return std::hypot(static_cast<T>(a.x), static_cast<T>(a.y), static_cast<T>(a.z));
}
template<class T> constexpr T NormSquare(const Point3<T>& a) {
    return a.x * a.x + a.y * a.y + a.z * a.z;
}
template<class T> constexpr T Dot(const Point3<T>& a, const Point3<T>& b) {
    return a.x * b.x + a.y * b.y + a.z * b.z;
}
template<class T> constexpr Point3<T> Cross(const Point3<T>& a, const Point3<T>& b) {
    return { a.y * b.z - a.z * b.y, a.z * b.x - a.x * b.z, a.x * b.y - a.y * b.x };
}
template<class T, class U> constexpr T NormSquare(const Point3<U>& a) {
    return static_cast<T>(a.x) * static_cast<T>(a.x) + static_cast<T>(a.y) * static_cast<T>(a.y) + static_cast<T>(a.z) * static_cast<T>(a.z);
}
template<class T, class U> constexpr T Dot(const Point3<U>& a, const Point3<U>& b) {
    return static_cast<T>(a.x) * static_cast<T>(b.x) + static_cast<T>(a.y) * static_cast<T>(b.y) + static_cast<T>(a.z) * static_cast<T>(b.z);
}
template<class T, class U> constexpr Point3<T> Cross(const Point3<U>& a, const Point3<U>& b) {
    return { static_cast<T>(a.y) * static_cast<T>(b.z) - static_cast<T>(a.z) * static_cast<T>(b.y), static_cast<T>(a.z) * static_cast<T>(b.x) - static_cast<T>(a.x) * static_cast<T>(b.z), static_cast<T>(a.x) * static_cast<T>(b.y) - static_cast<T>(a.y) * static_cast<T>(b.x) };
}


template<Rangeof<Point2<itype::i32>> T> constexpr Arr<Point2<itype::i32>> ArgumentSort(T&& r) {
    Arr<itype::u128> v(RangeTraits<T>::size(r));
    for (itype::u32 i = 0; auto&& p : r) {
        auto [x, y] = p;
        itype::u64 ord = 0;
        const bool xs = (x >= 0), ys = (y >= 0);
        const itype::u64 xu = (xs ? x : -x), yu = (ys ? y : -y);
        const itype::u64 mx = (xu < yu ? yu : xu), mn = (xu < yu ? xu : yu);
        const bool rev = (xs ^ ys) ^ (xu < yu);
        const itype::u64 id = ys * 4ull + (xs ^ ys) * 2ull + rev;
        if (x == 0 && y == 0) ord = 4ull << 61;
        else {
            constexpr itype::u64 li = (1ull << 61) - 1;
            auto [hi, lo] = internal::Mulu128(mn, li);
            const itype::u64 tmp = internal::Divu128(hi, lo, mx).first;
            ord = id << 61 | (rev ? li - tmp : tmp);
        }
        v[i++] = static_cast<itype::u128>(std::bit_cast<itype::u64>(p)) << 64 | ord;
    }
    internal::SortUnsigned64(v.data(), v.size());
    Arr<Point2<itype::i32>> res(v.size());
    for (itype::u32 i = 0, j = v.size(); i != j; ++i) res[i] = std::bit_cast<Point2<itype::i32>>(static_cast<itype::u64>(v[i] >> 64));
    return res;
}

template<Rangeof<Point2<itype::i32>> T> constexpr Arr<Point2<itype::i32>> ConvexHull(T&& r) {
    const itype::u32 n = RangeTraits<T>::size(r);
    if (n <= 1) return r;
    itype::u32 m = 1;
    Arr<Point2<itype::i32>> p(n);
    {
        Arr<itype::u64> sorted(n);
        for (itype::u32 i = 0; auto e : r) sorted[i++] = std::bit_cast<itype::u64>(e) ^ 0x8000000080000000;
        if (n < 5000) std::sort(sorted.begin(), sorted.end());
        else internal::SortUnsigned64(sorted.data(), n);
        p[0] = std::bit_cast<Point2<itype::i32>>(sorted[0] ^ 0x8000000080000000);
        for (itype::u32 i = 1; i != n; ++i) {
            p[m] = std::bit_cast<Point2<itype::i32>>(sorted[i] ^ 0x8000000080000000);
            m += sorted[i] != sorted[i - 1];
        }
    }
    if (m <= 2) {
        p.resize(m);
        return p;
    }
    Arr<Point2<itype::i32>> ch(2 * m);
    itype::u32 k = 0;
    for (itype::u32 i = 0; i < m; ch[k++] = p[i++]) {
        while (k >= 2 && Cross<itype::i64>(ch[k - 1] - ch[k - 2], p[i] - ch[k - 2]) <= 0) --k;
    }
    for (itype::u32 i = m - 1, t = k + 1; i > 0; ch[k++] = p[--i]) {
        while (k >= t && Cross<itype::i64>(ch[k - 1] - ch[k - 2], p[i - 1] - ch[k - 2]) <= 0) --k;
    }
    ch.resize(k - 1);
    return ch;
}

template<RandomAccessRange T>
    requires Rangeof<T, Point2<itype::i32>>
constexpr auto ConvexDiameter(T&& p) {
    using traits = RangeTraits<T>;
    struct result_type {
        Point2<itype::i32> a, b;
        template<class U = ftype::f64> constexpr auto distance() const noexcept { return Norm<U>(a - b); }
        constexpr const auto& first() const noexcept { return a; }
        constexpr const auto& second() const noexcept { return b; }
    };
    const itype::u32 n = traits::size(p);
    if (n == 0) throw Exception("gsh::ConvexDiameter / Input is empty.");
    const auto bg = traits::begin(p);
    if (n <= 2) {
        if (n == 1) return result_type{ *bg, *bg };
        else return result_type{ *bg, *std::next(bg) };
    }
    itype::u32 is = 0, js = 0;
    for (itype::u32 i = 1; i != n; i++) {
        auto a = std::next(bg, i)->y, b = std::next(bg, is)->y, c = std::next(bg, js)->y;
        is = (a > b ? i : is);
        js = (a < c ? i : js);
    }
    itype::i64 maxdis = NormSquare<itype::i64>(*std::next(bg, is) - *std::next(bg, js));
    itype::u32 maxi = is, maxj = js, i = is, j = js;
    do {
        const itype::u32 in = (i + 1 == n ? 0 : i + 1), jn = (j + 1 == n ? 0 : j + 1);
        const bool f = Cross<itype::i64>(*std::next(bg, in) - *std::next(bg, i), *std::next(bg, jn) - *std::next(bg, j)) > 0;
        j = f ? jn : j;
        i = f ? i : in;
        const itype::i64 tmp = NormSquare<itype::i64>(*std::next(bg, i) - *std::next(bg, j));
        const bool g = tmp > maxdis;
        maxdis = g ? tmp : maxdis;
        maxi = g ? i : maxi;
        maxj = g ? j : maxj;
    } while (i != is || j != js);
    return result_type{ *std::next(bg, maxi), *std::next(bg, maxj) };
}

template<Rangeof<Point2<itype::i32>> T> auto FurthestPair(T&& r) {
    return ConvexDiameter(ConvexHull(r));
}

/*
template<RandomAccessRange T>
    requires Rangeof<T, Point2<itype::i32>>
constexpr auto ClosestPair(T&& r) {
    using traits = RangeTraits<T>;
    struct result_type {
        Point2<itype::i32> a, b;
        template<class U = ftype::f64> constexpr auto distance() const noexcept { return Norm<U>(a - b); }
        constexpr const auto& first() const noexcept { return a; }
        constexpr const auto& second() const noexcept { return b; }
    };
    const auto n = traits::size(r);
    if (n == 0) throw Exception("gsh::ConvexDiameter / Input is empty.");
    const auto bg = traits::begin(r);
    if (n == 1) return result_type{ *bg, *std::next(bg) };
    RandBuffer32<64> engine;
    itype::u64 d = -1;
    result_type res;
    for (itype::u32 i = 0; i != (n + 31) / 32; ++i) {
        engine.init();
        for (itype::u32 j = 0; j != 32; ++j) {
            itype::u32 a = Uniform32(engine, n), b = Uniform32(engine, n - 1);
            b += b >= a;
            const auto &&c = *std::next(bg, a), d = *std::next(bg, b);
            const itype::u64 tmp = NormSquare<itype::u64>(c - d);
            d = tmp < d ? tmp : d;
            res = tmp < d ? result_type{ c, d } : res;
        }
    }
    if (d == 0) return res;
    std::unordered_multimap<itype::u64, Point2<itype::i32>> m;
    m.reserve(n);
    for (auto&& p : r) {
        const itype::u64 key = (static_cast<itype::u64>((std::bit_cast<itype::u32>(p.x) ^ (1u << 31)) / d) << 32) | ((std::bit_cast<itype::u32>(p.y) ^ (1u << 31)) / d);
        m.emplace(key, p);
    }
    const auto ed = traits::end(r);
    for (auto i = bg; i != ed;) {
        auto j = std::next(i);
        while (j != ed && *i != *j) std::advance(j);

        i = j;
    }
}
*/

}  // namespace gsh
