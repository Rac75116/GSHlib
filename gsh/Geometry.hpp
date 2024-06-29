#pragma once
#include <type_traits>
#include <gsh/Range.hpp>    // gsh::Rangeof
#include <gsh/TypeDef.hpp>  // gsh::itype

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

//template<Rangeof<Point2<itype::i32>> T> void SortArgument(T&& r) {}

}  // namespace gsh