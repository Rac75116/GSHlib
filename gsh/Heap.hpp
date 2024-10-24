#pragma once
#include "TypeDef.hpp"
#include "Memory.hpp"
#include "Functional.hpp"

namespace gsh {

template<class T, class Comp = Less, class Alloc = ConstexprAllocator<SharedAllocator<SingleAllocator<T>>>> class SkewHeap {
    struct node {
        using traits = AllocatorTraits<AllocatorTraits<Alloc>::template rebind_alloc<node>>;
        T x;
        node *l, *r;
        constexpr node* copy(auto& alloc) {
            node* res = traits::allocate(alloc, 1);
            traits::construct(alloc, res, x, nullptr, nullptr);
            if (l != nullptr) res.l = l->copy(alloc);
            if (r != nullptr) res.r = r->copy(alloc);
            return res;
        }
        constexpr node* move(auto& alloc) {
            node* res = traits::allocate(alloc, 1);
            traits::construct(alloc, std::move(res), x, nullptr, nullptr);
            if (l != nullptr) res.l = l->copy(alloc);
            if (r != nullptr) res.r = r->copy(alloc);
            return res;
        }
        constexpr void destroy(auto& alloc) noexcept {
            if (l != nullptr) {
                l->destroy(alloc);
                traits::destroy(alloc, l);
                traits::deallocate(alloc, l, 1);
            }
            if (r != nullptr) {
                r->destroy(alloc);
                traits::destroy(alloc, r);
                traits::deallocate(alloc, r, 1);
            }
        }
    };
    [[no_unique_address]] AllocatorTraits<Alloc>::template rebind_alloc<node> node_alloc;
    using traits = AllocatorTraits<decltype(node_alloc)>;
    [[no_unique_address]] Comp comp_func;
    node* root = nullptr;
    itype::u32 sz = 0;
public:
    using value_type = T;
    using reference = T&;
    using const_reference = const T&;
    using size_type = itype::u32;
    using difference_type = itype::i32;
    constexpr SkewHeap() noexcept {}
    constexpr explicit SkewHeap(const Comp& comp, const Alloc& alloc = Alloc()) : node_alloc(alloc), comp_func(comp) {}
    constexpr explicit SkewHeap(const Alloc& alloc) : node_alloc(alloc) {}
    template<class InputIterator> constexpr SkewHeap(InputIterator first, InputIterator last, const Comp& comp = Comp(), const Alloc& alloc = Alloc()) : node_alloc(alloc), comp_func(comp);
    template<class InputIterator> SkewHeap(InputIterator first, InputIterator last, const Alloc& alloc) : SkewHeap(first, last, Comp(), alloc) {}
    constexpr SkewHeap(const SkewHeap& x) : node_alloc(traits::select_on_container_copy_construction(x.node_alloc)), comp_func(x.comp_func), sz(x.sz) {
        if (x.root != nullptr) root = x.root->copy(node_alloc);
    }
    constexpr SkewHeap(SkewHeap&& y) : node_alloc(std::move(y.node_alloc)), comp_func(std::move(y.comp_func)), root(y.root), sz(y.sz) { y.root = nullptr; }
    constexpr SkewHeap(const SkewHeap& x, const Alloc& alloc) : node_alloc(alloc), comp_func(x.comp_func), sz(x.sz) {
        if (x.root != nullptr) root = x.root->copy(node_alloc);
    }
    constexpr SkewHeap(SkewHeap&& y, const Alloc& alloc) : node_alloc(alloc), comp_func(y.comp_func), sz(y.sz) {
        if (node_alloc == y.node_alloc) {
            root = y.root;
            y.root = nullptr;
        } else {
            root = y.root->move(node_alloc);
        }
    }
    constexpr SkewHeap(std::initializer_list<value_type> init, const Comp& comp = Comp(), const Alloc& alloc = Alloc()) : SkewHeap(init.begin(), init.end(), comp, alloc) {}
    constexpr SkewHeap(std::initializer_list<value_type> init, const Alloc& alloc) : SkewHeap(init.begin(), init.end(), Comp(), alloc) {}
    constexpr ~SkewHeap() noexcept {
        if (root != 0) {
            root->destroy(node_alloc);
            traits::destroy(node_alloc, root);
            traits::deallocate(node_alloc, root, 1);
        }
    }
};

}  // namespace gsh
