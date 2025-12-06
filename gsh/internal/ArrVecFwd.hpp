#pragma once
#include <concepts>
#include <memory>

namespace gsh {

template<class T>
    requires std::same_as<T, std::remove_cv_t<T>>
class ArrInitTag {};
template<class T = void> constexpr ArrInitTag<T> ArrInit;
class ArrNoInitTag {};
constexpr ArrNoInitTag ArrNoInit;

template<class T, class Alloc>
    requires std::same_as<T, typename std::allocator_traits<Alloc>::value_type> && std::same_as<T, std::remove_cv_t<T>>
class Arr;

template<class T, class Alloc>
    requires std::is_same_v<T, typename std::allocator_traits<Alloc>::value_type> && (!std::is_const_v<T>)
class Vec;

}  // namespace gsh