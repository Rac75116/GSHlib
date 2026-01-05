#pragma once
#include "Exception.hpp"
#include "Range.hpp"
#include "Str.hpp"
#include "TypeDef.hpp"
#include "Util.hpp"
#include "internal/UtilMacro.hpp"
namespace gsh {
template<class T, class Alloc = std::allocator<T>> requires std::is_same_v<T, typename std::allocator_traits<Alloc>::value_type> && (!std::is_const_v<T>)class Vec : public ViewInterface<Vec<T, Alloc>, T> {
  using traits = std::allocator_traits<Alloc>;
public:
  using iterator = T*;
  using const_iterator = const T*;
  using reverse_iterator = std::reverse_iterator<iterator>;
  using const_reverse_iterator = std::reverse_iterator<const_iterator>;
  using allocator_type = Alloc;
private:
  [[no_unique_address]] allocator_type alloc;
  T* ptr = nullptr;
  i64 len = 0, cap = 0;
public:
  constexpr Vec() noexcept(noexcept(Alloc())) {}
  constexpr explicit Vec(const allocator_type& a) noexcept : alloc(a) {}
  constexpr explicit Vec(i64 n, const Alloc& a = Alloc()) : alloc(a) {
    if(n == 0) [[unlikely]]
      return;
    ptr = traits::allocate(alloc, static_cast<std::size_t>(n));
    len = n, cap = n;
    for(i64 i = 0; i != n; ++i) traits::construct(alloc, ptr + i);
  }
  constexpr explicit Vec(const i64 n, const T& value, const allocator_type& a = Alloc()) : alloc(a) {
    if(n == 0) [[unlikely]]
      return;
    ptr = traits::allocate(alloc, static_cast<std::size_t>(n));
    len = n, cap = n;
    for(i64 i = 0; i != n; ++i) traits::construct(alloc, ptr + i, value);
  }
  template<std::forward_iterator Iter, std::sentinel_for<Iter> Sent> constexpr Vec(Iter first, Sent last, const allocator_type& a = Alloc()) : alloc(a) {
    const i64 n = static_cast<i64>(std::ranges::distance(first, last));
    if(n == 0) [[unlikely]]
      return;
    ptr = traits::allocate(alloc, static_cast<std::size_t>(n));
    len = n, cap = n;
    i64 i = 0;
    for(; i != n; ++first, ++i) traits::construct(alloc, ptr + i, *first);
  }
  constexpr Vec(const Vec& x) : Vec(x, traits::select_on_container_copy_construction(x.alloc)) {}
  constexpr Vec(Vec&& x) noexcept : alloc(std::move(x.alloc)), ptr(x.ptr), len(x.len), cap(x.cap) { x.ptr = nullptr, x.len = 0, x.cap = 0; }
  constexpr Vec(const Vec& x, const allocator_type& a) : alloc(a), len(x.len), cap(x.len) {
    if(len == 0) [[unlikely]]
      return;
    ptr = traits::allocate(alloc, static_cast<std::size_t>(cap));
    for(i64 i = 0; i != len; ++i) traits::construct(alloc, ptr + i, *(x.ptr + i));
  }
  constexpr Vec(Vec&& x, const allocator_type& a) : alloc(a) {
    if(traits::is_always_equal || x.get_allocator() == a) {
      ptr = x.ptr, len = x.len, cap = x.cap;
      x.ptr = nullptr, x.len = 0, x.cap = 0;
    } else {
      if(x.len == 0) [[unlikely]]
        return;
      len = x.len, cap = x.cap;
      ptr = traits::allocate(alloc, static_cast<std::size_t>(len));
      for(i64 i = 0; i != len; ++i) traits::construct(alloc, ptr + i, std::move(*(x.ptr + i)));
      traits::deallocate(x.alloc, x.ptr, static_cast<std::size_t>(x.cap));
      x.ptr = nullptr, x.len = 0, x.cap = 0;
    }
  }
  constexpr Vec(std::initializer_list<T> il, const allocator_type& a = Alloc()) : Vec(il.begin(), il.end(), a) {}
  constexpr ~Vec() {
    if(cap != 0) {
      for(i64 i = 0; i != len; ++i) traits::destroy(alloc, ptr + i);
      traits::deallocate(alloc, ptr, static_cast<std::size_t>(cap));
    }
  }
  constexpr Vec& operator=(const Vec& x) {
    if(&x == this) return *this;
    for(i64 i = 0; i != len; ++i) traits::destroy(alloc, ptr + i);
    if(traits::propagate_on_container_copy_assignment::value || cap < x.len) {
      if(cap != 0) traits::deallocate(alloc, ptr, static_cast<std::size_t>(cap));
      if constexpr(traits::propagate_on_container_copy_assignment::value) alloc = x.alloc;
      cap = x.len;
      ptr = traits::allocate(alloc, static_cast<std::size_t>(cap));
    }
    len = x.len;
    for(i64 i = 0; i != len; ++i) *(ptr + i) = *(x.ptr + i);
    return *this;
  }
  constexpr Vec& operator=(Vec&& x) noexcept(traits::propagate_on_container_move_assignment::value || traits::is_always_equal::value) {
    if(&x == this) return *this;
    if(cap != 0) {
      for(i64 i = 0; i != len; ++i) traits::destroy(alloc, ptr + i);
      traits::deallocate(alloc, ptr, static_cast<std::size_t>(cap));
    }
    if constexpr(traits::propagate_on_container_move_assignment::value) alloc = std::move(x.alloc);
    ptr = x.ptr, len = x.len, cap = x.cap;
    x.ptr = nullptr, x.len = 0, x.cap = 0;
    return *this;
  }
  constexpr Vec& operator=(std::initializer_list<T> init) {
    assign(init.begin(), init.end());
    return *this;
  }
  constexpr iterator begin() noexcept { return ptr; }
  constexpr const_iterator begin() const noexcept { return ptr; }
  constexpr iterator end() noexcept { return ptr + len; }
  constexpr const_iterator end() const noexcept { return ptr + len; }
  constexpr i64 size() const noexcept { return len; }
  constexpr i64 max_size() const noexcept {
    const auto tmp = traits::max_size(alloc);
    const auto lim = static_cast<decltype(tmp)>(std::numeric_limits<i64>::max());
    return static_cast<i64>(tmp < lim ? tmp : lim);
  }
  constexpr void resize(const i64 sz) {
    if(cap < sz) {
      T* new_ptr = traits::allocate(alloc, static_cast<std::size_t>(sz));
      if(cap != 0) {
        for(i64 i = 0; i != len; ++i) traits::construct(alloc, new_ptr + i, std::move(*(ptr + i)));
        for(i64 i = 0; i != len; ++i) traits::destroy(alloc, ptr + i);
        traits::deallocate(alloc, ptr, static_cast<std::size_t>(cap));
      }
      ptr = new_ptr;
      for(i64 i = len; i != sz; ++i) traits::construct(alloc, ptr + i);
      len = sz, cap = sz;
    } else if(len < sz) {
      for(i64 i = len; i != sz; ++i) traits::construct(alloc, ptr + i);
      len = sz;
    } else {
      for(i64 i = sz; i != len; ++i) traits::destroy(alloc, ptr + i);
      len = sz;
    }
  }
  constexpr void resize(const i64 sz, const T& c) {
    if(cap < sz) {
      T* new_ptr = traits::allocate(alloc, static_cast<std::size_t>(sz));
      if(cap != 0) {
        for(i64 i = 0; i != len; ++i) traits::construct(alloc, new_ptr + i, std::move(*(ptr + i)));
        for(i64 i = 0; i != len; ++i) traits::destroy(alloc, ptr + i);
        traits::deallocate(alloc, ptr, static_cast<std::size_t>(cap));
      }
      ptr = new_ptr;
      for(i64 i = len; i != sz; ++i) traits::construct(alloc, ptr + i, c);
      len = sz, cap = sz;
    } else if(len < sz) {
      for(i64 i = len; i != sz; ++i) traits::construct(alloc, ptr + i, c);
      len = sz;
    } else {
      for(i64 i = sz; i != len; ++i) traits::destroy(alloc, ptr + i);
      len = sz;
    }
  }
  constexpr i64 capacity() const noexcept { return cap; }
  [[nodiscard]] constexpr bool empty() const noexcept { return len == 0; }
  constexpr void reserve(const i64 n) {
    if(n > cap) {
      T* new_ptr = traits::allocate(alloc, static_cast<std::size_t>(n));
      if(cap != 0) {
        for(i64 i = 0; i != len; ++i) traits::construct(alloc, new_ptr + i, std::move(*(ptr + i)));
        for(i64 i = 0; i != len; ++i) traits::destroy(alloc, ptr + i);
        traits::deallocate(alloc, ptr, static_cast<std::size_t>(cap));
      }
      ptr = new_ptr, cap = n;
    }
  }
  constexpr void shrink_to_fit() {
    if(len == 0) {
      if(cap != 0) traits::deallocate(alloc, ptr, static_cast<std::size_t>(cap));
      ptr = nullptr, cap = 0;
      return;
    }
    if(len != cap) {
      T* new_ptr = traits::allocate(alloc, static_cast<std::size_t>(len));
      for(i64 i = 0; i != len; ++i) traits::construct(alloc, new_ptr + i, std::move(*(ptr + i)));
      for(i64 i = 0; i != len; ++i) traits::destroy(alloc, ptr + i);
      traits::deallocate(alloc, ptr, static_cast<std::size_t>(cap));
      ptr = new_ptr, cap = len;
    }
  }
  constexpr T* data() noexcept { return ptr; }
  constexpr const T* data() const noexcept { return ptr; }
  template<std::forward_iterator Iter, std::sentinel_for<Iter> Sent> constexpr void assign(Iter first, Sent last) {
    const i64 n = static_cast<i64>(std::ranges::distance(first, last));
    if(n > cap) {
      for(i64 i = 0; i != len; ++i) traits::destroy(alloc, ptr + i);
      traits::deallocate(alloc, ptr, static_cast<std::size_t>(cap));
      ptr = traits::allocate(alloc, static_cast<std::size_t>(n));
      cap = n;
      for(i64 i = 0; i != n; ++first, ++i) traits::construct(alloc, ptr + i, *first);
    } else if(n > len) {
      i64 i = 0;
      for(; i != len; ++first, ++i) *(ptr + i) = *first;
      for(; i != n; ++first, ++i) traits::construct(alloc, ptr + i, *first);
    } else {
      for(i64 i = n; i != len; ++i) traits::destroy(alloc, ptr + i);
      for(i64 i = 0; i != n; ++first, ++i) *(ptr + i) = *first;
    }
    len = n;
  }
  constexpr void assign(const i64 n, const T& t) {
    if(n > cap) {
      for(i64 i = 0; i != len; ++i) traits::destroy(alloc, ptr + i);
      traits::deallocate(alloc, ptr, static_cast<std::size_t>(cap));
      ptr = traits::allocate(alloc, static_cast<std::size_t>(n));
      cap = n;
      for(i64 i = 0; i != n; ++i) traits::construct(alloc, ptr + i, t);
    } else if(n > len) {
      i64 i = 0;
      for(; i != len; ++i) *(ptr + i) = t;
      for(; i != n; ++i) traits::construct(alloc, ptr + i, t);
    } else {
      for(i64 i = n; i != len; ++i) traits::destroy(alloc, ptr + i);
      for(i64 i = 0; i != n; ++i) *(ptr + i) = t;
    }
    len = n;
  }
  constexpr void assign(std::initializer_list<T> il) { assign(il.begin(), il.end()); }
#ifdef NDEBUG
  GSH_INTERNAL_INLINE
#endif
  constexpr T& operator[](i64 n) {
#ifndef NDEBUG
    if(n >= len) [[unlikely]]
      throw Exception("gsh::Vec::operator[] / The index is out of range. ( n=", n, ", size=", len, " )");
#endif
    return ptr[n];
  };
#ifdef NDEBUG
  GSH_INTERNAL_INLINE
#endif
  constexpr const T& operator[](i64 n) const {
#ifndef NDEBUG
    if(n >= len) [[unlikely]]
      throw Exception("gsh::Vec::operator[] / The index is out of range. ( n=", n, ", size=", len, " )");
#endif
    return ptr[n];
  }
private:
  constexpr void extend_one() {
    if(len == cap) {
      T* new_ptr = traits::allocate(alloc, static_cast<std::size_t>(cap * 2 + 8));
      if(cap != 0) {
        for(i64 i = 0; i != len; ++i) traits::construct(alloc, new_ptr + i, std::move_if_noexcept(*(ptr + i)));
        for(i64 i = 0; i != len; ++i) traits::destroy(alloc, ptr + i);
        traits::deallocate(alloc, ptr, static_cast<std::size_t>(cap));
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
  template<class... Args> constexpr T& emplace_back(Args&&... args) {
    extend_one();
    traits::construct(alloc, ptr + len, std::forward<Args>(args)...);
    return *(ptr + (len++));
  }
  constexpr void pop_back() {
#ifndef NDEBUG
    if(len == 0) [[unlikely]]
      throw gsh::Exception("gsh::Vec::pop_back / The container is empty.");
#endif
    traits::destroy(alloc, ptr + (--len));
  }
  /*
  constexpr iterator insert(const const_iterator position, const T& x);
  constexpr iterator insert(const const_iterator position, T&& x);
  constexpr iterator insert(const const_iterator position, const i64 n, const T& x);
  template<class InputIter> constexpr iterator insert(const const_iterator position, const InputIter first, const InputIter last);
  constexpr iterator insert(const const_iterator position, const std::initializer_list<T> il);
  template<class... Args> constexpr iterator emplace(const_iterator position, Args&&... args);
  constexpr iterator erase(const_iterator position);
  constexpr iterator erase(const_iterator first, const_iterator last);
  */
  constexpr void swap(Vec& x) noexcept(traits::propagate_on_container_swap::value || traits::is_always_equal::value) {
    using std::swap;
    swap(ptr, x.ptr);
    swap(len, x.len);
    swap(cap, x.cap);
    if constexpr(traits::propagate_on_container_swap::value) swap(alloc, x.alloc);
  }
  constexpr void clear() {
    for(i64 i = 0; i != len; ++i) traits::destroy(alloc, ptr + i);
    len = 0;
  }
  constexpr void reset() {
    if(cap != 0) {
      traits::deallocate(alloc, ptr, static_cast<std::size_t>(cap));
      ptr = nullptr, len = 0, cap = 0;
    }
  }
  constexpr void abandon() noexcept { ptr = nullptr, len = 0, cap = 0; }
  constexpr allocator_type get_allocator() const noexcept { return alloc; }
  friend constexpr void swap(Vec& x, Vec& y) noexcept(noexcept(x.swap(y))) { x.swap(y); }
};
template<std::input_iterator InputIter, class Alloc = std::allocator<typename std::iterator_traits<InputIter>::value_type>> Vec(InputIter, InputIter, Alloc = Alloc()) -> Vec<typename std::iterator_traits<InputIter>::value_type, Alloc>;
template<class T, class Alloc = std::allocator<T>> using Vec2 = Vec<Vec<T, Alloc>, typename std::allocator_traits<Alloc>::template rebind_alloc<Vec<T, Alloc>>>;
template<class T, class Alloc = std::allocator<T>> using Vec3 = Vec<Vec<Vec<T, Alloc>, typename std::allocator_traits<Alloc>::template rebind_alloc<Vec<T, Alloc>>>, typename std::allocator_traits<Alloc>::template rebind_alloc<Vec<Vec<T, Alloc>, typename std::allocator_traits<Alloc>::template rebind_alloc<Vec<T, Alloc>>>>>;
} // namespace gsh
