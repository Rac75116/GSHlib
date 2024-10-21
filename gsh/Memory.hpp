#pragma once
#include <type_traits>  // std::make_unsigned, std::is_empty, std::true_type, std::false_type, std::is_array_v, std::is_trivially_(***)
#include <limits>       // std::numeric_limits
#include <utility>      // std::forward
#include <iterator>     // std::begin, std::end
#include <cstdlib>      // std::malloc, std::free, std::realloc, std::aligned_alloc
#include <new>          // ::operator new
#include <memory>       // std::construct_at, std::destroy_at
#include "TypeDef.hpp"  // gsh::itype

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
        if constexpr (!std::is_trivially_constructible_v<T, Args...>) std::construct_at(p, std::forward<Args>(args)...);
    }
    template<class T> static constexpr void destroy(Alloc& a, T* p) noexcept(noexcept(a.destroy(p)))
        requires(destructible)
    {
        return a.destroy(p);
    }
    template<class T> static constexpr void destroy(Alloc&, T* p) noexcept(std::is_nothrow_destructible_v<T>)
        requires(!destructible)
    {
        if constexpr (!std::is_trivially_destructible_v<T>) return std::destroy_at(p);
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
    constexpr Allocator() noexcept = default;
    constexpr Allocator(const Allocator&) noexcept = default;
    template<class U> constexpr Allocator(const Allocator<U>&) noexcept {}
    [[nodiscard]] constexpr T* allocate(size_type n) {
        if constexpr (alignof(T) > __STDCPP_DEFAULT_NEW_ALIGNMENT__) return static_cast<T*>(::operator new(sizeof(T) * n, static_cast<std::align_val_t>(alignof(T))));
        else return static_cast<T*>(::operator new(sizeof(T) * n));
    }
    [[nodiscard]] constexpr T* allocate(size_type n, std::align_val_t align) { return static_cast<T*>(::operator new(sizeof(T) * n, align)); }
    constexpr void deallocate(T* p, [[maybe_unused]] size_type n) noexcept {
#ifdef __cpp_sized_deallocation
        if constexpr (alignof(T) > __STDCPP_DEFAULT_NEW_ALIGNMENT__) ::operator delete(p, n, static_cast<std::align_val_t>(alignof(T)));
        else ::operator delete(p, n);
#else
        if constexpr (alignof(T) > __STDCPP_DEFAULT_NEW_ALIGNMENT__) ::operator delete(p, static_cast<std::align_val_t>(alignof(T)));
        else ::operator delete(p);
#endif
    }
    constexpr void deallocate(T* p, [[maybe_unused]] size_type n, std::align_val_t align) noexcept {
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
        if (std::is_constant_evaluated()) {
            if constexpr (alignof(T) > __STDCPP_DEFAULT_NEW_ALIGNMENT__) return static_cast<T*>(::operator new(sizeof(T) * n, static_cast<std::align_val_t>(alignof(T))));
            else return static_cast<T*>(::operator new(sizeof(T) * n));
        }
        constexpr itype::u32 align = __STDCPP_DEFAULT_NEW_ALIGNMENT__ < alignof(T) ? alignof(T) : __STDCPP_DEFAULT_NEW_ALIGNMENT__;
        void* ptr = static_cast<void*>(buf + *cnt);
        std::size_t space = end - static_cast<ctype::c8*>(ptr);
        std::align(align, sizeof(T) * n, ptr, space);
        if (ptr == nullptr) throw Exception("gsh::PoolAllocator::allocate / Failed to allocate memory.");
        T* res = static_cast<T*>(ptr);
        *cnt = static_cast<ctype::c8*>(ptr) - buf;
        return res;
    }
    [[nodiscard]] constexpr T* allocate(size_type n, std::align_val_t align) {
        if (std::is_constant_evaluated()) return static_cast<T*>(::operator new(sizeof(T) * n, align));
        void* ptr = static_cast<void*>(buf + *cnt);
        std::size_t space = end - static_cast<ctype::c8*>(ptr);
        std::align(static_cast<std::size_t>(align), sizeof(T) * n, ptr, space);
        if (ptr == nullptr) throw Exception("gsh::PoolAllocator::allocate / Failed to allocate memory.");
        T* res = static_cast<T*>(ptr);
        *cnt = static_cast<ctype::c8*>(ptr) - buf;
        return res;
    }
    constexpr void deallocate(T* p, [[maybe_unused]] size_type n) {
        if (std::is_constant_evaluated()) {
#ifdef __cpp_sized_deallocation
            if constexpr (alignof(T) > __STDCPP_DEFAULT_NEW_ALIGNMENT__) ::operator delete(p, n, static_cast<std::align_val_t>(alignof(T)));
            else ::operator delete(p, n);
#else
            if constexpr (alignof(T) > __STDCPP_DEFAULT_NEW_ALIGNMENT__) ::operator delete(p, static_cast<std::align_val_t>(alignof(T)));
            else ::operator delete(p);
#endif
        }
    }
    constexpr void deallocate(T* p, [[maybe_unused]] size_type n, std::align_val_t align) {
        if (std::is_constant_evaluated()) {
#ifdef __cpp_sized_deallocation
            ::operator delete(p, n, align);
#else
            ::operator delete(p, align);
#endif
        }
    }
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

}  // namespace gsh
