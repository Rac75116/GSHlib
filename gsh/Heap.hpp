#pragma once
#include "TypeDef.hpp"     // gsh::itype
#include "Functional.hpp"  // gsh::Less, gsh::Invoke
#include "Vec.hpp"         // gsh::Vec
#include "Util.hpp"        // gsh::Assume

namespace gsh {

/*
template<class T, class Comp = Less, class Alloc = ConstexprAllocator<SharedAllocator<SingleAllocator<T>>>> class SkewHeap {
    struct node {
        using traits = AllocatorTraits<typename AllocatorTraits<Alloc>::template rebind_alloc<node>>;
        T x;
        node *l, *r;
        constexpr node(const T& x_, node* l_, node* r_) noexcept(std::is_nothrow_copy_constructible_v<T>) : x(x_), l(l_), r(r_) {}
        constexpr node(T&& x_, node* l_, node* r_) noexcept(std::is_nothrow_move_constructible_v<T>) : x(std::move(x_)), l(l_), r(r_) {}
        constexpr node* copy(auto& alloc) {
            node* res = traits::allocate(alloc, 1);
            traits::construct(alloc, res, x, nullptr, nullptr);
            if (l != nullptr) res->l = l->copy(alloc);
            if (r != nullptr) res->r = r->copy(alloc);
            return res;
        }
        constexpr node* move(auto& alloc) {
            node* res = traits::allocate(alloc, 1);
            traits::construct(alloc, std::move(res), x, nullptr, nullptr);
            if (l != nullptr) res->l = l->copy(alloc);
            if (r != nullptr) res->r = r->copy(alloc);
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
    GSH_INTERNAL_INLINE constexpr static node* merge_nodes(auto& comp, node* p, node* q) noexcept(std::is_nothrow_invocable_v<decltype(comp), const T&, const T&>) {
        if (p == nullptr) return q;
        if (q == nullptr) return p;
        node* res = nullptr;
        node *curp = p, *curq = q;
        node** prev = &res;
        while (curp != nullptr) {
            if (Invoke(comp, static_cast<const T&>(curq->x), static_cast<const T&>(curp->x))) [[unlikely]] {
                auto tmp = curp;
                curp = curq, curq = tmp;
            }
            node* tmp = curp->r;
            curp->r = curp->l;
            *prev = curp;
            prev = &(curp->l);
            curp = tmp;
        }
        *prev = curq;
        return res;
    }
    [[no_unique_address]] AllocatorTraits<Alloc>::template rebind_alloc<node> node_alloc;
    using traits = AllocatorTraits<decltype(node_alloc)>;
    [[no_unique_address]] Comp comp_func;
    node* root = nullptr;
    itype::u32 sz = 0;
public:
    using value_type = T;
    using reference = T&;
    using const_reference = const T&;
    using pointer = T*;
    using const_pointer = const T*;
    using size_type = itype::u32;
    using difference_type = itype::i32;
    using compare_type = Comp;
    using allocator_type = Alloc;
    constexpr SkewHeap() noexcept {}
    constexpr explicit SkewHeap(const Comp& comp, const Alloc& alloc = Alloc()) : node_alloc(alloc), comp_func(comp) {}
    constexpr explicit SkewHeap(const Alloc& alloc) : node_alloc(alloc) {}
    template<class InputIterator> constexpr SkewHeap(InputIterator first, InputIterator last, const Comp& comp = Comp(), const Alloc& alloc = Alloc()) : node_alloc(alloc), comp_func(comp) {}
    template<class InputIterator> SkewHeap(InputIterator first, InputIterator last, const Alloc& alloc) : SkewHeap(first, last, Comp(), alloc) {}
    constexpr SkewHeap(const SkewHeap& x) : node_alloc(traits::select_on_container_copy_construction(x.node_alloc)), comp_func(x.comp_func), sz(x.sz) {
        if (x.root != nullptr) root = x.root->copy(node_alloc);
    }
    constexpr SkewHeap(SkewHeap&& y) : node_alloc(std::move(y.node_alloc)), comp_func(std::move(y.comp_func)), root(y.root), sz(y.sz) { y.root = nullptr; }
    constexpr SkewHeap(const SkewHeap& x, const Alloc& alloc) : node_alloc(alloc), comp_func(x.comp_func), sz(x.sz) {
        if (x.root != nullptr) root = x.root->copy(node_alloc);
    }
    constexpr SkewHeap(SkewHeap&& y, const Alloc& alloc) : node_alloc(alloc), comp_func(y.comp_func), sz(y.sz) {
        if constexpr (typename traits::is_always_equal()) root = y.root, y.root = nullptr;
        else if (node_alloc == y.node_alloc) root = y.root, y.root = nullptr;
        else root = y.root->move(node_alloc);
    }
    constexpr SkewHeap(std::initializer_list<value_type> init, const Comp& comp = Comp(), const Alloc& alloc = Alloc()) : SkewHeap(init.begin(), init.end(), comp, alloc) {}
    constexpr SkewHeap(std::initializer_list<value_type> init, const Alloc& alloc) : SkewHeap(init.begin(), init.end(), Comp(), alloc) {}
    constexpr ~SkewHeap() noexcept {
        if (root != nullptr) {
            root->destroy(node_alloc);
            traits::destroy(node_alloc, root);
            traits::deallocate(node_alloc, root, 1);
        }
    }
    constexpr void push(const T& x) {
        node* p = traits::allocate(node_alloc, 1);
        traits::construct(node_alloc, p, x, nullptr, nullptr);
        root = merge_nodes(comp_func, root, p);
        ++sz;
    }
    constexpr void push(T&& x) {
        node* p = traits::allocate(node_alloc, 1);
        traits::construct(node_alloc, p, std::move(x), nullptr, nullptr);
        root = merge_nodes(comp_func, root, p);
        ++sz;
    }
    constexpr void pop() {
        node *l = root->l, *r = root->r;
        traits::destroy(node_alloc, root);
        traits::deallocate(node_alloc, root, 1);
        root = merge_nodes(comp_func, l, r);
        --sz;
    }
    constexpr reference top() noexcept { return root->x; }
    constexpr const_reference top() const noexcept { return root->x; }
    constexpr itype::u32 size() const noexcept { return sz; }
    constexpr void merge(SkewHeap&& h) {
        root = merge_nodes(comp_func, root, h.root);
        h.root = nullptr;
    }
};
*/

template<class T, class Comp = Less, class Alloc = Allocator<T>> class DoubleEndedHeap {
    Vec<T, Alloc> data;
    [[no_unique_address]] Comp comp_func;
    itype::u32 mx = 0;
public:
    using value_type = T;
    using reference = T&;
    using const_reference = const T&;
    using pointer = T*;
    using const_pointer = const T*;
    using size_type = itype::u32;
    using difference_type = itype::i32;
    using compare_type = Comp;
    using allocator_type = Alloc;
    constexpr DoubleEndedHeap() noexcept {}
    constexpr explicit DoubleEndedHeap(const Comp& comp, const Alloc& alloc = Alloc()) : data(alloc), comp_func(comp) {}
    constexpr explicit DoubleEndedHeap(const Alloc& alloc) : data(alloc) {}
    template<class InputIterator> constexpr DoubleEndedHeap(InputIterator first, InputIterator last, const Comp& comp = Comp(), const Alloc& alloc = Alloc()) : data(first, last, alloc), comp_func(comp) { make_heap(); }
    template<class InputIterator> DoubleEndedHeap(InputIterator first, InputIterator last, const Alloc& alloc) : data(first, last, alloc) { make_heap(); }
    constexpr DoubleEndedHeap(const DoubleEndedHeap& x) = default;
    constexpr DoubleEndedHeap(DoubleEndedHeap&& y) noexcept = default;
    constexpr DoubleEndedHeap(const DoubleEndedHeap& x, const Alloc& alloc) : data(x.data, alloc), comp_func(x.comp_func), mx(x.mx) {}
    constexpr DoubleEndedHeap(DoubleEndedHeap&& y, const Alloc& alloc) : data(std::move(y.data), alloc), comp_func(y.comp_func), mx(y.mx) {}
    constexpr DoubleEndedHeap(std::initializer_list<value_type> init, const Comp& comp = Comp(), const Alloc& alloc = Alloc()) : data(init, alloc), comp_func(comp) { make_heap(); }
    constexpr DoubleEndedHeap(std::initializer_list<value_type> init, const Alloc& alloc) : data(init, alloc) { make_heap(); }
    constexpr DoubleEndedHeap& operator=(const DoubleEndedHeap&) = default;
    constexpr DoubleEndedHeap& operator=(DoubleEndedHeap&&) noexcept(std::is_nothrow_move_assignable_v<Comp>) = default;
private:
    constexpr static bool nothrow_op = std::is_nothrow_move_constructible_v<T> && std::is_nothrow_move_assignable_v<T> && std::is_nothrow_invocable_v<Comp, T&, T&>;
    GSH_INTERNAL_INLINE constexpr bool is_min_level(itype::u32 idx) const noexcept {
        Assume(idx + 1 != 0);
        return std::bit_width(idx + 1) & 1;
    }
    GSH_INTERNAL_INLINE constexpr void set_mx() noexcept(nothrow_op) {
        if (data.size() >= 3) [[likely]]
            mx = 1 + Invoke(comp_func, data[1], data[2]);
        else mx = data.size() == 2;
    }
    constexpr void make_heap() noexcept(nothrow_op) {
        if (data.size() <= 1) [[unlikely]]
            return;
        bool levels = is_min_level(data.size() - 1);
        itype::u32 lim1 = data.size() / 2;
        if (data.size() % 2 == 1) {
            --lim1;
            itype::u32 ch = (lim1 + 1) / 2 - 1;
            if (Invoke(comp_func, data[lim1], data[ch]) ^ levels) {
                auto tmp = std::move(data[lim1]);
                data[lim1] = std::move(data[ch]);
                data[ch] = std::move(tmp);
            }
        }
        set_mx();
    }
    constexpr void push_up() noexcept(nothrow_op) {
        const itype::u32 idx = data.size() - 1;
        if (idx <= 2) [[unlikely]] {
            if (Invoke(comp_func, data[idx], data[0])) {
                auto tmp = std::move(data[idx]);
                data[idx] = std::move(data[0]);
                data[0] = std::move(tmp);
            }
            set_mx();
            return;
        }
        itype::u32 p = ((idx + 1) >> 1) - 1;
        if (is_min_level(idx)) {
            if (Invoke(comp_func, data[p], data[idx])) {
                // push_up_max(p)
                T tmp = std::move(data[idx]);
                data[idx] = std::move(data[p]);
                itype::u32 cur = p;
                while (cur > 2 && Invoke(comp_func, data[p = ((cur + 1) / 4) - 1], tmp)) {
                    data[cur] = std::move(data[p]);
                    cur = p;
                }
                data[cur] = std::move(tmp);
                Assume(data.size() >= 3);
                set_mx();
            } else {
                // push_up_min(idx)
                T tmp = std::move(data[idx]);
                itype::u32 cur = idx;
                while (Invoke(comp_func, tmp, data[p = ((cur + 1) / 4) - 1])) {
                    data[cur] = std::move(data[p]);
                    cur = p;
                    if (cur == 0) [[unlikely]]
                        break;
                }
                data[cur] = std::move(tmp);
            }
        } else {
            if (Invoke(comp_func, data[idx], data[p])) {
                // push_up_min(p)
                T tmp = std::move(data[idx]);
                data[idx] = std::move(data[p]);
                itype::u32 cur = p;
                while (cur != 0 && Invoke(comp_func, tmp, data[p = ((cur + 1) / 4) - 1])) {
                    data[cur] = std::move(data[p]);
                    cur = p;
                }
                data[cur] = std::move(tmp);
            } else {
                // push_up_max(idx)
                T tmp = std::move(data[idx]);
                itype::u32 cur = idx;
                while (Invoke(comp_func, data[p = ((cur + 1) / 4) - 1], tmp)) {
                    data[cur] = std::move(data[p]);
                    cur = p;
                    if (cur <= 2) [[unlikely]] {
                        data[cur] = std::move(tmp);
                        Assume(data.size() >= 3);
                        set_mx();
                        return;
                    }
                }
                data[cur] = std::move(tmp);
            }
        }
    }
public:
    constexpr const_reference top() const noexcept { return data[0]; }
    constexpr const_reference front() const noexcept { return data[0]; }
    constexpr const_reference back() const noexcept { return data[mx]; }
    [[nodiscard]] constexpr bool empty() const noexcept { return data.empty(); }
    constexpr itype::u32 size() const noexcept { return data.size(); }
    constexpr void reserve(itype::u32 n) { data.reserve(n); }
    constexpr void push(const T& x) {
        data.push_back(x);
        push_up();
    }
    constexpr void push(T&& x) {
        data.push_back(std::move(x));
        push_up();
    }
    template<class... Args> constexpr void emplace(Args&&... args) {
        data.emplace_back(std::forward<Args>(args)...);
        push_up();
    }
    constexpr void pop() noexcept(nothrow_op) { pop_front(); }
    constexpr void pop_front() noexcept(nothrow_op) {
        data[0] = std::move(data.back());
        data.pop_back();
        if (data.size() <= 3) [[unlikely]] {
            switch (data.size()) {
            case 0 : break;
            case 1 : mx = 0; break;
            case 2 :
                {
                    if (Invoke(comp_func, data[1], data[0])) {
                        auto tmp = std::move(data[0]);
                        data[0] = std::move(data[1]);
                        data[1] = std::move(tmp);
                    }
                    mx = 1;
                    break;
                }
            case 3 :
                {
                    itype::u32 m = 1 + Invoke(comp_func, data[2], data[1]);
                    if (Invoke(comp_func, data[m], data[0])) {
                        auto tmp = std::move(data[0]);
                        data[0] = std::move(data[m]);
                        data[m] = std::move(tmp);
                    }
                    mx = 1 + Invoke(comp_func, data[1], data[2]);
                    break;
                }
            default : Unreachable();
            }
            return;
        }
        itype::u32 lim = (data.size() + 1) / 4 - 1;
        itype::u32 cur = 0;
        T tmp = std::move(data[0]);
        while (true) {
            itype::u32 grdch = (cur + 1) * 4 - 1;
            if (cur >= lim) [[unlikely]] {
                itype::u32 ch = (cur + 1) * 2 - 1;
                if (grdch < data.size()) [[unlikely]] {
                    itype::u32 m = ch + Invoke(comp_func, data[ch + 1], data[ch]);
                    switch (data.size() - grdch) {
                    case 3 :
                        {
                            itype::u32 n = grdch + 1 + Invoke(comp_func, data[grdch + 2], data[grdch + 1]);
                            m = Invoke(comp_func, data[m], data[grdch]) ? m : grdch;
                            m = Invoke(comp_func, data[m], data[n]) ? m : n;
                            break;
                        }
                    case 2 :
                        {
                            itype::u32 n = grdch + Invoke(comp_func, data[grdch + 1], data[grdch]);
                            m = Invoke(comp_func, data[m], data[n]) ? m : n;
                            break;
                        }
                    case 1 :
                        {
                            m = Invoke(comp_func, data[m], data[grdch]) ? m : grdch;
                            break;
                        }
                    default : Unreachable();
                    };
                    if (m < grdch) {
                        if (Invoke(comp_func, data[m], tmp)) {
                            data[cur] = std::move(data[m]);
                            data[m] = std::move(tmp);
                        } else {
                            data[cur] = std::move(tmp);
                        }
                    } else {
                        itype::u32 p = (m + 1) / 2 - 1;
                        if (Invoke(comp_func, data[m], tmp)) {
                            data[cur] = std::move(data[m]);
                            if (Invoke(comp_func, data[p], tmp)) {
                                data[m] = std::move(data[p]);
                                data[p] = std::move(tmp);
                            } else {
                                data[m] = std::move(tmp);
                            }
                        } else {
                            data[cur] = std::move(tmp);
                        }
                    }
                } else if (ch >= data.size()) [[likely]] {
                    data[cur] = std::move(tmp);
                } else if (ch < data.size() - 1) [[likely]] {
                    bool f = Invoke(comp_func, data[ch + 1], data[ch]);
                    T m = std::move(f ? data[ch + 1] : data[ch]);
                    bool g = Invoke(comp_func, m, tmp);
                    data[cur] = std::move(g ? m : tmp);
                    data[ch + f] = std::move(g ? tmp : m);
                } else if (Invoke(comp_func, data[ch], tmp)) {
                    data[cur] = std::move(data[ch]);
                    data[ch] = std::move(tmp);
                } else {
                    data[cur] = std::move(tmp);
                }
                Assume(data.size() >= 3);
                set_mx();
                return;
            }
            itype::u32 a = grdch + Invoke(comp_func, data[grdch + 1], data[grdch]);
            itype::u32 b = grdch + 2 + Invoke(comp_func, data[grdch + 3], data[grdch + 2]);
            itype::u32 c = a + Invoke(comp_func, data[b], data[a]) * (b - a);
            itype::u32 p = (c + 1) / 2 - 1;
            if (!Invoke(comp_func, data[c], tmp)) {
                data[cur] = std::move(tmp);
                Assume(data.size() >= 3);
                set_mx();
                return;
            }
            data[cur] = std::move(data[c]);
            cur = c;
            bool f = Invoke(comp_func, data[p], tmp);
            T tmp2 = data[p];
            data[p] = std::move(f ? tmp : tmp2);
            tmp = std::move(f ? tmp2 : tmp);
        }
    }
    constexpr void pop_back() noexcept(nothrow_op) {
        data[mx] = std::move(data.back());
        data.pop_back();
        if (data.size() <= 3) [[unlikely]] {
            set_mx();
            return;
        }
        itype::u32 lim = (data.size() + 1) / 4 - 1;
        itype::u32 cur = mx;
        T tmp = std::move(data[mx]);
        while (true) {
            itype::u32 grdch = (cur + 1) * 4 - 1;
            if (cur >= lim) [[unlikely]] {
                itype::u32 ch = (cur + 1) * 2 - 1;
                if (grdch < data.size()) [[unlikely]] {
                    itype::u32 m = ch + Invoke(comp_func, data[ch], data[ch + 1]);
                    switch (data.size() - grdch) {
                    case 3 :
                        {
                            itype::u32 n = grdch + 1 + Invoke(comp_func, data[grdch + 1], data[grdch + 2]);
                            m = Invoke(comp_func, data[grdch], data[m]) ? m : grdch;
                            m = Invoke(comp_func, data[n], data[m]) ? m : n;
                            break;
                        }
                    case 2 :
                        {
                            itype::u32 n = grdch + Invoke(comp_func, data[grdch], data[grdch + 1]);
                            m = Invoke(comp_func, data[n], data[m]) ? m : n;
                            break;
                        }
                    case 1 :
                        {
                            m = Invoke(comp_func, data[grdch], data[m]) ? m : grdch;
                            break;
                        }
                    default : Unreachable();
                    };
                    if (m < grdch) {
                        if (Invoke(comp_func, tmp, data[m])) {
                            data[cur] = std::move(data[m]);
                            data[m] = std::move(tmp);
                        } else {
                            data[cur] = std::move(tmp);
                        }
                    } else {
                        itype::u32 p = (m + 1) / 2 - 1;
                        if (Invoke(comp_func, tmp, data[m])) {
                            data[cur] = std::move(data[m]);
                            if (Invoke(comp_func, tmp, data[p])) {
                                data[m] = std::move(data[p]);
                                data[p] = std::move(tmp);
                            } else {
                                data[m] = std::move(tmp);
                            }
                        } else {
                            data[cur] = std::move(tmp);
                        }
                    }
                } else if (ch >= data.size()) [[likely]] {
                    data[cur] = std::move(tmp);
                } else if (ch < data.size() - 1) [[likely]] {
                    bool f = Invoke(comp_func, data[ch], data[ch + 1]);
                    T m = std::move(f ? data[ch + 1] : data[ch]);
                    bool g = Invoke(comp_func, tmp, m);
                    data[cur] = std::move(g ? m : tmp);
                    data[ch + f] = std::move(g ? tmp : m);
                } else if (Invoke(comp_func, tmp, data[ch])) {
                    data[cur] = std::move(data[ch]);
                    data[ch] = std::move(tmp);
                } else {
                    data[cur] = std::move(tmp);
                }
                Assume(data.size() >= 3);
                set_mx();
                return;
            }
            itype::u32 a = grdch + Invoke(comp_func, data[grdch], data[grdch + 1]);
            itype::u32 b = grdch + 2 + Invoke(comp_func, data[grdch + 2], data[grdch + 3]);
            itype::u32 c = a + Invoke(comp_func, data[a], data[b]) * (b - a);
            itype::u32 p = (c + 1) / 2 - 1;
            if (!Invoke(comp_func, tmp, data[c])) {
                data[cur] = std::move(tmp);
                Assume(data.size() >= 3);
                set_mx();
                return;
            }
            data[cur] = std::move(data[c]);
            cur = c;
            bool f = Invoke(comp_func, tmp, data[p]);
            T tmp2 = data[p];
            data[p] = std::move(f ? tmp : tmp2);
            tmp = std::move(f ? tmp2 : tmp);
        }
    }
};

}  // namespace gsh
