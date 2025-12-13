#pragma once
#include "Memory.hpp"
#include "Pair.hpp"

namespace gsh {

namespace internal {

    template<class Key, class Value, class Alloc> class AVLTree {
        struct node : KeyValuePair<Key, Value> {
            u32 sz, rev;
            node *left, *right;
        };
    public:
        using allocator_type = std::allocator_traits<Alloc>::template rebind_alloc<node>;
    private:
        constexpr AVLTree() {}
    };

}  // namespace internal

template<class T, class Alloc = ConstexprAllocator<SharedAllocator<SingleAllocator<T>, true>>> using Set = internal::AVLTree<T, void, typename std::allocator_traits<Alloc>::template rebind_alloc<KeyValuePair<T, void>>>;

}  // namespace gsh
