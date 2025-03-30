#pragma once
#include "TypeDef.hpp"     // gsh::itype
#include "Functional.hpp"  // gsh::Less, gsh::Invoke
#include "Vec.hpp"         // gsh::Vec
#include "Util.hpp"        // gsh::Assume

namespace gsh {

template<class T, class Comp = Less, class Alloc = Allocator<T>> class Heap {
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
    constexpr Heap() noexcept {}
    constexpr explicit Heap(const Comp& comp, const Alloc& alloc = Alloc()) : data(alloc), comp_func(comp) {}
    constexpr explicit Heap(const Alloc& alloc) : data(alloc) {}
    template<class InputIterator> constexpr Heap(InputIterator first, InputIterator last, const Comp& comp = Comp(), const Alloc& alloc = Alloc()) : data(first, last, alloc), comp_func(comp) { make_heap(); }
    template<class InputIterator> Heap(InputIterator first, InputIterator last, const Alloc& alloc) : data(first, last, alloc) { make_heap(); }
    constexpr Heap(const Heap& x) = default;
    constexpr Heap(Heap&& y) noexcept = default;
    constexpr Heap(const Heap& x, const Alloc& alloc) : data(x.data, alloc), comp_func(x.comp_func), mx(x.mx) {}
    constexpr Heap(Heap&& y, const Alloc& alloc) : data(std::move(y.data), alloc), comp_func(y.comp_func), mx(y.mx) {}
    constexpr Heap(std::initializer_list<value_type> init, const Comp& comp = Comp(), const Alloc& alloc = Alloc()) : data(init, alloc), comp_func(comp) { make_heap(); }
    constexpr Heap(std::initializer_list<value_type> init, const Alloc& alloc) : data(init, alloc) { make_heap(); }
    constexpr Heap& operator=(const Heap&) = default;
    constexpr Heap& operator=(Heap&&) noexcept(std::is_nothrow_move_assignable_v<Comp>) = default;
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
    template<bool Min, bool SetMax> GSH_INTERNAL_INLINE constexpr void push_down(itype::u32 idx) noexcept(nothrow_op) {
        itype::u32 lim = (data.size() + 1) / 4 - 1;
        auto comp = [&](auto&& a, auto&& b) GSH_INTERNAL_INLINE {
            if constexpr (Min) return static_cast<bool>(Invoke(comp_func, a, b));
            else return static_cast<bool>(Invoke(comp_func, b, a));
        };
        itype::u32 cur = idx;
        T tmp = std::move(data[idx]);
        while (true) {
            itype::u32 grdch = (cur + 1) * 4 - 1;
            if (cur >= lim) [[unlikely]] {
                itype::u32 ch = (cur + 1) * 2 - 1;
                if (grdch < data.size()) [[unlikely]] {
                    itype::u32 m = ch + comp(data[ch + 1], data[ch]);
                    switch (data.size() - grdch) {
                    case 3 :
                        {
                            itype::u32 n = grdch + 1 + comp(data[grdch + 2], data[grdch + 1]);
                            m = comp(data[m], data[grdch]) ? m : grdch;
                            m = comp(data[m], data[n]) ? m : n;
                            break;
                        }
                    case 2 :
                        {
                            itype::u32 n = grdch + comp(data[grdch + 1], data[grdch]);
                            m = comp(data[m], data[n]) ? m : n;
                            break;
                        }
                    case 1 :
                        {
                            m = comp(data[m], data[grdch]) ? m : grdch;
                            break;
                        }
                    default : Unreachable();
                    };
                    if (m < grdch) {
                        if (comp(data[m], tmp)) {
                            data[cur] = std::move(data[m]);
                            data[m] = std::move(tmp);
                        } else {
                            data[cur] = std::move(tmp);
                        }
                    } else {
                        itype::u32 p = (m + 1) / 2 - 1;
                        if (comp(data[m], tmp)) {
                            data[cur] = std::move(data[m]);
                            if (comp(data[p], tmp)) {
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
                    bool f = comp(data[ch + 1], data[ch]);
                    T m = std::move(f ? data[ch + 1] : data[ch]);
                    bool g = comp(m, tmp);
                    data[cur] = std::move(g ? m : tmp);
                    data[ch + f] = std::move(g ? tmp : m);
                } else if (comp(data[ch], tmp)) {
                    data[cur] = std::move(data[ch]);
                    data[ch] = std::move(tmp);
                } else {
                    data[cur] = std::move(tmp);
                }
                if constexpr (SetMax) {
                    Assume(data.size() >= 3);
                    set_mx();
                }
                return;
            }
            itype::u32 a = grdch + comp(data[grdch + 1], data[grdch]);
            itype::u32 b = grdch + 2 + comp(data[grdch + 3], data[grdch + 2]);
            itype::u32 c = a + comp(data[b], data[a]) * (b - a);
            itype::u32 p = (c + 1) / 2 - 1;
            if (!comp(data[c], tmp)) {
                data[cur] = std::move(tmp);
                if constexpr (SetMax) {
                    Assume(data.size() >= 3);
                    set_mx();
                }
                return;
            }
            data[cur] = std::move(data[c]);
            cur = c;
            bool f = comp(data[p], tmp);
            T tmp2 = data[p];
            data[p] = std::move(f ? tmp : tmp2);
            tmp = std::move(f ? tmp2 : tmp);
        }
    }
    GSH_INTERNAL_INLINE constexpr void pop_min_impl() noexcept(nothrow_op) {
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
        push_down<true, true>(0);
    }
    GSH_INTERNAL_INLINE constexpr void pop_max_impl() noexcept(nothrow_op) {
        if (data.size() <= 3) [[unlikely]] {
            set_mx();
            return;
        }
        push_down<false, true>(mx);
    }
    constexpr void make_heap() noexcept(nothrow_op) {
        if (data.size() <= 1) [[unlikely]]
            return;
        itype::u32 lim1 = data.size() / 2;
        if (data.size() % 2 == 0) {
            --lim1;
            itype::u32 ch = (lim1 + 1) * 2 - 1;
            if (Invoke(comp_func, data[lim1], data[ch]) ^ is_min_level(lim1)) {
                auto tmp = std::move(data[lim1]);
                data[lim1] = std::move(data[ch]);
                data[ch] = std::move(tmp);
            }
        }
        itype::u32 lim2 = data.size() / 4;
        Assume(lim2 + 1 != 0);
        itype::u32 lr = std::bit_floor(lim2 + 1) * 2 - 1;
        lr = lim1 < lr ? lim1 : lr;
        bool lim2_min = is_min_level(lim2);
        for (itype::u32 i = lim2_min ? lim2 : lr, j = lim2_min ? lr : lim1; i < j; ++i) {
            itype::u32 ch = (i + 1) * 2 - 1;
            itype::u32 m = ch + static_cast<bool>(Invoke(comp_func, data[ch + 1], data[ch]));
            bool f = Invoke(comp_func, data[i], data[m]);
            T tmp1 = std::move(data[i]);
            T tmp2 = std::move(data[m]);
            data[i] = std::move(f ? tmp1 : tmp2);
            data[m] = std::move(f ? tmp2 : tmp1);
        }
        for (itype::u32 i = lim2_min ? lr : lim2, j = lim2_min ? lim1 : lr; i < j; ++i) {
            itype::u32 ch = (i + 1) * 2 - 1;
            itype::u32 m = ch + static_cast<bool>(Invoke(comp_func, data[ch], data[ch + 1]));
            bool f = Invoke(comp_func, data[m], data[i]);
            T tmp1 = std::move(data[i]);
            T tmp2 = std::move(data[m]);
            data[i] = std::move(f ? tmp1 : tmp2);
            data[m] = std::move(f ? tmp2 : tmp1);
        }
        for (itype::u32 i = lim2; i--;) {
            if (is_min_level(i)) {
                push_down<true, false>(i);
            } else {
                push_down<false, false>(i);
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
    constexpr void clear() noexcept {
        data.clear();
        mx = 0;
    }
    template<std::ranges::range R> constexpr void assign(R&& r) {
        data.assign(std::forward<R>(r));
        make_heap();
    }
    constexpr const_reference top() const noexcept { return data[0]; }
    constexpr const_reference min() const noexcept { return data[0]; }
    constexpr const_reference max() const noexcept { return data[mx]; }
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
    constexpr void pop() noexcept(nothrow_op) { pop_min(); }
    constexpr void pop_min() noexcept(nothrow_op) {
        data[0] = std::move(data.back());
        data.pop_back();
        pop_min_impl();
    }
    constexpr void pop_max() noexcept(nothrow_op) {
        data[mx] = std::move(data.back());
        data.pop_back();
        pop_max_impl();
    }
    constexpr void replace(const T& x) noexcept(nothrow_op && std::is_nothrow_copy_assignable_v<T>) { replace_min(x); }
    constexpr void replace(T&& x) noexcept(nothrow_op) { replace_min(std::move(x)); }
    constexpr void replace_min(const T& x) noexcept(nothrow_op && std::is_nothrow_copy_assignable_v<T>) {
        data[0] = x;
        pop_min_impl();
    }
    constexpr void replace_min(T&& x) noexcept(nothrow_op) {
        data[0] = std::move(x);
        pop_min_impl();
    }
    constexpr void replace_max(const T& x) noexcept(nothrow_op && std::is_nothrow_copy_assignable_v<T>) {
        data[mx] = x;
        pop_max_impl();
    }
    constexpr void replace_max(T&& x) noexcept(nothrow_op) {
        data[mx] = std::move(x);
        pop_max_impl();
    }
    constexpr void pushpop(const T& x) noexcept(nothrow_op && std::is_nothrow_copy_assignable_v<T>) { pushpop_min(x); }
    constexpr void pushpop(T&& x) noexcept(nothrow_op) { pushpop_min(std::move(x)); }
    constexpr void pushpop_min(const T& x) noexcept(nothrow_op && std::is_nothrow_copy_assignable_v<T>) {
        if (Invoke(comp_func, data[0], x)) {
            data[0] = x;
            pop_min_impl();
        }
    }
    constexpr void pushpop_min(T&& x) noexcept(nothrow_op) {
        if (Invoke(comp_func, data[0], x)) {
            data[0] = std::move(x);
            pop_min_impl();
        }
    }
    constexpr void pushpop_max(const T& x) noexcept(nothrow_op && std::is_nothrow_copy_assignable_v<T>) {
        if (Invoke(comp_func, x, data[mx])) {
            data[mx] = x;
            pop_max_impl();
        }
    }
    constexpr void pushpop_max(T&& x) noexcept(nothrow_op) {
        if (Invoke(comp_func, x, data[mx])) {
            data[mx] = std::move(x);
            pop_max_impl();
        }
    }
};

}  // namespace gsh
