#pragma once
#include <gsh/Functional.hpp>
#include <gsh/Memory.hpp>

namespace gsh {

template<class T, class Hasher = Hash, class Pred = EqualTo, class Alloc = Allocator<T>> class HashSet {
    using traits = AllocatorTraits<Alloc>;
};

}  // namespace gsh