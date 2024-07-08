#pragma once
#include <type_traits>                 // std::is_arithmetic_v
#include <bit>                         // std::bit_cast
#include <gsh/Range.hpp>               // gsh::Rangeof
#include <gsh/Arr.hpp>                 // gsh::Arr
#include <gsh/TypeDef.hpp>             // gsh::itype
#include <gsh/Algorithm.hpp>           // gsh::internal::SortUnsigned64
#include <gsh/internal/Operation.hpp>  // gsh::internal::ArithmeticInterface

namespace gsh {

template<class T>
    requires std::is_arithmetic_v<T>
class Point2 : public internal::ArithmeticInterface<Point2<T>> {
public:
    T x{}, y{};
    using value_type = T;
    constexpr Point2& operator+=(const Point2& p) {
        x += p.x, y += p.y;
        return *this;
    }
    constexpr Point2& operator-=(const Point2& p) {
        x -= p.x, y -= p.y;
        return *this;
    }
    friend constexpr bool operator==(const Point2& a, const Point2& b) { return a.x == b.x && a.y == b.y; }
};
template<class T> constexpr T Dot(const Point2<T>& a, const Point2<T>& b) {
    return a.x * b.x + a.y * b.y;
}
template<class T> constexpr T Cross(const Point2<T>& a, const Point2<T>& b) {
    return a.x * b.y - a.y * b.x;
}

template<class T>
    requires std::is_arithmetic_v<T>
class Point3 : public internal::ArithmeticInterface<Point3<T>> {
public:
    T x{}, y{}, z{};
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
template<class T> constexpr T Dot(const Point3<T>& a, const Point3<T>& b) {
    return a.x * b.x + a.y * b.y + a.z * b.z;
}
template<class T> constexpr Point3<T> Cross(const Point3<T>& a, const Point3<T>& b) {
    return { a.y * b.z - a.z * b.y, a.z * b.x - a.x * b.z, a.x * b.y - a.y * b.x };
}

template<Rangeof<Point2<itype::i32>> T> Arr<Point2<itype::i32>> ArgumentSort(T&& r) {
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
            const itype::u64 tmp = (static_cast<itype::u128>(mn) * li) / mx;
            ord = id << 61 | (rev ? li - tmp : tmp);
        }
        v[i++] = static_cast<itype::u128>(std::bit_cast<itype::u64>(p)) << 64 | ord;
    }
    internal::SortUnsigned64(v.data(), v.size());
    Arr<Point2<itype::i32>> res(v.size());
    for (itype::u32 i = 0, j = v.size(); i != j; ++i) res[i] = std::bit_cast<Point2<itype::i32>>(static_cast<itype::u64>(v[i] >> 64));
    return res;
}

template<Rangeof<Point2<itype::i32>> T> Arr<Point2<itype::i32>> ConvexHull(T&& r) {
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
        while (k >= 2 && Cross(ch[k - 1] - ch[k - 2], p[i] - ch[k - 2]) <= 0) --k;
    }
    for (itype::u32 i = m - 1, t = k + 1; i > 0; ch[k++] = p[--i]) {
        while (k >= t && Cross(ch[k - 1] - ch[k - 2], p[i - 1] - ch[k - 2]) <= 0) --k;
    }
    ch.resize(k - 1);
    return ch;
}

}  // namespace gsh