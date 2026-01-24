#pragma once
#include <concepts>
#include <memory>
namespace gsh { template<class T, class Alloc> requires std::is_same_v<T, typename std::allocator_traits<Alloc>::value_type> && (!std::is_const_v<T>)class Vec; } // namespace gsh
