#pragma once
#include "Exception.hpp"
#include "Range.hpp"
#include "Str.hpp"
#include "TypeDef.hpp"
#include "Util.hpp"
#include "internal/UtilMacro.hpp"
#include <algorithm>
#include <initializer_list>
#include <iterator>
#include <type_traits>
#include <utility>
namespace gsh {
template<class T, class Alloc = std::allocator<T>>
requires std::is_same_v<T, typename std::allocator_traits<Alloc>::value_type> && (!std::is_const_v<T>)
class Vec : public ViewInterface<Vec<T, Alloc>, T> {
  using traits = std::allocator_traits<Alloc>;
public:
  using reference = T&;
  using const_reference = const T&;
  using iterator = T*;
  using const_iterator = const T*;
  using size_type = u32;
  using difference_type = i32;
  using value_type = T;
  using allocator_type = Alloc;
  using pointer = typename traits::pointer;
  using const_pointer = typename traits::const_pointer;
  using reverse_iterator = std::reverse_iterator<iterator>;
  using const_reverse_iterator = std::reverse_iterator<const_iterator>;
private:
  [[no_unique_address]] allocator_type alloc;
  pointer ptr = nullptr;
  size_type len = 0, cap = 0;
public:
  constexpr Vec() noexcept(noexcept(Alloc())) {}
  constexpr explicit Vec(const allocator_type& a) noexcept : alloc(a) {}
  constexpr explicit Vec(size_type n, const Alloc& a = Alloc()) : alloc(a) {
    if(n == 0) [[unlikely]]
      return;
    ptr = traits::allocate(alloc, n);
    len = n, cap = n;
    for(size_type i = 0; i != n; ++i) traits::construct(alloc, ptr + i);
  }
  constexpr explicit Vec(const size_type n, const value_type& value, const allocator_type& a = Alloc()) : alloc(a) {
    if(n == 0) [[unlikely]]
      return;
    ptr = traits::allocate(alloc, n);
    len = n, cap = n;
    for(size_type i = 0; i != n; ++i) traits::construct(alloc, ptr + i, value);
  }
  template<std::forward_iterator Iter, std::sentinel_for<Iter> Sent> constexpr Vec(Iter first, Sent last, const allocator_type& a = Alloc()) : alloc(a) {
    const size_type n = std::ranges::distance(first, last);
    if(n == 0) [[unlikely]]
      return;
    ptr = traits::allocate(alloc, n);
    len = n, cap = n;
    size_type i = 0;
    for(; i != n; ++first, ++i) traits::construct(alloc, ptr + i, *first);
  }
  constexpr Vec(const Vec& x) : Vec(x, traits::select_on_container_copy_construction(x.alloc)) {}
  constexpr Vec(Vec&& x) noexcept : alloc(std::move(x.alloc)), ptr(x.ptr), len(x.len), cap(x.cap) { x.ptr = nullptr, x.len = 0, x.cap = 0; }
  constexpr Vec(const Vec& x, const allocator_type& a) : alloc(a), len(x.len), cap(x.len) {
    if(len == 0) [[unlikely]]
      return;
    ptr = traits::allocate(alloc, cap);
    for(size_type i = 0; i != len; ++i) traits::construct(alloc, ptr + i, *(x.ptr + i));
  }
  constexpr Vec(Vec&& x, const allocator_type& a) : alloc(a) {
    if(traits::is_always_equal || x.get_allocator() == a) {
      ptr = x.ptr, len = x.len, cap = x.cap;
      x.ptr = nullptr, x.len = 0, x.cap = 0;
    } else {
      if(x.len == 0) [[unlikely]]
        return;
      len = x.len, cap = x.cap;
      ptr = traits::allocate(alloc, len);
      for(size_type i = 0; i != len; ++i) traits::construct(alloc, ptr + i, std::move(*(x.ptr + i)));
      traits::deallocate(x.alloc, x.ptr, x.cap);
      x.ptr = nullptr, x.len = 0, x.cap = 0;
    }
  }
  constexpr Vec(std::initializer_list<value_type> il, const allocator_type& a = Alloc()) : Vec(il.begin(), il.end(), a) {}
  constexpr ~Vec() {
    if(cap != 0) {
      for(size_type i = 0; i != len; ++i) traits::destroy(alloc, ptr + i);
      traits::deallocate(alloc, ptr, cap);
    }
  }
  constexpr Vec& operator=(const Vec& x) {
    if(&x == this) return *this;
    for(size_type i = 0; i != len; ++i) traits::destroy(alloc, ptr + i);
    if(traits::propagate_on_container_copy_assignment::value || cap < x.len) {
      if(cap != 0) traits::deallocate(alloc, ptr, cap);
      if constexpr(traits::propagate_on_container_copy_assignment::value) alloc = x.alloc;
      cap = x.len;
      ptr = traits::allocate(alloc, cap);
    }
    len = x.len;
    for(size_type i = 0; i != len; ++i) *(ptr + i) = *(x.ptr + i);
    return *this;
  }
  constexpr Vec& operator=(Vec&& x) noexcept(traits::propagate_on_container_move_assignment::value || traits::is_always_equal::value) {
    if(&x == this) return *this;
    if(cap != 0) {
      for(size_type i = 0; i != len; ++i) traits::destroy(alloc, ptr + i);
      traits::deallocate(alloc, ptr, cap);
    }
    if constexpr(traits::propagate_on_container_move_assignment::value) alloc = std::move(x.alloc);
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
    if(cap < sz) {
      const pointer new_ptr = traits::allocate(alloc, sz);
      if(cap != 0) {
        for(size_type i = 0; i != len; ++i) traits::construct(alloc, new_ptr + i, std::move(*(ptr + i)));
        for(size_type i = 0; i != len; ++i) traits::destroy(alloc, ptr + i);
        traits::deallocate(alloc, ptr, cap);
      }
      ptr = new_ptr;
      for(size_type i = len; i != sz; ++i) traits::construct(alloc, ptr + i);
      len = sz, cap = sz;
    } else if(len < sz) {
      for(size_type i = len; i != sz; ++i) traits::construct(alloc, ptr + i);
      len = sz;
    } else {
      for(size_type i = sz; i != len; ++i) traits::destroy(alloc, ptr + i);
      len = sz;
    }
  }
  constexpr void resize(const size_type sz, const value_type& c) {
    if(cap < sz) {
      const pointer new_ptr = traits::allocate(sz);
      if(cap != 0) {
        for(size_type i = 0; i != len; ++i) traits::construct(alloc, new_ptr + i, std::move(*(ptr + i)));
        for(size_type i = 0; i != len; ++i) traits::destroy(alloc, ptr + i);
        traits::deallocate(alloc, ptr, cap);
      }
      ptr = new_ptr;
      for(size_type i = len; i != sz; ++i) traits::construct(alloc, *(ptr + i), c);
      len = sz, cap = sz;
    } else if(len < sz) {
      for(size_type i = len; i != sz; ++i) traits::construct(alloc, *(ptr + i), c);
      len = sz;
    } else {
      for(size_type i = sz; i != len; ++i) traits::destroy(alloc, ptr + i);
      len = sz;
    }
  }
  constexpr size_type capacity() const noexcept { return cap; }
  [[nodiscard]] constexpr bool empty() const noexcept { return len == 0; }
  constexpr void reserve(const size_type n) {
    if(n > cap) {
      const pointer new_ptr = traits::allocate(alloc, n);
      if(cap != 0) {
        for(size_type i = 0; i != len; ++i) traits::construct(alloc, new_ptr + i, std::move(*(ptr + i)));
        for(size_type i = 0; i != len; ++i) traits::destroy(alloc, ptr + i);
        traits::deallocate(alloc, ptr, cap);
      }
      ptr = new_ptr, cap = n;
    }
  }
  constexpr void shrink_to_fit() {
    if(len == 0) {
      if(cap != 0) traits::deallocate(alloc, ptr, cap);
      ptr = nullptr, cap = 0;
      return;
    }
    if(len != cap) {
      const pointer new_ptr = traits::allocate(alloc, len);
      for(size_type i = 0; i != len; ++i) traits::construct(alloc, new_ptr + i, std::move(*(ptr + i)));
      for(size_type i = 0; i != len; ++i) traits::destroy(alloc, ptr + i);
      traits::deallocate(alloc, ptr, cap);
      ptr = new_ptr, cap = len;
    }
  }
  GSH_INTERNAL_INLINE constexpr reference operator[](const size_type n) {
#ifndef NDEBUG
    if(n >= len) [[unlikely]]
      throw gsh::Exception("gsh::Vec::operator[] / The index is out of range. ( n=", n, ", size=", len, " )");
#endif
    Assume(n < len);
    return *(ptr + n);
  }
  GSH_INTERNAL_INLINE constexpr const_reference operator[](const size_type n) const {
#ifndef NDEBUG
    if(n >= len) [[unlikely]]
      throw gsh::Exception("gsh::Vec::operator[] / The index is out of range. ( n=", n, ", size=", len, " )");
#endif
    Assume(n < len);
    return *(ptr + n);
  }
  GSH_INTERNAL_INLINE constexpr reference at(const size_type n) {
    if(n >= len) [[unlikely]]
      throw gsh::Exception("gsh::Vec::at / The index is out of range. ( n=", n, ", size=", len, " )");
    return *(ptr + n);
  }
  GSH_INTERNAL_INLINE constexpr const_reference at(const size_type n) const {
    if(n >= len) [[unlikely]]
      throw gsh::Exception("gsh::Vec::at / The index is out of range. ( n=", n, ", size=", len, " )");
    return *(ptr + n);
  }
  GSH_INTERNAL_INLINE constexpr reference at_unchecked(const size_type n) noexcept {
    Assume(n < len);
    return *(ptr + n);
  }
  GSH_INTERNAL_INLINE constexpr const_reference at_unchecked(const size_type n) const noexcept {
    Assume(n < len);
    return *(ptr + n);
  }
  constexpr pointer data() noexcept { return ptr; }
  constexpr const_pointer data() const noexcept { return ptr; }
  constexpr reference front() noexcept { return *ptr; }
  constexpr const_reference front() const noexcept { return *ptr; }
  constexpr reference back() noexcept { return *(ptr + len - 1); }
  constexpr const_reference back() const noexcept { return *(ptr + len - 1); }
  template<std::forward_iterator Iter, std::sentinel_for<Iter> Sent> constexpr void assign(Iter first, Sent last) {
    const size_type n = std::ranges::distance(first, last);
    if(n > cap) {
      for(size_type i = 0; i != len; ++i) traits::destroy(alloc, ptr + i);
      traits::deallocate(alloc, ptr, cap);
      ptr = traits::allocate(alloc, n);
      cap = n;
      for(size_type i = 0; i != n; ++first, ++i) traits::construct(alloc, ptr + i, *first);
    } else if(n > len) {
      size_type i = 0;
      for(; i != len; ++first, ++i) *(ptr + i) = *first;
      for(; i != n; ++first, ++i) traits::construct(alloc, ptr + i, *first);
    } else {
      for(size_type i = n; i != len; ++i) traits::destroy(alloc, ptr + i);
      for(size_type i = 0; i != n; ++first, ++i) *(ptr + i) = *first;
    }
    len = n;
  }
  constexpr void assign(const size_type n, const value_type& t) {
    if(n > cap) {
      for(size_type i = 0; i != len; ++i) traits::destroy(alloc, ptr + i);
      traits::deallocate(alloc, ptr, cap);
      ptr = traits::allocate(alloc, n);
      cap = n;
      for(size_type i = 0; i != n; ++i) traits::construct(alloc, ptr + i, t);
    } else if(n > len) {
      size_type i = 0;
      for(; i != len; ++i) *(ptr + i) = t;
      for(; i != n; ++i) traits::construct(alloc, ptr + i, t);
    } else {
      for(size_type i = n; i != len; ++i) traits::destroy(alloc, ptr + i);
      for(size_type i = 0; i != n; ++i) *(ptr + i) = t;
    }
    len = n;
  }
  constexpr void assign(std::initializer_list<value_type> il) { assign(il.begin(), il.end()); }
private:
  constexpr void extend_one() {
    if(len == cap) {
      const pointer new_ptr = traits::allocate(alloc, cap * 2 + 8);
      if(cap != 0) {
        for(size_type i = 0; i != len; ++i) traits::construct(alloc, new_ptr + i, std::move_if_noexcept(*(ptr + i)));
        for(size_type i = 0; i != len; ++i) traits::destroy(alloc, ptr + i);
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
    if(len == 0) [[unlikely]]
      throw gsh::Exception("gsh::Vec::pop_back / The container is empty.");
#endif
    traits::destroy(alloc, ptr + (--len));
  }
  /*
  constexpr iterator insert(const const_iterator position, const value_type& x);
  constexpr iterator insert(const const_iterator position, value_type&& x);
  constexpr iterator insert(const const_iterator position, const size_type n, const value_type& x);
  template<class InputIter> constexpr iterator insert(const const_iterator position, const InputIter first, const InputIter last);
  constexpr iterator insert(const const_iterator position, const std::initializer_list<value_type> il);
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
    for(size_type i = 0; i != len; ++i) traits::destroy(alloc, ptr + i);
    len = 0;
  }
  constexpr void reset() {
    if(cap != 0) {
      traits::deallocate(alloc, ptr, cap);
      ptr = nullptr, len = 0, cap = 0;
    }
  }
  constexpr void abandon() noexcept { ptr = nullptr, len = 0, cap = 0; }
  constexpr allocator_type get_allocator() const noexcept { return alloc; }
  friend constexpr bool operator==(const Vec& x, const Vec& y) {
    if(x.len != y.len) return false;
    bool res = true;
    for(size_type i = 0; i != x.len;) {
      const bool f = *(x.ptr + i) == *(y.ptr + i);
      res &= f;
      i = f ? i + 1 : x.len;
    }
    return res;
  }
  friend constexpr auto operator<=>(const Vec& x, const Vec& y) { return std::lexicographical_compare_three_way(x.begin(), x.end(), y.begin(), y.end()); }
  friend constexpr void swap(Vec& x, Vec& y) noexcept(noexcept(x.swap(y))) { x.swap(y); }
};
template<std::input_iterator InputIter, class Alloc = std::allocator<typename std::iterator_traits<InputIter>::value_type>> Vec(InputIter, InputIter, Alloc = Alloc()) -> Vec<typename std::iterator_traits<InputIter>::value_type, Alloc>;
template<class T, class Alloc = std::allocator<T>> using Vec2 = Vec<Vec<T, Alloc>, typename std::allocator_traits<Alloc>::template rebind_alloc<Vec<T, Alloc>>>;
template<class T, class Alloc = std::allocator<T>> using Vec3 = Vec<Vec<Vec<T, Alloc>, typename std::allocator_traits<Alloc>::template rebind_alloc<Vec<T, Alloc>>>, typename std::allocator_traits<Alloc>::template rebind_alloc<Vec<Vec<T, Alloc>, typename std::allocator_traits<Alloc>::template rebind_alloc<Vec<T, Alloc>>>>>;
} // namespace gsh
