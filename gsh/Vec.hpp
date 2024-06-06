#pragma once
#include <memory>             // std::allocator, std::allocator_traits
#include <iterator>           // std::reverse_iterator, std::iterator_traits, std::input_iterator, std::distance
#include <algorithm>          // std::lexicographical_compare_three_way
#include <initializer_list>   // std::initializer_list
#include <type_traits>        // std::is_same_v, std::is_const_v, std::is_trivially_(***), std::is_constant_evaluated
#include <cstring>            // std::memcpy, std::memset
#include <utility>            // std::move, std::forward, std::swap
#include <gsh/TypeDef.hpp>    // gsh::itype
#include <gsh/Exception.hpp>  // gsh::Exception
#include <gsh/Range.hpp>      // gsh::ViewInterface

namespace gsh {

template<class T, class Allocator = std::allocator<T>>
    requires std::is_same_v<T, typename std::allocator_traits<Allocator>::value_type> && (!std::is_const_v<T>)
class Vec : public ViewInterface<Vec<T, Allocator>, T> {
    using traits = std::allocator_traits<Allocator>;
public:
    using reference = T&;
    using const_reference = const T&;
    using iterator = T*;
    using const_iterator = const T*;
    using size_type = itype::u32;
    using difference_type = itype::i32;
    using value_type = T;
    using allocator_type = Allocator;
    using pointer = typename traits::pointer;
    using const_pointer = typename traits::const_pointer;
    using reverse_iterator = std::reverse_iterator<iterator>;
    using const_reverse_iterator = std::reverse_iterator<const_iterator>;
private:
    [[no_unique_address]] allocator_type alloc;
    pointer ptr = nullptr;
    size_type len = 0, cap = 0;
public:
    constexpr Vec() noexcept(noexcept(Allocator())) : Vec(Allocator()) {}
    constexpr explicit Vec(const allocator_type& a) noexcept : alloc(a) {}
    constexpr explicit Vec(size_type n, const Allocator& a = Allocator()) : alloc(a) {
        if (n == 0) [[unlikely]]
            return;
        ptr = traits::allocate(alloc, n);
        len = n, cap = n;
        if constexpr (!std::is_trivially_default_constructible_v<value_type>)
            for (size_type i = 0; i != n; ++i) traits::construct(alloc, ptr + i);
        else std::memset(ptr, 0, sizeof(value_type) * n);
    }
    constexpr explicit Vec(const size_type n, const value_type& value, const allocator_type& a = Allocator()) : alloc(a) {
        if (n == 0) [[unlikely]]
            return;
        ptr = traits::allocate(alloc, n);
        len = n, cap = n;
        for (size_type i = 0; i != n; ++i) traits::construct(alloc, ptr + i, value);
    }
    template<std::input_iterator InputIter> constexpr Vec(const InputIter first, const InputIter last, const allocator_type& a = Allocator()) : alloc(a) {
        const size_type n = std::distance(first, last);
        if (n == 0) [[unlikely]]
            return;
        ptr = traits::allocate(alloc, n);
        len = n, cap = n;
        size_type i = 0;
        for (InputIter itr = first; i != n; ++itr, ++i) traits::construct(alloc, ptr + i, *itr);
    }
    constexpr Vec(const Vec& x) : Vec(x, traits::select_on_container_copy_construction(x.alloc)) {}
    constexpr Vec(Vec&& x) noexcept : alloc(std::move(x.alloc)), ptr(x.ptr), len(x.len), cap(x.cap) { x.ptr = nullptr, x.len = 0, x.cap = 0; }
    constexpr Vec(const Vec& x, const allocator_type& a) : alloc(a), ptr(x.len == 0 ? nullptr : traits::allocate(a, x.len)), len(x.len), cap(x.len) {
        if (len == 0) [[unlikely]]
            return;
        if constexpr (std::is_trivially_copy_constructible_v<value_type> && !std::is_constant_evaluated()) {
            std::memcpy(ptr, x.ptr, sizeof(value_type) * len);
        } else {
            for (size_type i = 0; i != len; ++i) traits::construct(alloc, ptr + i, *(x.ptr + i));
        }
    }
    constexpr Vec(Vec&& x, const allocator_type& a) : alloc(a) {
        if (traits::is_always_equal || x.get_allocator() == a) {
            ptr = x.ptr, len = x.len, cap = x.cap;
            x.ptr = nullptr, x.len = 0, x.cap = 0;
        } else {
            if (x.len == 0) [[unlikely]]
                return;
            len = x.len, cap = x.cap;
            ptr = traits::allocate(alloc, len);
            if constexpr (std::is_trivially_move_constructible_v<value_type> && !std::is_constant_evaluated()) {
                std::memcpy(ptr, x.ptr, sizeof(value_type) * len);
            } else {
                for (size_type i = 0; i != len; ++i) traits::construct(alloc, ptr + i, std::move(*(x.ptr + i)));
            }
            traits::deallocate(x.alloc, x.ptr, x.cap);
            x.ptr = nullptr, x.len = 0, x.cap = 0;
        }
    }
    constexpr Vec(std::initializer_list<value_type> il, const allocator_type& a = Allocator()) : Vec(il.begin(), il.end(), a) {}
    constexpr ~Vec() {
        if (cap != 0) {
            if constexpr (!std::is_trivially_destructible_v<value_type>)
                for (size_type i = 0; i != len; ++i) traits::destroy(alloc, ptr + i);
            traits::deallocate(alloc, ptr, cap);
        }
    }
    constexpr Vec& operator=(const Vec& x) {
        if constexpr (!std::is_trivially_destructible_v<value_type>)
            for (size_type i = 0; i != len; ++i) traits::destroy(alloc, ptr + i);
        if (traits::propagate_on_container_copy_assignment || cap < x.len) {
            if (cap != 0) traits::deallocate(alloc, ptr, cap);
            if constexpr (traits::propagate_on_container_copy_assignment) alloc = x.alloc;
            cap = x.len;
            ptr = traits::allocate(alloc, cap);
        }
        len = x.len;
        if (std::is_trivially_copy_assignable_v<value_type> && !std::is_constant_evaluated()) {
            std::memcpy(ptr, x.ptr, sizeof(value_type) * len);
        } else {
            for (size_type i = 0; i != len; ++i) *(ptr + i) = *(x.ptr + i);
        }
        return *this;
    }
    constexpr Vec& operator=(Vec&& x) noexcept(traits::propagate_on_container_move_assignment::value || traits::is_always_equal::value) {
        if (cap != 0) {
            if constexpr (!std::is_trivially_destructible_v<value_type>)
                for (size_type i = 0; i != len; ++i) traits::destroy(alloc, ptr + i);
            traits::deallocate(alloc, ptr, cap);
        }
        if constexpr (traits::propagate_on_container_move_assignment) alloc = std::move(x.alloc);
        ptr = x.ptr, len = x.len, cap = x.cap;
        x.ptr = nullptr, x.len = 0, x.cap = 0;
        return *this;
    }
    constexpr Vec& operator=(std::initializer_list<value_type> init) {
        assign(init.begin(), init.end());
        return *this;
    }
    constexpr iterator begin() noexcept { return ptr; }
    constexpr const_iterator begin() const noexcept { return ptr; }
    constexpr iterator end() noexcept { return ptr + len; }
    constexpr const_iterator end() const noexcept { return ptr + len; }
    constexpr const_iterator cbegin() const noexcept { return ptr; }
    constexpr const_iterator cend() const noexcept { return ptr + len; }
    constexpr reverse_iterator rbegin() noexcept { return reverse_iterator(ptr + len); }
    constexpr const_reverse_iterator rbegin() const noexcept { return const_reverse_iterator(ptr + len); }
    constexpr reverse_iterator rend() noexcept { return reverse_iterator(ptr); }
    constexpr const_reverse_iterator rend() const noexcept { return const_reverse_iterator(ptr); }
    constexpr const_reverse_iterator crbegin() const noexcept { return const_reverse_iterator(ptr + len); }
    constexpr const_reverse_iterator crend() const noexcept { return const_reverse_iterator(ptr); }
    constexpr size_type size() const noexcept { return len; }
    constexpr size_type max_size() const noexcept {
        const auto tmp = traits::max_size(alloc);
        return tmp < 2147483647 ? tmp : 2147483647;
    }
    constexpr void resize(const size_type sz) {
        if (cap < sz) {
            const pointer new_ptr = traits::allocate(alloc, sz);
            if (cap != 0) {
                if (std::is_trivially_move_constructible_v<value_type> && !std::is_constant_evaluated()) {
                    std::memcpy(new_ptr, ptr, sizeof(value_type) * len);
                } else {
                    for (size_type i = 0; i != len; ++i) traits::construct(alloc, new_ptr + i, std::move(*(ptr + i)));
                }
                if constexpr (!std::is_trivially_destructible_v<value_type>)
                    for (size_type i = 0; i != len; ++i) traits::destroy(alloc, ptr + i);
                traits::deallocate(alloc, ptr, cap);
            }
            ptr = new_ptr;
            if constexpr (!std::is_trivially_default_constructible_v<value_type>)
                for (size_type i = len; i != sz; ++i) traits::construct(alloc, *(ptr + i));
            else std::memset(ptr + len, 0, sizeof(value_type) * (sz - len));
            len = sz, cap = sz;
        } else if (len < sz) {
            if constexpr (!std::is_trivially_default_constructible_v<value_type>)
                for (size_type i = len; i != sz; ++i) traits::construct(alloc, *(ptr + i));
            else std::memset(ptr + len, 0, sizeof(value_type) * (sz - len));
            len = sz;
        } else {
            if constexpr (!std::is_trivially_destructible_v<value_type>)
                for (size_type i = sz; i != len; ++i) traits::destroy(alloc, ptr + i);
            len = sz;
        }
    }
    constexpr void resize(const size_type sz, const value_type& c) {
        if (cap < sz) {
            const pointer new_ptr = traits::allocate(sz);
            if (cap != 0) {
                if (std::is_trivially_move_constructible_v<value_type> && !std::is_constant_evaluated()) {
                    std::memcpy(new_ptr, ptr, sizeof(value_type) * len);
                } else {
                    for (size_type i = 0; i != len; ++i) traits::construct(alloc, new_ptr + i, std::move(*(ptr + i)));
                }
                if constexpr (!std::is_trivially_destructible_v<value_type>)
                    for (size_type i = 0; i != len; ++i) traits::destroy(alloc, ptr + i);
                traits::deallocate(alloc, ptr, cap);
            }
            ptr = new_ptr;
            for (size_type i = len; i != sz; ++i) traits::construct(alloc, *(ptr + i), c);
            len = sz, cap = sz;
        } else if (len < sz) {
            for (size_type i = len; i != sz; ++i) traits::construct(alloc, *(ptr + i), c);
            len = sz;
        } else {
            if constexpr (!std::is_trivially_destructible_v<value_type>)
                for (size_type i = sz; i != len; ++i) traits::destroy(alloc, ptr + i);
            len = sz;
        }
    }
    constexpr size_type capacity() const noexcept { return cap; }
    [[nodiscard]] constexpr bool empty() const noexcept { return len == 0; }
    constexpr void reserve(const size_type n) {
        if (n > cap) {
            const pointer new_ptr = traits::allocate(alloc, n);
            if (cap != 0) {
                if (std::is_trivially_move_constructible_v<value_type> && !std::is_constant_evaluated()) {
                    std::memcpy(new_ptr, ptr, sizeof(value_type) * len);
                } else {
                    for (size_type i = 0; i != len; ++i) traits::construct(alloc, new_ptr + i, std::move(*(ptr + i)));
                }
                if constexpr (!std::is_trivially_destructible_v<value_type>)
                    for (size_type i = 0; i != len; ++i) traits::destroy(alloc, ptr + i);
                traits::deallocate(alloc, ptr, cap);
            }
            ptr = new_ptr, cap = n;
        }
    }
    constexpr void shrink_to_fit() {
        if (len == 0) {
            if (cap != 0) traits::deallocate(alloc, ptr, cap);
            ptr = nullptr, cap = 0;
            return;
        }
        if (len != cap) {
            const pointer new_ptr = traits::allocate(alloc, len);
            if (std::is_trivially_move_constructible_v<value_type> && !std::is_constant_evaluated()) {
                std::memcpy(new_ptr, ptr, sizeof(value_type) * len);
            } else {
                for (size_type i = 0; i != len; ++i) traits::construct(alloc, new_ptr + i, std::move(*(ptr + i)));
            }
            if constexpr (!std::is_trivially_destructible_v<value_type>)
                for (size_type i = 0; i != len; ++i) traits::destroy(alloc, ptr + i);
            traits::deallocate(alloc, ptr, cap);
            ptr = new_ptr, cap = len;
        }
    }
    constexpr reference operator[](const size_type n) {
#ifndef NDEBUG
        if (n >= len) [[unlikely]]
            throw gsh::Exception("gsh::Vec::operator[] / The index is out of range. ( n=", n, ", size=", len, " )");
#endif
        return *(ptr + n);
    }
    constexpr const_reference operator[](const size_type n) const {
#ifndef NDEBUG
        if (n >= len) [[unlikely]]
            throw gsh::Exception("gsh::Vec::operator[] / The index is out of range. ( n=", n, ", size=", len, " )");
#endif
        return *(ptr + n);
    }
    constexpr reference at(const size_type n) {
        if (n >= len) [[unlikely]]
            throw gsh::Exception("gsh::Vec::at / The index is out of range. ( n=", n, ", size=", len, " )");
        return *(ptr + n);
    }
    constexpr const_reference at(const size_type n) const {
        if (n >= len) [[unlikely]]
            throw gsh::Exception("gsh::Vec::at / The index is out of range. ( n=", n, ", size=", len, " )");
        return *(ptr + n);
    }
    constexpr pointer data() noexcept { return ptr; }
    constexpr const_pointer data() const noexcept { return ptr; }
    constexpr reference front() { return *ptr; }
    constexpr const_reference front() const { return *ptr; }
    constexpr reference back() { return *(ptr + len - 1); }
    constexpr const_reference back() const { return *(ptr + len - 1); }
    template<std::input_iterator InputIter> constexpr void assign(const InputIter first, const InputIter last) {
        const size_type n = std::distance(first, last);
        if (n > cap) {
            if constexpr (!std::is_trivially_destructible_v<value_type>)
                for (size_type i = 0; i != len; ++i) traits::destroy(alloc, ptr + i);
            traits::deallocate(alloc, ptr, cap);
            ptr = traits::allocate(alloc, n);
            cap = n;
            InputIter itr = first;
            for (size_type i = 0; i != n; ++itr, ++i) traits::construct(alloc, ptr + i, *itr);
        } else if (n > len) {
            size_type i = 0;
            InputIter itr = first;
            for (; i != len; ++itr, ++i) *(ptr + i) = *itr;
            for (; i != n; ++itr, ++i) traits::construct(alloc, ptr + i, *itr);
        } else {
            for (size_type i = n; i != len; ++i) traits::destroy(alloc, ptr + i);
            InputIter itr = first;
            for (size_type i = 0; i != n; ++itr, ++i) *(ptr + i) = *itr;
        }
        len = n;
    }
    constexpr void assign(const size_type n, const value_type& t) {
        const size_type n = std::distance(first, last);
        if (n > cap) {
            if constexpr (!std::is_trivially_destructible_v<value_type>)
                for (size_type i = 0; i != len; ++i) traits::destroy(alloc, ptr + i);
            traits::deallocate(alloc, ptr, cap);
            ptr = traits::allocate(alloc, n);
            cap = n;
            for (size_type i = 0; i != n; ++i) traits::construct(alloc, ptr + i, t);
        } else if (n > len) {
            size_type i = 0;
            for (; i != len; ++i) *(ptr + i) = t;
            for (; i != n; ++i) traits::construct(alloc, ptr + i, t);
        } else {
            if constexpr (!std::is_trivially_destructible_v<value_type>)
                for (size_type i = n; i != len; ++i) traits::destroy(alloc, ptr + i);
            for (size_type i = 0; i != n; ++i) *(ptr + i) = t;
        }
        len = n;
    }
    constexpr void assign(std::initializer_list<value_type> il) { assign(il.begin(), il.end()); }
private:
    constexpr void extend_one() {
        if (len == cap) {
            const pointer new_ptr = traits::allocate(alloc, cap * 2 + 8);
            if (cap != 0) {
                if (std::is_trivially_move_constructible_v<value_type> && !std::is_constant_evaluated()) {
#pragma GCC diagnostic ignored "-Wclass-memaccess"
                    std::memcpy(new_ptr, ptr, sizeof(value_type) * len);
#pragma GCC diagnostic warning "-Wclass-memaccess"
                } else {
                    for (size_type i = 0; i != len; ++i) traits::construct(alloc, new_ptr + i, std::move(*(ptr + i)));
                }
                if constexpr (!std::is_trivially_destructible_v<value_type>)
                    for (size_type i = 0; i != len; ++i) traits::destroy(alloc, ptr + i);
                traits::deallocate(alloc, ptr, cap);
            }
            ptr = new_ptr, cap = cap * 2 + 8;
        }
    }
public:
    constexpr void push_back(const T& x) {
        extend_one();
        traits::construct(alloc, ptr + (len++), x);
    }
    constexpr void push_back(T&& x) {
        extend_one();
        traits::construct(alloc, ptr + (len++), std::move(x));
    }
    template<class... Args> constexpr reference emplace_back(Args&&... args) {
        extend_one();
        traits::construct(alloc, ptr + len, std::forward<Args>(args)...);
        return *(ptr + (len++));
    }
    constexpr void pop_back() {
#ifndef NDEBUG
        if (len == 0) [[unlikely]]
            throw gsh::Exception("gsh::Vec::pop_back / The container is empty.");
#endif
        if constexpr (std::is_trivially_destructible_v<value_type>) --len;
        else traits::destroy(alloc, ptr + (--len));
    }
    constexpr iterator insert(const const_iterator position, const value_type& x) {
        //TODO
    }
    constexpr iterator insert(const const_iterator position, value_type&& x) {
        //TODO
    }
    constexpr iterator insert(const const_iterator position, const size_type n, const value_type& x) {
        //TODO
    }
    template<class InputIter> constexpr iterator insert(const const_iterator position, const InputIter first, const InputIter last) {
        //TODO
    }
    constexpr iterator insert(const const_iterator position, const std::initializer_list<value_type> il) {
        //TODO
    }
    template<class... Args> constexpr iterator emplace(const_iterator position, Args&&... args) {
        //TODO
    }
    constexpr iterator erase(const_iterator position) {
        //TODO
    }
    constexpr iterator erase(const_iterator first, const_iterator last) {
        //TODO
    }
    constexpr void swap(Vec& x) noexcept(traits::propagate_on_container_swap::value || traits::is_always_equal::value) {
        using std::swap;
        swap(ptr, x.ptr);
        swap(len, x.len);
        swap(cap, x.cap);
        if constexpr (traits::propagate_on_container_swap::value) swap(alloc, x.alloc);
    }
    constexpr void clear() {
        if constexpr (!std::is_trivially_destructible_v<value_type>)
            for (size_type i = 0; i != len; ++i) traits::destroy(alloc, ptr + i);
        len = 0;
    }
    constexpr allocator_type get_allocator() const noexcept { return alloc; }
    friend constexpr bool operator==(const Vec& x, const Vec& y) {
        if (x.len != y.len) return false;
        bool res = true;
        for (size_type i = 0; i != x.len;) {
            const bool f = *(x.ptr + i) == *(y.ptr + i);
            res &= f;
            i = f ? i + 1 : x.len;
        }
        return res;
    }
    friend constexpr auto operator<=>(const Vec& x, const Vec& y) { return std::lexicographical_compare_three_way(x.begin(), x.end(), y.begin(), y.end()); }
    friend constexpr void swap(Vec& x, Vec& y) noexcept(noexcept(x.swap(y))) { x.swap(y); }
};
template<class InputIter, class Allocator = std::allocator<typename std::iterator_traits<InputIter>::value_type>> Vec(InputIter, InputIter, Allocator = Allocator()) -> Vec<typename std::iterator_traits<InputIter>::value_type, Allocator>;

}  // namespace gsh