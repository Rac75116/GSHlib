#pragma once
#include <type_traits>        // std::is_arithmetic_v
#include <bit>                // std::bit_cast
#include <gsh/Range.hpp>      // gsh::Rangeof
#include <gsh/Arr.hpp>        // gsh::Arr
#include <gsh/TypeDef.hpp>    // gsh::itype
#include <gsh/Algorithm.hpp>  // gsh::internal::SortUnsigned64

namespace gsh {

template<class T>
    requires std::is_arithmetic_v<T>
class Point2 {
public:
    T x, y;
    using value_type = T;
};
template<class T>
    requires std::is_arithmetic_v<T>
class Point3 {
public:
    T x, y, z;
    using value_type = T;
};

template<Rangeof<Point2<itype::i32>> T> Arr<Point2<itype::i32>> ArgumentSort(T&& r) {
    Arr<itype::u128> v(Size(r));
    for (itype::u32 i = 0; auto& p : r) {
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

}  // namespace gsh