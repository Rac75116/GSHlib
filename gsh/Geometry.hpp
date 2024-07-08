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
};
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
};

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

template<Rangeof<Point2<itype::i32>> T> auto ConvexHull(T&& r) {
    const itype::u32 n = RangeTraits<T>::size(r);
    Arr<itype::u64> sorted(n);
    for (itype::u32 i = 0; auto [x, y] : r) {
        const itype::u32 xu = std::bit_cast<itype::u32>(x), yu = std::bit_cast<itype::u32>(y);
        const itype::u32 tmpx = (x < 0 ? 0xffffffffu - xu : xu | (1u << 31)), tmpy = (y < 0 ? 0xffffffffu - yu : yu | (1u << 31));
        sorted[i++] = static_cast<itype::u64>(tmpx) << 32 | tmpy;
    }
    internal::SortUnsigned64(sorted.data(), sorted.size());
    class result_type {
        Arr<Point2<itype::i32>> arr;
    } res;
    res.arr.resize(n);
    for (itype::u32 i = 0; i != n; ++i) {
        const itype::u32 x = sorted[i] >> 32, y = sorted[i];
        const itype::i32 tmpx = std::bit_cast<itype::i32>(x < (1u << 31) ? 0xffffffffu - x : x ^ (1u << 31));
        const itype::i32 tmpy = std::bit_cast<itype::i32>(y < (1u << 31) ? 0xffffffffu - y : y ^ (1u << 31));
        res.arr[i] = { tmpx, tmpy };
    }
    // TODO
    return res;
}

}  // namespace gsh