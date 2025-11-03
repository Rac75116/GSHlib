#pragma once
#include "gsh/Memory.hpp"
#include "gsh/Pair.hpp"

namespace gsh {

namespace internal {

    template<class Key, class Value, class Alloc> class AVLTree {
        struct node : KeyValuePair<Key, Value> {
            u32 sz, rev;
            node *left, *right;
        };
    public:
        using allocator_type = AllocatorTraits<Alloc>::template rebind_alloc<node>;
    private:
        constexpr AVLTree() {}
    };

}  // namespace internal

template<class T, class Alloc = ConstexprAllocator<SharedAllocator<SingleAllocator<T>, true>>> using Set = internal::AVLTree<T, void, typename AllocatorTraits<Alloc>::template rebind_alloc<KeyValuePair<T, void>>>;

}  // namespace gsh
