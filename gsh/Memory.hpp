#pragma once
#include "TypeDef.hpp"
#include "internal/UtilMacro.hpp"
#include <cstdlib>
#include <memory>
#include <new>
#include <type_traits>
#include <utility>
namespace gsh {
template<class T> class NoFreeAllocator {
public:
  using value_type = T;
  using propagate_on_container_move_assignment = std::true_type;
  using size_type = u32;
  using difference_type = i32;
  using is_always_equal = std::true_type;
  constexpr NoFreeAllocator() noexcept {}
  constexpr NoFreeAllocator(const NoFreeAllocator&) noexcept {}
  template<class U> constexpr NoFreeAllocator(const NoFreeAllocator<U>&) noexcept {}
  [[nodiscard]] constexpr T* allocate(size_type n) {
    if(std::is_constant_evaluated()) return std::allocator<T>().allocate(n);
    if constexpr(alignof(T) > __STDCPP_DEFAULT_NEW_ALIGNMENT__) return static_cast<T*>(::operator new(sizeof(T) * n, static_cast<std::align_val_t>(alignof(T))));
    else return static_cast<T*>(::operator new(sizeof(T) * n));
  }
  [[nodiscard]] T* allocate(size_type n, std::align_val_t align) { return static_cast<T*>(::operator new(sizeof(T) * n, align)); }
  constexpr void deallocate(T*, size_type) noexcept {}
  void deallocate(T*, size_type, std::align_val_t) noexcept {}
  constexpr NoFreeAllocator& operator=(const NoFreeAllocator&) = default;
  template<class U> friend constexpr bool operator==(const NoFreeAllocator&, const NoFreeAllocator<U>&) noexcept { return true; }
};
template<class T, u32 N> class PoolAllocator {
  c8* cur = buf;
  alignas(T) c8 buf[sizeof(T) * N];
public:
  using value_type = T;
  using propagate_on_container_copy_assignmant = std::true_type;
  using propagate_on_container_move_assignment = std::true_type;
  using propagate_on_container_swap = std::true_type;
  using size_type = u32;
  using difference_type = i32;
  using is_always_equal = std::false_type;
  template<class U> class rebind {
  public:
    ~rebind() = delete;
    using other = PoolAllocator<U, N>;
  };
  constexpr PoolAllocator() noexcept {}
  constexpr PoolAllocator(const PoolAllocator&) noexcept {}
  template<class U, u32 M> constexpr PoolAllocator(const PoolAllocator<U, M>&) noexcept {}
  [[nodiscard]] T* allocate(size_type n) noexcept {
    T* result = reinterpret_cast<T*>(cur);
    cur += sizeof(T) * n;
    return result;
  }
  constexpr void deallocate(T*, size_type) noexcept {}
  constexpr PoolAllocator& operator=(const PoolAllocator&) noexcept {}
  template<class U, u32 M> friend constexpr bool operator==(const PoolAllocator&, const PoolAllocator<U, M>&) noexcept { return false; }
};
template<class T> class SingleAllocator {
  T* buffer[24] = {};
  u32 x = 0xffffffff, y = 0;
  T** del = nullptr;
  u32 end = 0;
  [[no_unique_address]] std::allocator<T> alloc;
  [[no_unique_address]] std::allocator<T*> del_alloc;
  using traits = std::allocator_traits<std::allocator<T>>;
  using del_alloc_traits = std::allocator_traits<std::allocator<T*>>;
public:
  using value_type = T;
  using propagate_on_container_copy_assignmant = std::false_type;
  using propagate_on_container_move_assignment = std::false_type;
  using propagate_on_container_swap = std::false_type;
  using size_type = u32;
  using difference_type = i32;
  using is_always_equal = std::false_type;
  constexpr SingleAllocator() noexcept {}
  constexpr SingleAllocator(const SingleAllocator&) noexcept {}
  template<class U> constexpr SingleAllocator(const SingleAllocator<U>&) noexcept {}
  constexpr ~SingleAllocator() noexcept {
    for(u32 i = 0; i != x + 1; ++i) traits::deallocate(alloc, buffer[i], 1 << i);
    if(del != nullptr) del_alloc_traits::deallocate(del_alloc, del, (1u << (x + 1)) - 1);
  }
  constexpr SingleAllocator& operator=(const SingleAllocator&) noexcept {}
  constexpr T* allocate(u32) {
    if(y == (1u << (x + 1)) >> 1) {
      if(end != 0) [[likely]] {
        return del[--end];
      } else {
        x += 1, y = 0;
        buffer[x] = traits::allocate(alloc, 1 << x);
        T** new_del = del_alloc_traits::allocate(del_alloc, (1 << (x + 1)) - 1);
        if(del != nullptr) [[likely]] {
          for(u32 i = 0; i != end; ++i) new_del[i] = del[i];
          del_alloc_traits::deallocate(del_alloc, del, (1 << x) - 1);
        }
        del = new_del;
      }
    }
    return &buffer[x][y++];
  }
  constexpr void deallocate(T* p, u32) noexcept { del[end++] = p; }
  constexpr u32 max_size() const noexcept { return (1 << 24) - 1; }
  constexpr SingleAllocator select_on_container_copy_construction() const noexcept { return {}; }
  template<class U> friend constexpr bool operator==(const SingleAllocator&, const SingleAllocator<U>&) noexcept { return false; }
  template<class... Args> constexpr void construct(T* p, Args&&... args) { std::construct_at(p, std::forward<Args>(args)...); }
};
namespace internal {
template<class Alloc, bool OmitDestruction> struct SharedAllocatorImpl {
  static inline Alloc alloc{};
  static constexpr Alloc& get() noexcept { return alloc; }
};
template<class Alloc> struct SharedAllocatorImpl<Alloc, true> {
  static inline Alloc* alloc = new Alloc();
  static constexpr Alloc& get() noexcept { return *alloc; }
};
}
template<class Alloc, bool OmitDestruction = false> class SharedAllocator {
  using alloc = internal::SharedAllocatorImpl<Alloc, OmitDestruction>;
  using traits = std::allocator_traits<Alloc>;
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
    using other = SharedAllocator<typename traits::template rebind_alloc<U>, OmitDestruction>;
  };
  constexpr SharedAllocator() noexcept {}
  constexpr SharedAllocator(const SharedAllocator&) noexcept = default;
  template<class T> constexpr SharedAllocator(const SharedAllocator<T>&) noexcept {}
  constexpr SharedAllocator& operator=(const SharedAllocator&) noexcept = default;
  template<class... Args> auto allocate(Args&&... args) noexcept(noexcept(alloc::get().allocate(std::forward<Args>(args)...))) { return alloc::get().allocate(std::forward<Args>(args)...); }
  template<class... Args> void deallocate(Args&&... args) noexcept(noexcept(alloc::get().deallocate(std::forward<Args>(args)...))) { return alloc::get().deallocate(std::forward<Args>(args)...); }
  size_type max_size() const noexcept { return alloc::get().max_size(); }
  static Alloc& get_allocator() noexcept { return alloc::get(); }
  template<class T> friend constexpr bool operator==(const SharedAllocator&, const SharedAllocator<T>&) noexcept { return true; }
};
template<class Alloc> class ConstexprAllocator {
  [[no_unique_address]] Alloc alloc;
  using traits = std::allocator_traits<Alloc>;
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
    if(std::is_constant_evaluated()) return std::allocator<value_type>().allocate(std::forward<Args>(args)...);
    else return alloc.allocate(std::forward<Args>(args)...);
  }
  template<class... Args> constexpr void deallocate(Args&&... args) noexcept(noexcept(alloc.deallocate(std::forward<Args>(args)...))) {
    if(std::is_constant_evaluated()) return std::allocator<value_type>().deallocate(std::forward<Args>(args)...);
    else return alloc.deallocate(std::forward<Args>(args)...);
  }
  constexpr size_type max_size() const noexcept {
    if(std::is_constant_evaluated()) return std::allocator_traits<std::allocator<value_type>>::max_size();
    else return alloc.max_size();
  }
  constexpr Alloc& get_allocator() noexcept { return alloc; }
  template<class U> friend constexpr bool operator==(const ConstexprAllocator& a, const ConstexprAllocator<U>& b) noexcept(noexcept(a.alloc == b.alloc)) { return a.alloc == b.alloc; }
};
template<class T> class Mem {
  u32 sz;
  T* ptr;
  GSH_INTERNAL_INLINE constexpr void copy(const Mem& v) {
    sz = v.sz;
    ptr = std::allocator<T>{}.allocate(sz);
    for(u32 i = 0; i != sz; ++i) ptr[i] = v[i];
  }
  GSH_INTERNAL_INLINE constexpr void move(Mem& v) {
    sz = v.sz;
    ptr = v.ptr;
    v.sz = 0;
    v.ptr = nullptr;
  }
public:
  constexpr Mem() : sz(0), ptr(nullptr) {}
  constexpr Mem(u32 n) : sz(n), ptr(std::allocator<T>().allocate(n)) {}
  constexpr Mem(u32 n, const T& value) : sz(n), ptr(std::allocator<T>().allocate(n)) {
    for(u32 i = 0; i != n; ++i) std::construct_at(ptr + i, value);
  }
  constexpr Mem(const Mem& v) { copy(v); }
  constexpr Mem(Mem&& v) { move(v); }
  constexpr ~Mem() noexcept { clear(); }
  GSH_INTERNAL_INLINE constexpr Mem& operator=(const Mem& v) {
    if(ptr != v.ptr) clear(), copy(v);
    return *this;
  }
  GSH_INTERNAL_INLINE constexpr Mem& operator=(Mem&& v) noexcept {
    if(ptr != v.ptr) clear(), move(v);
    return *this;
  }
  GSH_INTERNAL_INLINE constexpr void clear() noexcept {
    if(ptr != nullptr) {
      if constexpr(!std::is_trivially_destructible_v<T>)
        for(u32 i = 0; i != sz; ++i) std::destroy_at(ptr + i);
      std::allocator<T>().deallocate(ptr, sz);
      sz = 0;
      ptr = nullptr;
    }
  }
  GSH_INTERNAL_INLINE constexpr u32 size() const { return sz; }
  GSH_INTERNAL_INLINE constexpr bool empty() const { return sz == 0; }
  GSH_INTERNAL_INLINE constexpr T* data() { return ptr; }
  GSH_INTERNAL_INLINE constexpr const T* data() const { return ptr; }
  GSH_INTERNAL_INLINE constexpr T& operator[](u32 n) { return *(ptr + n); }
  GSH_INTERNAL_INLINE constexpr const T& operator[](u32 n) const { return *(ptr + n); }
};
}
