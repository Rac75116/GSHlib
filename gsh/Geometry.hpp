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
    for (itype::u32 i = 0; auto p : r) {
        auto [x, y] = p;
        itype::u64 ord = 0;
        if (y == 0) ord = (x >= 0 ? (1ull << 63) : 0xffffffffffffffffull);
        else {
            const itype::i64 tmp = ((x * (1ll << 32)) / y);
            ord = (std::bit_cast<itype::u64>(-tmp) >> 1) | (static_cast<itype::u64>(y > 0) << 63);
        }
        v[i++] = static_cast<itype::u128>(std::bit_cast<itype::u64>(p)) << 64 | ord;
    }
    internal::SortUnsigned64(v.data(), v.size());
    Arr<Point2<itype::i32>> res(v.size());
    for (itype::u32 i = 0, j = v.size(); i != j; ++i) res[i] = std::bit_cast<Point2<itype::i32>>(static_cast<itype::u64>(v[i] >> 64));
    return res;
}

}  // namespace gsh