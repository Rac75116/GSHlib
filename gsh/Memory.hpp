#pragma once
#include <type_traits>    // std::make_unsigned, std::is_empty, std::true_type, std::false_type, std::is_array_v, std::is_trivially_(***)
#include <limits>         // std::numeric_limits
#include <utility>        // std::forward
#include <iterator>       // std::begin, std::end
#include <cstdlib>        // std::malloc, std::free, std::realloc, std::aligned_alloc
#include <new>            // ::operator new
#include <memory>         // std::construct_at, std::destroy_at
#include "TypeDef.hpp"    // gsh::itype
#include "Exception.hpp"  // gsh::Exception

namespace gsh {

namespace internal {
    template<class T, class U> struct GetPtr {
        using type = U*;
    };
    template<class T, class U>
        requires requires { typename T::pointer; }
    struct GetPtr<T, U> {
        using type = typename T::pointer;
    };
    template<class T, class U> struct RepFirst {};
    template<template<class, class...> class SomeTemplate, class U, class T, class... Types> struct RepFirst<SomeTemplate<T, Types...>, U> {
        using type = SomeTemplate<U, Types...>;
    };
    template<class T, class U> struct Rebind {
        using type = typename RepFirst<T, U>::type;
    };
    template<class T, class U>
        requires requires { typename T::template rebind<U>; }
    struct Rebind<T, U> {
        using type = typename T::template rebind<U>;
    };
    template<class T, class U> struct GetRebindPtr {
        using type = typename Rebind<T, U>::type;
    };
    template<class T, class U> struct GetRebindPtr<T*, U> {
        using type = U;
    };
    template<class T, class U, class V> struct GetConstPtr {
        using type = typename GetRebindPtr<U, const V*>::type;
    };
    template<class T, class U, class V>
        requires requires { typename T::const_pointer; }
    struct GetConstPtr<T, U, V> {
        using type = typename T::const_pointer;
    };
    template<class T, class U> struct GetVoidPtr {
        using type = typename GetRebindPtr<U, void*>::type;
    };
    template<class T, class U>
        requires requires { typename T::void_pointer; }
    struct GetVoidPtr<T, U> {
        using type = typename T::void_pointer;
    };
    template<class T, class U> struct GetConstVoidPtr {
        using type = typename GetRebindPtr<U, const void*>::type;
    };
    template<class T, class U>
        requires requires { typename T::const_void_pointer; }
    struct GetConstVoidPtr<T, U> {
        using type = typename T::const_void_pointer;
    };
    template<class T> struct GetDifferenceTypeSub {
        using type = itype::i32;
    };
    template<class T>
        requires requires { typename T::difference_type; }
    struct GetDifferenceTypeSub<T> {
        using type = typename T::difference_type;
    };
    template<class T, class U> struct GetDifferenceType {
        using type = typename GetDifferenceTypeSub<U>::type;
    };
    template<class T, class U>
        requires requires { typename T::difference_type; }
    struct GetDifferenceType<T, U> {
        using type = typename T::difference_type;
    };
    template<class T, class U> struct GetSizeType {
        using type = std::make_unsigned_t<U>;
    };
    template<class T, class U>
        requires requires { typename T::size_type; }
    struct GetSizeType<T, U> {
        using type = typename T::size_type;
    };
    template<class T> struct IsPropCopy {
        using type = std::false_type;
    };
    template<class T>
        requires requires { typename T::propagate_on_container_copy_assignment; }
    struct IsPropCopy<T> {
        using type = typename T::propagate_on_container_copy_assignment;
    };
    template<class T> struct IsPropMove {
        using type = std::false_type;
    };
    template<class T>
        requires requires { typename T::propagate_on_container_move_assignment; }
    struct IsPropMove<T> {
        using type = typename T::propagate_on_container_move_assignment;
    };
    template<class T> struct IsPropSwap {
        using type = std::false_type;
    };
    template<class T>
        requires requires { typename T::propagate_on_container_swap; }
    struct IsPropSwap<T> {
        using type = typename T::propagate_on_container_swap;
    };
    template<class T> struct IsAlwaysEqual {
        using type = typename std::is_empty<T>::type;
    };
    template<class T>
        requires requires { typename T::is_always_equal; }
    struct IsAlwaysEqual<T> {
        using type = typename T::is_always_equal;
    };
    template<class T, class U> struct RebindAlloc {
        using type = typename internal::RepFirst<T, U>::type;
    };
    template<class T, class U>
        requires requires { typename T::template rebind<U>::other; }
    struct RebindAlloc<T, U> {
        using type = typename T::template rebind<U>::other;
    };
}  // namespace internal

template<class Alloc> class AllocatorTraits {
public:
    using allocator_type = Alloc;
    using value_type = typename Alloc::value_type;
    using pointer = typename internal::GetPtr<Alloc, value_type>::type;
    using const_pointer = typename internal::GetConstPtr<Alloc, pointer, value_type>::type;
    using void_pointer = typename internal::GetVoidPtr<Alloc, pointer>::type;
    using const_void_pointer = typename internal::GetConstVoidPtr<Alloc, pointer>::type;
    using difference_type = typename internal::GetDifferenceType<Alloc, pointer>::type;
    using size_type = typename internal::GetSizeType<Alloc, difference_type>::type;
    using propagate_on_container_copy_assignment = typename internal::IsPropCopy<Alloc>::type;
    using propagate_on_container_move_assignment = typename internal::IsPropMove<Alloc>::type;
    using propagate_on_container_swap = typename internal::IsPropSwap<Alloc>::type;
    using is_always_equal = typename internal::IsAlwaysEqual<Alloc>::type;
    template<class U> using rebind_alloc = typename internal::RebindAlloc<Alloc, U>::type;
    template<class U> using rebind_traits = AllocatorTraits<typename internal::RebindAlloc<Alloc, U>::type>;
private:
    constexpr static bool with_hint = requires(Alloc& a, size_type n, const_void_pointer hint) { a.allocate(n, hint); };
    constexpr static bool aligned_with_hint = requires(Alloc& a, size_type n, std::align_val_t align, const_void_pointer hint) { a.allocate(n, align, hint); };
    template<class... Args> constexpr static bool constructible = requires(Alloc& a, pointer p, Args&&... args) { a.construct(p, std::forward<Args>(args)...); };
    constexpr static bool destructible = requires(Alloc& a, pointer p) { a.destroy(p); };
    constexpr static bool selectable = requires(Alloc& a) { a.select_on_container_copy_construction(); };
public:
    [[nodiscard]] static constexpr pointer allocate(Alloc& a, size_type n) noexcept(noexcept(a.allocate(n))) { return a.allocate(n); }
    [[nodiscard]] static constexpr pointer allocate(Alloc& a, size_type n, std::align_val_t align) noexcept(noexcept(a.allocate(n, align))) { return a.allocate(n, align); }
    [[nodiscard]] static constexpr pointer allocate(Alloc& a, size_type n, const_void_pointer hint) noexcept(noexcept(a.allocate(n, hint)))
        requires(with_hint)
    {
        return a.allocate(n, hint);
    }
    [[nodiscard]] static constexpr pointer allocate(Alloc& a, size_type n, const_void_pointer) noexcept(noexcept(a.allocate(n)))
        requires(!with_hint)
    {
        return a.allocate(n);
    }
    [[nodiscard]] static constexpr pointer allocate(Alloc& a, size_type n, std::align_val_t align, const_void_pointer hint) noexcept(noexcept(a.allocate(n, align, hint)))
        requires(aligned_with_hint)
    {
        return a.allocate(n, align, hint);
    }
    [[nodiscard]] static constexpr pointer allocate(Alloc& a, size_type n, std::align_val_t align, const_void_pointer) noexcept(noexcept(a.allocate(align, n)))
        requires(!aligned_with_hint)
    {
        return a.allocate(align, n);
    }
    static constexpr void deallocate(Alloc& a, pointer p, size_type n) noexcept(noexcept(a.deallocate(p, n))) { a.deallocate(p, n); }
    static constexpr void deallocate(Alloc& a, pointer p, size_type n, std::align_val_t align) noexcept(noexcept(a.deallocate(p, n, align))) { a.deallocate(p, n, align); }
    static constexpr size_type max_size(const Alloc& a) noexcept {
        if constexpr (requires { a.max_size(); }) return a.max_size();
        else return std::numeric_limits<size_type>::max() / sizeof(value_type);
    }
    template<class T, class... Args> static constexpr void construct(Alloc& a, T* p, Args&&... args) noexcept(noexcept(a.construct(p, std::forward<Args>(args)...)))
        requires(constructible<Args...>)
    {
        a.construct(p, std::forward<Args>(args)...);
    }
    template<class T, class... Args> static constexpr void construct(Alloc&, T* p, Args&&... args) noexcept(std::is_nothrow_constructible_v<T, Args...>)
        requires(!constructible<Args...>)
    {
        std::construct_at(p, std::forward<Args>(args)...);
    }
    template<class T> static constexpr void destroy(Alloc& a, T* p) noexcept(noexcept(a.destroy(p)))
        requires(destructible)
    {
        return a.destroy(p);
    }
    template<class T> static constexpr void destroy(Alloc&, T* p) noexcept(std::is_nothrow_destructible_v<T>)
        requires(!destructible)
    {
        return std::destroy_at(p);
    }
    static constexpr Alloc select_on_container_copy_construction(const Alloc& a) noexcept(noexcept(a.select_on_container_copy_construction()))
        requires(selectable)
    {
        return a.select_on_container_copy_construction();
    }
    static constexpr Alloc select_on_container_copy_construction(const Alloc& a) noexcept(std::is_nothrow_copy_constructible_v<Alloc>)
        requires(!selectable)
    {
        return a;
    }
};

template<class T> class Allocator {
public:
    using value_type = T;
    using propagate_on_container_move_assignment = std::true_type;
    using size_type = itype::u32;
    using difference_type = itype::i32;
    using is_always_equal = std::true_type;
    constexpr Allocator() noexcept {}
    constexpr Allocator(const Allocator&) noexcept {}
    template<class U> constexpr Allocator(const Allocator<U>&) noexcept {}
    [[nodiscard]] constexpr T* allocate(size_type n) {
        if (std::is_constant_evaluated()) return std::allocator<T>().allocate(n);
        if constexpr (alignof(T) > __STDCPP_DEFAULT_NEW_ALIGNMENT__) return static_cast<T*>(::operator new(sizeof(T) * n, static_cast<std::align_val_t>(alignof(T))));
        else return static_cast<T*>(::operator new(sizeof(T) * n));
    }
    [[nodiscard]] T* allocate(size_type n, std::align_val_t align) { return static_cast<T*>(::operator new(sizeof(T) * n, align)); }
    constexpr void deallocate(T* p, [[maybe_unused]] size_type n) noexcept {
        if (std::is_constant_evaluated()) return std::allocator<T>().deallocate(p, n);
#ifdef __cpp_sized_deallocation
        if constexpr (alignof(T) > __STDCPP_DEFAULT_NEW_ALIGNMENT__) ::operator delete(p, n, static_cast<std::align_val_t>(alignof(T)));
        else ::operator delete(p, n);
#else
        if constexpr (alignof(T) > __STDCPP_DEFAULT_NEW_ALIGNMENT__) ::operator delete(p, static_cast<std::align_val_t>(alignof(T)));
        else ::operator delete(p);
#endif
    }
    void deallocate(T* p, [[maybe_unused]] size_type n, std::align_val_t align) noexcept {
#ifdef __cpp_sized_deallocation
        ::operator delete(p, n, align);
#else
        ::operator delete(p, align);
#endif
    }
    constexpr Allocator& operator=(const Allocator&) = default;
    template<class U> friend constexpr bool operator==(const Allocator&, const Allocator<U>&) noexcept { return true; }
};

template<itype::u32 Size> class MemoryPool {
    template<class T> friend class PoolAllocator;
    itype::u32 cnt = 0;
    itype::u32 ref = 0;
    ctype::c8 buf[Size];
public:
    constexpr ~MemoryPool() noexcept(false) {
        if (ref != 0) throw Exception("gsh::MemoryPool::~MemoryPool / There are some gsh::PoolAllocator tied to this object have not yet been destroyed.");
    }
};
template<class T> class PoolAllocator {
    template<class U> friend class PoolAllocator;
    itype::u32* cnt;
    itype::u32* ref;
    ctype::c8 *buf, *end;
public:
    using value_type = T;
    using propagate_on_container_copy_assignmant = std::true_type;
    using propagate_on_container_move_assignment = std::true_type;
    using propagate_on_container_swap = std::true_type;
    using size_type = itype::u32;
    using difference_type = itype::i32;
    using is_always_equal = std::false_type;
    constexpr PoolAllocator() noexcept : cnt(nullptr), ref(nullptr), buf(nullptr), end(nullptr) {}
    constexpr PoolAllocator(const PoolAllocator& a) noexcept : cnt(a.cnt), ref(a.ref), buf(a.buf), end(a.end) { ++*ref; }
    template<class U> constexpr PoolAllocator(const PoolAllocator<U>& a) noexcept : cnt(a.cnt), ref(a.ref), buf(a.buf), end(a.end) { ++*ref; }
    template<itype::u32 Size> constexpr PoolAllocator(MemoryPool<Size>& p) noexcept : cnt(&p.cnt), ref(&p.ref), buf(p.buf), end(p.buf + Size) { ++*ref; }
    constexpr ~PoolAllocator() noexcept {
        if (ref != nullptr) --*ref;
    }
    [[nodiscard]] constexpr T* allocate(size_type n) {
        if (std::is_constant_evaluated()) return Allocator<T>().allocate(n);
        constexpr itype::u32 align = __STDCPP_DEFAULT_NEW_ALIGNMENT__ < alignof(T) ? alignof(T) : __STDCPP_DEFAULT_NEW_ALIGNMENT__;
        void* ptr = static_cast<void*>(buf + *cnt);
        std::size_t space = end - static_cast<ctype::c8*>(ptr);
        std::align(align, sizeof(T) * n, ptr, space);
        if (ptr == nullptr) throw Exception("gsh::PoolAllocator::allocate / Failed to allocate memory.");
        T* res = static_cast<T*>(ptr);
        *cnt = static_cast<ctype::c8*>(ptr) - buf;
        return res;
    }
    [[nodiscard]] T* allocate(size_type n, std::align_val_t align) {
        void* ptr = static_cast<void*>(buf + *cnt);
        std::size_t space = end - static_cast<ctype::c8*>(ptr);
        std::align(static_cast<std::size_t>(align), sizeof(T) * n, ptr, space);
        if (ptr == nullptr) throw Exception("gsh::PoolAllocator::allocate / Failed to allocate memory.");
        T* res = static_cast<T*>(ptr);
        *cnt = static_cast<ctype::c8*>(ptr) - buf;
        return res;
    }
    constexpr void deallocate(T* p, [[maybe_unused]] size_type n) noexcept {
        if (std::is_constant_evaluated()) return Allocator<T>().deallocate(p, n);
    }
    void deallocate(T*, size_type, std::align_val_t) noexcept {}
    constexpr PoolAllocator& operator=(const PoolAllocator& a) noexcept {
        if (ref != nullptr) --*ref;
        cnt = a.cnt, ref = a.ref, buf = a.buf, end = a.end;
        ++*ref;
        return *this;
    }
    template<itype::u32 Size> constexpr PoolAllocator& operator=(MemoryPool<Size>& p) noexcept {
        if (ref != nullptr) --*ref;
        cnt = &p.cnt, ref = &p.ref, buf = p.buf, end = p.buf + Size;
        ++*ref;
        return *this;
    }
    template<class U> friend constexpr bool operator==(const PoolAllocator& a, const PoolAllocator<U>& b) noexcept { return a.cnt == b.cnt && a.ref == b.ref && a.buf == b.buf && a.end == b.end; }
};

template<class T> class SingleAllocator {
    T* buffer[24] = {};
    itype::u32 x = 0xffffffff, y = 0;
    T** del = nullptr;
    itype::u32 end = 0;
    [[no_unique_address]] Allocator<T> alloc;
    [[no_unique_address]] Allocator<T*> del_alloc;
    using traits = AllocatorTraits<Allocator<T>>;
    using del_alloc_traits = AllocatorTraits<Allocator<T*>>;
public:
    using value_type = T;
    using propagate_on_container_copy_assignmant = std::false_type;
    using propagate_on_container_move_assignment = std::false_type;
    using propagate_on_container_swap = std::false_type;
    using size_type = itype::u32;
    using difference_type = itype::i32;
    using is_always_equal = std::false_type;
    constexpr SingleAllocator() noexcept {}
    constexpr SingleAllocator(const SingleAllocator&) noexcept {}
    template<class U> constexpr SingleAllocator(const SingleAllocator<U>&) noexcept {}
    constexpr ~SingleAllocator() noexcept {
        for (itype::u32 i = 0; i != x + 1; ++i) traits::deallocate(alloc, buffer[i], 1 << i);
        if (del != nullptr) del_alloc_traits::deallocate(del_alloc, del, (1u << (x + 1)) - 1);
    }
    constexpr SingleAllocator& operator=(const SingleAllocator&) noexcept {}
    constexpr T* allocate(itype::u32) {
        if (y == (1u << (x + 1)) >> 1) {
            if (end != 0) [[likely]] {
                return del[--end];
            } else {
                x += 1, y = 0;
                buffer[x] = traits::allocate(alloc, 1 << x);
                T** new_del = del_alloc_traits::allocate(del_alloc, (1 << (x + 1)) - 1);
                if (del != nullptr) [[likely]] {
                    for (itype::u32 i = 0; i != end; ++i) new_del[i] = del[i];
                    del_alloc_traits::deallocate(del_alloc, del, (1 << x) - 1);
                }
                del = new_del;
            }
        }
        return &buffer[x][y++];
    }
    constexpr void deallocate(T* p, itype::u32) noexcept { del[end++] = p; }
    constexpr itype::u32 max_size() const noexcept { return (1 << 24) - 1; }
    constexpr SingleAllocator select_on_container_copy_construction() const noexcept { return {}; }
    template<class U> friend constexpr bool operator==(const SingleAllocator&, const SingleAllocator<U>&) noexcept { return false; }
};

template<class Alloc> class SharedAllocator {
    static inline Alloc alloc;
    using traits = AllocatorTraits<Alloc>;
public:
    using value_type = typename traits::value_type;
    using propagate_on_container_copy_assignmant = std::true_type;
    using propagate_on_container_move_assignment = std::true_type;
    using propagate_on_container_swap = std::true_type;
    using size_type = typename traits::size_type;
    using difference_type = typename traits::difference_type;
    using is_always_equal = std::true_type;
    using allocator_type = Alloc;
    template<class U> class rebind {
    public:
        ~rebind() = delete;
        using other = SharedAllocator<typename traits::template rebind_alloc<U>>;
    };
    constexpr SharedAllocator() noexcept {}
    constexpr SharedAllocator(const SharedAllocator&) noexcept = default;
    template<class T> constexpr SharedAllocator(const SharedAllocator<T>&) noexcept {}
    constexpr SharedAllocator& operator=(const SharedAllocator&) noexcept = default;
    template<class... Args> auto allocate(Args&&... args) noexcept(noexcept(alloc.allocate(std::forward<Args>(args)...))) { return alloc.allocate(std::forward<Args>(args)...); }
    template<class... Args> void deallocate(Args&&... args) noexcept(noexcept(alloc.deallocate(std::forward<Args>(args)...))) { return alloc.deallocate(std::forward<Args>(args)...); }
    size_type max_size() const noexcept { return alloc.max_size(); }
    static Alloc& get_allocator() noexcept { return alloc; }
    template<class T> friend constexpr bool operator==(const SharedAllocator&, const SharedAllocator<T>&) noexcept { return true; }
};

template<class Alloc> class ConstexprAllocator {
    [[no_unique_address]] Alloc alloc;
    using traits = AllocatorTraits<Alloc>;
public:
    using allocator_type = Alloc;
    using value_type = typename traits::value_type;
    using pointer = typename traits::pointer;
    using const_pointer = typename traits::const_pointer;
    using void_pointer = typename traits::void_pointer;
    using const_void_pointer = typename traits::const_void_pointer;
    using difference_type = typename traits::difference_type;
    using size_type = typename traits::size_type;
    using propagate_on_container_copy_assignment = typename traits::propagate_on_container_copy_assignment;
    using propagate_on_container_move_assignment = typename traits::propagate_on_container_move_assignment;
    using propagate_on_container_swap = typename traits::propagate_on_container_swap;
    using is_always_equal = typename traits::is_always_equal;
    template<class U> class rebind {
    public:
        ~rebind() = delete;
        using other = ConstexprAllocator<typename traits::template rebind_alloc<U>>;
    };
    constexpr ConstexprAllocator() noexcept(noexcept(Alloc())) {}
    constexpr ConstexprAllocator(const ConstexprAllocator&) noexcept(std::is_nothrow_copy_constructible_v<Alloc>) = default;
    constexpr ConstexprAllocator(ConstexprAllocator&&) noexcept(std::is_nothrow_move_constructible_v<Alloc>) = default;
    template<class U> constexpr ConstexprAllocator(const ConstexprAllocator<U>& a) noexcept(std::is_nothrow_constructible_v<Alloc, const U&>) : alloc(a.alloc) {}
    template<class U> constexpr ConstexprAllocator(ConstexprAllocator<U>&& a) noexcept(std::is_nothrow_constructible_v<Alloc, U&&>) : alloc(std::move(a.alloc)) {}
    constexpr ConstexprAllocator& operator=(const ConstexprAllocator& a) noexcept(std::is_nothrow_copy_assignable_v<Alloc>) {
        alloc = a.alloc;
        return *this;
    }
    constexpr ConstexprAllocator& operator=(ConstexprAllocator&& a) noexcept(std::is_nothrow_move_assignable_v<Alloc>) {
        alloc = std::move(a.alloc);
        return *this;
    }
    template<class... Args> constexpr auto allocate(Args&&... args) noexcept(noexcept(alloc.allocate(std::forward<Args>(args)...))) {
        if (std::is_constant_evaluated()) return Allocator<value_type>().allocate(std::forward<Args>(args)...);
        else return alloc.allocate(std::forward<Args>(args)...);
    }
    template<class... Args> constexpr void deallocate(Args&&... args) noexcept(noexcept(alloc.deallocate(std::forward<Args>(args)...))) {
        if (std::is_constant_evaluated()) return Allocator<value_type>().deallocate(std::forward<Args>(args)...);
        else return alloc.deallocate(std::forward<Args>(args)...);
    }
    constexpr size_type max_size() const noexcept {
        if (std::is_constant_evaluated()) return AllocatorTraits<Allocator<value_type>>::max_size();
        else return alloc.max_size();
    }
    constexpr Alloc& get_allocator() noexcept { return alloc; }
    template<class U> friend constexpr bool operator==(const ConstexprAllocator& a, const ConstexprAllocator<U>& b) noexcept(noexcept(a.alloc == b.alloc)) { return a.alloc == b.alloc; }
};

}  // namespace gsh
