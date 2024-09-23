#pragma once
#include <iterator>          // std::reverse_iterator, std::iterator_traits, std::input_iterator, std::distance
#include <algorithm>         // std::lexicographical_compare_three_way
#include <initializer_list>  // std::initializer_list
#include <type_traits>       // std::remove_cv_t, std::is_constant_evaluated, std::common_type_t, std::conditional_t, std::is_void_v, std::integral_constant
#include <concepts>          // std::same_as
#include <cstring>           // std::memset
#include <utility>           // std::move, std::forward, std::swap
#include <tuple>             // std::tuple_size, std::tuple_element
#include "TypeDef.hpp"       // gsh::itype
#include "Exception.hpp"     // gsh::Exception
#include "Range.hpp"         // gsh::ViewInterface
#include "Memory.hpp"        // gsh::Allocator, gsh::AllocatorTraits, gsh::DestroyAt
#include "internal/UtilMacro.hpp"

namespace gsh {

template<class T>
    requires std::same_as<T, std::remove_cv_t<T>>
class ArrInitTag {};
template<class T = void> constexpr ArrInitTag<T> ArrInit;
class ArrNoInitTag {};
constexpr ArrNoInitTag ArrNoInit;

template<class T, class Allocator = Allocator<T>>
    requires std::same_as<T, typename AllocatorTraits<Allocator>::value_type> && std::same_as<T, std::remove_cv_t<T>>
class Arr : public ViewInterface<Arr<T, Allocator>, T> {
    using traits = AllocatorTraits<Allocator>;
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
    size_type len = 0;
public:
    constexpr Arr() noexcept(noexcept(Allocator())) : Arr(Allocator()) {}
    constexpr explicit Arr(const allocator_type& a) noexcept : alloc(a) {}
    constexpr explicit Arr(size_type n, const allocator_type& a = Allocator()) : alloc(a) {
        if (n == 0) [[unlikely]]
            return;
        ptr = traits::allocate(alloc, n);
        len = n;
        for (size_type i = 0; i != n; ++i) traits::construct(alloc, ptr + i);
    }
    constexpr explicit Arr(ArrNoInitTag, size_type n, const allocator_type& a = Allocator()) : alloc(a) {
        if (n == 0) [[unlikely]]
            return;
        ptr = traits::allocate(alloc, n);
        len = n;
    }
    constexpr explicit Arr(const size_type n, const value_type& value, const allocator_type& a = Allocator()) : alloc(a) {
        if (n == 0) [[unlikely]]
            return;
        ptr = traits::allocate(alloc, n);
        len = n;
        for (size_type i = 0; i != n; ++i) traits::construct(alloc, ptr + i, value);
    }
    template<std::input_iterator InputIter> constexpr Arr(InputIter first, InputIter last, const allocator_type& a = Allocator()) : alloc(a) {
        const size_type n = std::distance(first, last);
        if (n == 0) [[unlikely]]
            return;
        ptr = traits::allocate(alloc, n);
        len = n;
        size_type i = 0;
        for (InputIter itr = first; i != n; ++itr, ++i) traits::construct(alloc, ptr + i, *itr);
    }
    constexpr Arr(const Arr& x) : Arr(x, traits::select_on_container_copy_construction(x.alloc)) {}
    constexpr Arr(Arr&& x) noexcept : alloc(std::move(x.alloc)), ptr(x.ptr), len(x.len) { x.ptr = nullptr, x.len = 0; }
    constexpr Arr(const Arr& x, const allocator_type& a) : alloc(a), len(x.len) {
        if (len == 0) [[unlikely]]
            return;
        ptr = traits::allocate(alloc, len);
        for (size_type i = 0; i != len; ++i) traits::construct(alloc, ptr + i, *(x.ptr + i));
    }
    constexpr Arr(Arr&& x, const allocator_type& a) : alloc(a) {
        if (traits::is_always_equal || x.get_allocator() == a) {
            ptr = x.ptr, len = x.len;
            x.ptr = nullptr, x.len = 0;
        } else {
            if (x.len == 0) [[unlikely]]
                return;
            len = x.len;
            ptr = traits::allocate(alloc, len);
            for (size_type i = 0; i != len; ++i) traits::construct(alloc, ptr + i, std::move(*(x.ptr + i)));
            traits::deallocate(x.alloc, x.ptr, x.len);
            x.ptr = nullptr, x.len = 0;
        }
    }
    constexpr Arr(std::initializer_list<value_type> il, const allocator_type& a = Allocator()) : Arr(il.begin(), il.end(), a) {}
    template<ForwardRange R>
        requires(!std::same_as<Arr, std::remove_cvref_t<R>>)
    constexpr Arr(R&& r, const allocator_type& a = Allocator()) : Arr(RangeTraits<R>::begin(r), RangeTraits<R>::end(r), a) {}
    constexpr ~Arr() {
        if (len != 0) {
            for (size_type i = 0; i != len; ++i) traits::destroy(alloc, ptr + i);
            traits::deallocate(alloc, ptr, len);
        }
    }
    constexpr Arr& operator=(const Arr& x) {
        if (&x == this) return *this;
        for (size_type i = 0; i != len; ++i) traits::destroy(alloc, ptr + i);
        if (traits::propagate_on_container_copy_assignment::value || len != x.len) {
            if (len != 0) traits::deallocate(alloc, ptr, len);
            if constexpr (traits::propagate_on_container_copy_assignment::value) alloc = x.alloc;
            ptr = traits::allocate(alloc, x.len);
        }
        len = x.len;
        for (size_type i = 0; i != len; ++i) *(ptr + i) = *(x.ptr + i);
        return *this;
    }
    constexpr Arr& operator=(Arr&& x) noexcept(traits::propagate_on_container_move_assignment::value || traits::is_always_equal::value) {
        if (&x == this) return *this;
        if (len != 0) {
            for (size_type i = 0; i != len; ++i) traits::destroy(alloc, ptr + i);
            traits::deallocate(alloc, ptr, len);
        }
        if constexpr (traits::propagate_on_container_move_assignment::value) alloc = std::move(x.alloc);
        ptr = x.ptr, len = x.len;
        x.ptr = nullptr, x.len = 0;
        return *this;
    }
    constexpr Arr& operator=(std::initializer_list<value_type> init) {
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
        if (len == sz) return;
        if (sz == 0) {
            clear();
            return;
        }
        const pointer new_ptr = traits::allocate(alloc, sz);
        const size_type mn = len < sz ? len : sz;
        if (len != 0) {
            for (size_type i = 0; i != mn; ++i) traits::construct(alloc, new_ptr + i, std::move(*(ptr + i)));
            for (size_type i = 0; i != len; ++i) traits::destroy(alloc, ptr + i);
            traits::deallocate(alloc, ptr, len);
        }
        ptr = new_ptr;
        for (size_type i = len; i < sz; ++i) traits::construct(alloc, ptr + i);
        len = sz;
    }
    constexpr void resize(const size_type sz, const value_type& c) {
        if (len == sz) return;
        if (sz == 0) {
            clear();
            return;
        }
        const pointer new_ptr = traits::allocate(alloc, sz);
        const size_type mn = len < sz ? len : sz;
        if (len != 0) {
            for (size_type i = 0; i != mn; ++i) traits::construct(alloc, new_ptr + i, std::move(*(ptr + i)));
            for (size_type i = 0; i != len; ++i) traits::destroy(alloc, ptr + i);
            traits::deallocate(alloc, ptr, len);
        }
        ptr = new_ptr;
        for (size_type i = len; i < sz; ++i) traits::construct(alloc, ptr + i, c);
        len = sz;
    }
    constexpr void resize(const size_type sz, ArrNoInitTag) {
        if (len == sz) return;
        if (sz == 0) {
            clear();
            return;
        }
        const pointer new_ptr = traits::allocate(alloc, sz);
        const size_type mn = len < sz ? len : sz;
        if (len != 0) {
            for (size_type i = 0; i != mn; ++i) traits::construct(alloc, new_ptr + i, std::move(*(ptr + i)));
            for (size_type i = 0; i != len; ++i) traits::destroy(alloc, ptr + i);
            traits::deallocate(alloc, ptr, len);
        }
        ptr = new_ptr;
        len = sz;
    }
    [[nodiscard]] constexpr bool empty() const noexcept { return len == 0; }
    GSH_INTERNAL_INLINE constexpr reference operator[](const size_type n) {
#ifndef NDEBUG
        if (n >= len) [[unlikely]]
            throw gsh::Exception("gsh::Arr::operator[] / The index is out of range. ( n=", n, ", size=", len, " )");
#endif
        return *(ptr + n);
    }
    GSH_INTERNAL_INLINE constexpr const_reference operator[](const size_type n) const {
#ifndef NDEBUG
        if (n >= len) [[unlikely]]
            throw gsh::Exception("gsh::Arr::operator[] / The index is out of range. ( n=", n, ", size=", len, " )");
#endif
        return *(ptr + n);
    }
    GSH_INTERNAL_INLINE constexpr reference at(const size_type n) {
        if (n >= len) [[unlikely]]
            throw gsh::Exception("gsh::Arr::at / The index is out of range. ( n=", n, ", size=", len, " )");
        return *(ptr + n);
    }
    GSH_INTERNAL_INLINE constexpr const_reference at(const size_type n) const {
        if (n >= len) [[unlikely]]
            throw gsh::Exception("gsh::Arr::at / The index is out of range. ( n=", n, ", size=", len, " )");
        return *(ptr + n);
    }
    GSH_INTERNAL_INLINE constexpr reference at_unchecked(const size_type n) { return *(ptr + n); }
    GSH_INTERNAL_INLINE constexpr const_reference at_unchecked(const size_type n) const { return *(ptr + n); }
    constexpr pointer data() noexcept { return ptr; }
    constexpr const_pointer data() const noexcept { return ptr; }
    constexpr reference front() { return *ptr; }
    constexpr const_reference front() const { return *ptr; }
    constexpr reference back() { return *(ptr + len - 1); }
    constexpr const_reference back() const { return *(ptr + len - 1); }
    template<std::input_iterator InputIter> constexpr void assign(const InputIter first, const InputIter last) {
        const size_type n = std::distance(first, last);
        if (n == 0) {
            clear();
        } else if (len == n) {
            InputIter itr = first;
            for (size_type i = 0; i != len; ++itr, ++i) *(ptr + i) = *itr;
        } else {
            for (size_type i = 0; i != len; ++i) traits::destroy(alloc, ptr + i);
            traits::deallocate(alloc, ptr, len);
            ptr = traits::allocate(alloc, n);
            len = n;
            InputIter itr = first;
            for (size_type i = 0; i != n; ++itr, ++i) traits::construct(alloc, ptr + i, *itr);
        }
    }
    constexpr void assign(const size_type n, const value_type& t) {
        if (n == 0) {
            clear();
        } else if (len == n) {
            for (size_type i = 0; i != len; ++i) *(ptr + i) = t;
        } else {
            for (size_type i = 0; i != len; ++i) traits::destroy(alloc, ptr + i);
            traits::deallocate(alloc, ptr, len);
            ptr = traits::allocate(alloc, n);
            len = n;
            for (size_type i = 0; i != n; ++i) traits::construct(alloc, ptr + i, t);
        }
    }
    constexpr void assign(const size_type n, ArrNoInitTag) {
        clear();
        if (n != 0) {
            ptr = traits::allocator(alloc, n);
            len = n;
        }
    }
    constexpr void assign(std::initializer_list<value_type> il) { assign(il.begin(), il.end()); }
    constexpr void swap(Arr& x) noexcept(traits::propagate_on_container_swap::value || traits::is_always_equal::value) {
        using std::swap;
        swap(ptr, x.ptr);
        swap(len, x.len);
        if constexpr (traits::propagate_on_container_swap::value) swap(alloc, x.alloc);
    }
    constexpr void clear() {
        if (len != 0) {
            for (size_type i = 0; i != len; ++i) traits::destroy(alloc, ptr + i);
            traits::deallocate(alloc, ptr, len);
            ptr = nullptr, len = 0;
        }
    }
    constexpr allocator_type get_allocator() const noexcept { return alloc; }
    friend constexpr bool operator==(const Arr& x, const Arr& y) {
        if (x.len != y.len) return false;
        bool res = true;
        for (size_type i = 0; i != x.len;) {
            const bool f = *(x.ptr + i) == *(y.ptr + i);
            res &= f;
            i = f ? i + 1 : x.len;
        }
        return res;
    }
    friend constexpr auto operator<=>(const Arr& x, const Arr& y) { return std::lexicographical_compare_three_way(x.begin(), x.end(), y.begin(), y.end()); }
    friend constexpr void swap(Arr& x, Arr& y) noexcept(noexcept(x.swap(y))) { x.swap(y); }
};
template<std::input_iterator InputIter, class Alloc = Allocator<typename std::iterator_traits<InputIter>::value_type>> Arr(InputIter, InputIter, Alloc = Alloc()) -> Arr<typename std::iterator_traits<InputIter>::value_type, Alloc>;
template<Range R, class Alloc = Allocator<typename RangeTraits<R>::value_type>> Arr(R, Alloc = Alloc()) -> Arr<typename RangeTraits<R>::value_type, Alloc>;

template<class T, itype::u32 N>
    requires std::same_as<T, std::remove_cv_t<T>>
class StaticArr : public ViewInterface<StaticArr<T, N>, T> {
    union {
        T elems[(N == 0 ? 1 : N)];
    };
public:
    using reference = T&;
    using const_reference = const T&;
    using iterator = T*;
    using const_iterator = const T*;
    using size_type = itype::u32;
    using difference_type = itype::i32;
    using value_type = T;
    using pointer = T*;
    using const_pointer = const T*;
    using reverse_iterator = std::reverse_iterator<iterator>;
    using const_reverse_iterator = std::reverse_iterator<const_iterator>;
    constexpr StaticArr() noexcept(noexcept(value_type{})) : elems{} {}
    constexpr StaticArr(ArrNoInitTag) noexcept {}
    template<class U, class... Args> constexpr StaticArr(ArrInitTag<U>, Args&&... args) : elems{ static_cast<U>(std::forward<Args>(args))... } {
        static_assert(std::same_as<T, U>, "gsh::StaticArr::StaticArr / The type specified in gsh::ArrInitTag is different from value_type.");
        static_assert(sizeof...(Args) == N, "gsh::StaticArr::StaticArr / The number of arguments is greater than the length of the array.");
    }
    template<class... Args> constexpr StaticArr(ArrInitTag<void>, Args&&... args) : elems{ static_cast<T>(std::forward<Args>(args))... } { static_assert(sizeof...(Args) <= N, "gsh::StaticArr::StaticArr / The number of arguments is greater than the length of the array."); }
    constexpr explicit StaticArr(const value_type& value) {
        for (itype::u32 i = 0; i != N; ++i) std::construct_at(elems + i, value);
    }
    template<std::input_iterator InputIter> constexpr explicit StaticArr(InputIter first) {
        for (itype::u32 i = 0; i != N; ++first, ++i) std::construct_at(elems + i, *first);
    }
    template<std::input_iterator InputIter> constexpr StaticArr(InputIter first, InputIter last) {
        const itype::u32 n = std::distance(first, last);
        if (n != N) throw gsh::Exception("gsh::StaticArr::StaticArr / The size of the given range differs from the size of the array.");
        for (itype::u32 i = 0; i != N; ++first, ++i) std::construct_at(elems + i, *first);
    }
    template<Rangeof<value_type> R> constexpr StaticArr(R&& r) : StaticArr(RangeTraits<R>::begin(r), RangeTraits<R>::end(r)) {}
    constexpr StaticArr(const value_type (&a)[N]) {
        for (itype::u32 i = 0; i != N; ++i) std::construct_at(elems + i, a[i]);
    }
    constexpr StaticArr(value_type (&&a)[N]) {
        for (itype::u32 i = 0; i != N; ++i) std::construct_at(elems + i, std::move(a[i]));
    }
    constexpr StaticArr(const StaticArr& x) {
        for (itype::u32 i = 0; i != N; ++i) std::construct_at(elems + i, x.elems[i]);
    }
    constexpr StaticArr(StaticArr&& y) {
        for (itype::u32 i = 0; i != N; ++i) std::construct_at(elems + i, std::move(y.elems[i]));
    }
    constexpr StaticArr(std::initializer_list<value_type> il) : StaticArr(il.begin(), il.end()) {}
    template<ForwardRange R>
        requires(!std::same_as<StaticArr, std::remove_cvref_t<R>>)
    constexpr StaticArr(R&& r) : StaticArr(RangeTraits<R>::begin(r), RangeTraits<R>::end(r)) {}
    constexpr ~StaticArr() noexcept {
        if constexpr (!std::is_trivially_destructible_v<value_type>)
            for (itype::u32 i = 0; i != N; ++i) std::destroy_at(elems + i);
    }
    constexpr StaticArr& operator=(const StaticArr& x) {
        for (itype::u32 i = 0; i != N; ++i) elems[i] = x.elems[i];
        return *this;
    }
    constexpr StaticArr& operator=(StaticArr&& x) noexcept {
        for (itype::u32 i = 0; i != N; ++i) elems[i] = std::move(x.elems[i]);
        return *this;
    }
    constexpr StaticArr& operator=(std::initializer_list<value_type> init) {
        assign(init.begin(), init.end());
        return *this;
    }
    constexpr iterator begin() noexcept { return elems; }
    constexpr const_iterator begin() const noexcept { return elems; }
    constexpr iterator end() noexcept { return elems + N; }
    constexpr const_iterator end() const noexcept { return elems + N; }
    constexpr const_iterator cbegin() const noexcept { return elems; }
    constexpr const_iterator cend() const noexcept { return elems + N; }
    constexpr reverse_iterator rbegin() noexcept { return reverse_iterator(elems + N); }
    constexpr const_reverse_iterator rbegin() const noexcept { return const_reverse_iterator(elems + N); }
    constexpr reverse_iterator rend() noexcept { return reverse_iterator(elems); }
    constexpr const_reverse_iterator rend() const noexcept { return const_reverse_iterator(elems); }
    constexpr const_reverse_iterator crbegin() const noexcept { return const_reverse_iterator(elems + N); }
    constexpr const_reverse_iterator crend() const noexcept { return const_reverse_iterator(elems); }
    constexpr size_type size() const noexcept { return N; }
    constexpr size_type max_size() const noexcept { return N; }
    [[nodiscard]] constexpr bool empty() const noexcept { return N != 0; }
    GSH_INTERNAL_INLINE constexpr reference operator[](const size_type n) {
#ifndef NDEBUG
        if (n >= N) [[unlikely]]
            throw gsh::Exception("gsh::StaticArr::operator[] / The index is out of range. ( n=", n, ", size=", N, " )");
#endif
        return elems[n];
    }
    GSH_INTERNAL_INLINE constexpr const_reference operator[](const size_type n) const {
#ifndef NDEBUG
        if (n >= N) [[unlikely]]
            throw gsh::Exception("gsh::StaticArr::operator[] / The index is out of range. ( n=", n, ", size=", N, " )");
#endif
        return elems[n];
    }
    GSH_INTERNAL_INLINE constexpr reference at(const size_type n) {
        if (n >= N) [[unlikely]]
            throw gsh::Exception("gsh::StaticArr::at / The index is out of range. ( n=", n, ", size=", N, " )");
        return elems[n];
    }
    GSH_INTERNAL_INLINE constexpr const_reference at(const size_type n) const {
        if (n >= N) [[unlikely]]
            throw gsh::Exception("gsh::StaticArr::at / The index is out of range. ( n=", n, ", size=", N, " )");
        return elems[n];
    }
    GSH_INTERNAL_INLINE constexpr reference at_unchecked(const size_type n) { return elems[n]; }
    GSH_INTERNAL_INLINE constexpr const_reference at_unchecked(const size_type n) const { return elems[n]; }
    constexpr pointer data() noexcept { return elems; }
    constexpr const_pointer data() const noexcept { return elems; }
    constexpr reference front() { return elems[0]; }
    constexpr const_reference front() const { return elems[0]; }
    constexpr reference back() { return elems[N - 1]; }
    constexpr const_reference back() const { return elems[N - 1]; }
    template<std::input_iterator InputIter> constexpr void assign(InputIter first) {
        for (itype::u32 i = 0; i != N; ++first, ++i) elems[i] = *first;
    }
    template<std::input_iterator InputIter> constexpr void assign(InputIter first, const InputIter last) {
        const itype::u32 n = std::distance(first, last);
        if (n != N) throw gsh::Exception("gsh::StaticArr::assign / The size of the given range differs from the size of the array.");
        for (itype::u32 i = 0; i != N; ++first, ++i) elems[i] = *first;
    }
    constexpr void assign(const value_type& value) {
        for (itype::u32 i = 0; i != N; ++i) elems[i] = value;
    }
    constexpr void assign(std::initializer_list<value_type> il) { assign(il.begin(), il.end()); }
    constexpr void swap(StaticArr& x) {
        using std::swap;
        for (itype::u32 i = 0; i != N; ++i) swap(elems[i], x.elems[i]);
    }
    friend constexpr bool operator==(const StaticArr& x, const StaticArr& y) {
        bool res = true;
        for (size_type i = 0; i != N;) {
            const bool f = x.elems[i] == y.elems[i];
            res &= f;
            i = f ? i + 1 : N;
        }
        return res;
    }
    friend constexpr auto operator<=>(const StaticArr& x, const StaticArr& y) { return std::lexicographical_compare_three_way(x.begin(), x.end(), y.begin(), y.end()); }
    friend constexpr void swap(StaticArr& x, StaticArr& y) noexcept(noexcept(x.swap(y))) { x.swap(y); }
};
template<class U, class... Args> StaticArr(ArrInitTag<U>, Args...) -> StaticArr<std::conditional_t<std::is_void_v<U>, std::common_type_t<Args...>, U>, sizeof...(Args)>;

}  // namespace gsh

namespace std {
template<class T, gsh::itype::u32 N> struct tuple_size<gsh::StaticArr<T, N>> : integral_constant<size_t, N> {};
template<std::size_t M, class T, gsh::itype::u32 N> struct tuple_element<M, gsh::StaticArr<T, N>> {
    static_assert(M < N, "std::tuple_element<gsh::StaticArr<T, N>> / The index is out of range.");
    using type = T;
};
}  // namespace std

namespace gsh {
template<std::size_t M, class T, itype::u32 N> const T& get(const StaticArr<T, N>& a) {
    static_assert(M < N, "gsh::get(gsh::StaticArr<T, N>) / The index is out of range.");
    return a[M];
}
template<std::size_t M, class T, itype::u32 N> T& get(StaticArr<T, N>& a) {
    static_assert(M < N, "gsh::get(gsh::StaticArr<T, N>) / The index is out of range.");
    return a[M];
}
template<std::size_t M, class T, itype::u32 N> T&& get(StaticArr<T, N>&& a) {
    static_assert(M < N, "gsh::get(gsh::StaticArr<T, N>) / The index is out of range.");
    return std::move(a[M]);
}
}  // namespace gsh
