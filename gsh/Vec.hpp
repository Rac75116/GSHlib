#pragma once
#include "Exception.hpp"
#include "Range.hpp"
#include "TypeDef.hpp"
#include <initializer_list>
#include <iterator>
#include <memory>
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
  u32 len = 0, cap = 0;
public:
  constexpr Vec() noexcept(noexcept(Alloc())) {}
  constexpr explicit Vec(const allocator_type& a) noexcept : alloc(a) {}
  constexpr explicit Vec(u32 n, const Alloc& a = Alloc()) : alloc(a) {
    if(n == 0) [[unlikely]]
      return;
    ptr = traits::allocate(alloc, n);
    len = n, cap = n;
    for(u32 i = 0; i != n; ++i) traits::construct(alloc, ptr + i);
  }
  constexpr explicit Vec(const u32 n, const T& value, const allocator_type& a = Alloc()) : alloc(a) {
    if(n == 0) [[unlikely]]
      return;
    ptr = traits::allocate(alloc, n);
    len = n, cap = n;
    for(u32 i = 0; i != n; ++i) traits::construct(alloc, ptr + i, value);
  }
  template<std::forward_iterator Iter, std::sentinel_for<Iter> Sent> constexpr Vec(Iter first, Sent last, const allocator_type& a = Alloc()) : alloc(a) {
    const u32 n = std::ranges::distance(first, last);
    if(n == 0) [[unlikely]]
      return;
    ptr = traits::allocate(alloc, n);
    len = n, cap = n;
    u32 i = 0;
    for(; i != n; ++first, ++i) traits::construct(alloc, ptr + i, *first);
  }
  constexpr Vec(const Vec& x) : Vec(x, traits::select_on_container_copy_construction(x.alloc)) {}
  constexpr Vec(Vec&& x) noexcept : alloc(std::move(x.alloc)), ptr(x.ptr), len(x.len), cap(x.cap) { x.ptr = nullptr, x.len = 0, x.cap = 0; }
  constexpr Vec(const Vec& x, const allocator_type& a) : alloc(a), len(x.len), cap(x.len) {
    if(len == 0) [[unlikely]]
      return;
    ptr = traits::allocate(alloc, cap);
    for(u32 i = 0; i != len; ++i) traits::construct(alloc, ptr + i, *(x.ptr + i));
  }
  constexpr Vec(Vec&& x, const allocator_type& a) : alloc(a) {
    if(traits::is_always_equal || x.get_allocator() == a) {
      ptr = x.ptr, len = x.len, cap = x.cap;
      x.ptr = nullptr, x.len = 0, x.cap = 0;
    } else {
      if(x.len == 0) [[unlikely]]
        return;
      len = x.len, cap = x.len;
      ptr = traits::allocate(alloc, cap);
      for(u32 i = 0; i != len; ++i) traits::construct(alloc, ptr + i, std::move(*(x.ptr + i)));
      traits::deallocate(x.alloc, x.ptr, x.cap);
      x.ptr = nullptr, x.len = 0, x.cap = 0;
    }
  }
  constexpr Vec(std::initializer_list<T> il, const allocator_type& a = Alloc()) : Vec(il.begin(), il.end(), a) {}
  constexpr ~Vec() {
    if(cap != 0) {
      for(u32 i = 0; i != len; ++i) traits::destroy(alloc, ptr + i);
      traits::deallocate(alloc, ptr, cap);
    }
  }
  constexpr Vec& operator=(const Vec& x) {
    if(x.ptr == ptr) return *this;
    for(u32 i = 0; i != len; ++i) traits::destroy(alloc, ptr + i);
    if(traits::propagate_on_container_copy_assignment::value || cap < x.len) {
      if(cap != 0) traits::deallocate(alloc, ptr, cap);
      if constexpr(traits::propagate_on_container_copy_assignment::value) alloc = x.alloc;
      cap = x.len;
      ptr = traits::allocate(alloc, cap);
    }
    len = x.len;
    for(u32 i = 0; i != len; ++i) *(ptr + i) = *(x.ptr + i);
    return *this;
  }
  constexpr Vec& operator=(Vec&& x) noexcept(traits::propagate_on_container_move_assignment::value || traits::is_always_equal::value) {
    if(x.ptr == ptr) return *this;
    if(cap != 0) {
      for(u32 i = 0; i != len; ++i) traits::destroy(alloc, ptr + i);
      traits::deallocate(alloc, ptr, cap);
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
  constexpr u32 size() const noexcept { return len; }
  constexpr u32 max_size() const noexcept {
    const auto tmp = traits::max_size(alloc);
    return tmp < 2147483647 ? tmp : 2147483647;
  }
  constexpr void resize(const u32 sz) {
    if(cap < sz) {
      T* new_ptr = traits::allocate(alloc, sz);
      if(cap != 0) {
        for(u32 i = 0; i != len; ++i) traits::construct(alloc, new_ptr + i, std::move(*(ptr + i)));
        for(u32 i = 0; i != len; ++i) traits::destroy(alloc, ptr + i);
        traits::deallocate(alloc, ptr, cap);
      }
      ptr = new_ptr;
      for(u32 i = len; i != sz; ++i) traits::construct(alloc, ptr + i);
      len = sz, cap = sz;
    } else if(len < sz) {
      for(u32 i = len; i != sz; ++i) traits::construct(alloc, ptr + i);
      len = sz;
    } else {
      for(u32 i = sz; i != len; ++i) traits::destroy(alloc, ptr + i);
      len = sz;
    }
  }
  constexpr void resize(const u32 sz, const T& c) {
    if(cap < sz) {
      T* new_ptr = traits::allocate(alloc, sz);
      for(u32 i = len; i != sz; ++i) traits::construct(alloc, new_ptr + i, c);
      if(cap != 0) {
        for(u32 i = 0; i != len; ++i) traits::construct(alloc, new_ptr + i, std::move(*(ptr + i)));
        for(u32 i = 0; i != len; ++i) traits::destroy(alloc, ptr + i);
        traits::deallocate(alloc, ptr, cap);
      }
      ptr = new_ptr;
      len = sz, cap = sz;
    } else if(len < sz) {
      for(u32 i = len; i != sz; ++i) traits::construct(alloc, ptr + i, c);
      len = sz;
    } else {
      for(u32 i = sz; i != len; ++i) traits::destroy(alloc, ptr + i);
      len = sz;
    }
  }
  constexpr u32 capacity() const noexcept { return cap; }
  constexpr bool empty() const noexcept { return len == 0; }
  constexpr void reserve(const u32 n) {
    if(n > cap) {
      T* new_ptr = traits::allocate(alloc, n);
      if(cap != 0) {
        for(u32 i = 0; i != len; ++i) traits::construct(alloc, new_ptr + i, std::move(*(ptr + i)));
        for(u32 i = 0; i != len; ++i) traits::destroy(alloc, ptr + i);
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
      T* new_ptr = traits::allocate(alloc, len);
      for(u32 i = 0; i != len; ++i) traits::construct(alloc, new_ptr + i, std::move(*(ptr + i)));
      for(u32 i = 0; i != len; ++i) traits::destroy(alloc, ptr + i);
      traits::deallocate(alloc, ptr, cap);
      ptr = new_ptr, cap = len;
    }
  }
  constexpr T* data() noexcept { return ptr; }
  constexpr const T* data() const noexcept { return ptr; }
  template<std::forward_iterator Iter, std::sentinel_for<Iter> Sent> constexpr void assign(Iter first, Sent last) {
    const u32 n = std::ranges::distance(first, last);
    if(n > cap) {
      T* new_ptr = traits::allocate(alloc, n);
      for(u32 i = 0; i != n; ++first, ++i) traits::construct(alloc, new_ptr + i, *first);
      if(cap != 0) {
        for(u32 i = 0; i != len; ++i) traits::destroy(alloc, ptr + i);
        traits::deallocate(alloc, ptr, cap);
      }
      ptr = new_ptr, cap = n;
    } else if(n > len) {
      u32 i = 0;
      for(; i != len; ++first, ++i) *(ptr + i) = *first;
      for(; i != n; ++first, ++i) traits::construct(alloc, ptr + i, *first);
    } else {
      for(u32 i = n; i != len; ++i) traits::destroy(alloc, ptr + i);
      for(u32 i = 0; i != n; ++first, ++i) *(ptr + i) = *first;
    }
    len = n;
  }
  constexpr void assign(const u32 n, const T& t) {
    if(n > cap) {
      T* new_ptr = traits::allocate(alloc, n);
      for(u32 i = 0; i != n; ++i) traits::construct(alloc, new_ptr + i, t);
      if(cap != 0) {
        for(u32 i = 0; i != len; ++i) traits::destroy(alloc, ptr + i);
        traits::deallocate(alloc, ptr, cap);
      }
      ptr = new_ptr;
      cap = n;
    } else if(n > len) {
      u32 i = 0;
      for(; i != len; ++i) *(ptr + i) = t;
      for(; i != n; ++i) traits::construct(alloc, ptr + i, t);
    } else {
      for(u32 i = n; i != len; ++i) traits::destroy(alloc, ptr + i);
      for(u32 i = 0; i != n; ++i) *(ptr + i) = t;
    }
    len = n;
  }
  constexpr void assign(std::initializer_list<T> il) { assign(il.begin(), il.end()); }
#ifdef NDEBUG
  GSH_INTERNAL_INLINE
#endif
  constexpr T& operator[](u32 n) {
#ifndef NDEBUG
    if(n >= len) [[unlikely]]
      throw Exception("gsh::Vec::operator[] / The index is out of range. ( n=", n, ", size=", len, " )");
#endif
    return ptr[n];
  };
#ifdef NDEBUG
  GSH_INTERNAL_INLINE
#endif
  constexpr const T& operator[](u32 n) const {
#ifndef NDEBUG
    if(n >= len) [[unlikely]]
      throw Exception("gsh::Vec::operator[] / The index is out of range. ( n=", n, ", size=", len, " )");
#endif
    return ptr[n];
  }
public:
  constexpr void push_back(const T& x) { emplace_back(x); }
  constexpr void push_back(T&& x) { emplace_back(std::move(x)); }
  template<class... Args> constexpr T& emplace_back(Args&&... args) {
    if(len == cap) {
      u32 new_cap = cap * 2 + 8;
      T* new_ptr = traits::allocate(alloc, new_cap);
      for(u32 i = 0; i != len; ++i) traits::construct(alloc, new_ptr + i, std::move_if_noexcept(*(ptr + i)));
      traits::construct(alloc, new_ptr + len, std::forward<Args>(args)...);
      for(u32 i = 0; i != len; ++i) traits::destroy(alloc, ptr + i);
      if(cap != 0) traits::deallocate(alloc, ptr, cap);
      ptr = new_ptr, cap = new_cap;
    } else traits::construct(alloc, ptr + len, std::forward<Args>(args)...);
    return *(ptr + (len++));
  }
  constexpr void pop_back() {
#ifndef NDEBUG
    if(len == 0) [[unlikely]]
      throw gsh::Exception("gsh::Vec::pop_back / The container is empty.");
#endif
    traits::destroy(alloc, ptr + (--len));
  }
  constexpr iterator insert(const const_iterator position, const T& x) { return emplace(position, x); }
  constexpr iterator insert(const const_iterator position, T&& x) { return emplace(position, std::move(x)); }
  constexpr iterator insert(const const_iterator position, const u32 n, const T& x) {
    if(n == 0) return (len == 0 ? ptr : const_cast<iterator>(position));
    const u32 idx = (len == 0 ? 0u : static_cast<u32>(position - ptr));
    if(len + n > cap) {
      u32 new_cap = cap;
      while(new_cap < len + n) new_cap = new_cap * 2 + 8;
      T* new_ptr = traits::allocate(alloc, new_cap);
      for(u32 i = 0; i != idx; ++i) traits::construct(alloc, new_ptr + i, std::move_if_noexcept(*(ptr + i)));
      for(u32 i = 0; i != n; ++i) traits::construct(alloc, new_ptr + (idx + i), x);
      for(u32 i = idx; i != len; ++i) traits::construct(alloc, new_ptr + (i + n), std::move_if_noexcept(*(ptr + i)));
      for(u32 i = 0; i != len; ++i) traits::destroy(alloc, ptr + i);
      if(cap != 0) traits::deallocate(alloc, ptr, cap);
      ptr = new_ptr, cap = new_cap;
      len += n;
      return ptr + idx;
    }
    if(idx == len) {
      for(u32 i = 0; i != n; ++i) traits::construct(alloc, ptr + (len + i), x);
      len += n;
      return ptr + idx;
    }
    for(u32 i = len; i > idx; --i) {
      const u32 src = i - 1;
      const u32 dst = src + n;
      if(dst >= len) {
        traits::construct(alloc, ptr + dst, std::move_if_noexcept(*(ptr + src)));
      } else {
        *(ptr + dst) = std::move_if_noexcept(*(ptr + src));
      }
    }
    for(u32 i = 0; i != n; ++i) {
      traits::destroy(alloc, ptr + (idx + i));
      traits::construct(alloc, ptr + (idx + i), x);
    }
    len += n;
    return ptr + idx;
  }
  template<class InputIter> constexpr iterator insert(const const_iterator position, const InputIter first, const InputIter last) {
    Vec tmp(alloc);
    for(auto it = first; it != last; ++it) tmp.push_back(*it);
    const u32 n = tmp.size();
    if(n == 0) return (len == 0 ? ptr : const_cast<iterator>(position));
    const u32 idx = (len == 0 ? 0u : static_cast<u32>(position - ptr));
    if(len + n > cap) {
      u32 new_cap = cap;
      while(new_cap < len + n) new_cap = new_cap * 2 + 8;
      T* new_ptr = traits::allocate(alloc, new_cap);
      for(u32 i = 0; i != idx; ++i) traits::construct(alloc, new_ptr + i, std::move_if_noexcept(*(ptr + i)));
      for(u32 i = 0; i != n; ++i) traits::construct(alloc, new_ptr + (idx + i), tmp[i]);
      for(u32 i = idx; i != len; ++i) traits::construct(alloc, new_ptr + (i + n), std::move_if_noexcept(*(ptr + i)));
      for(u32 i = 0; i != len; ++i) traits::destroy(alloc, ptr + i);
      if(cap != 0) traits::deallocate(alloc, ptr, cap);
      ptr = new_ptr, cap = new_cap;
      len += n;
      return ptr + idx;
    }
    if(idx == len) {
      for(u32 i = 0; i != n; ++i) traits::construct(alloc, ptr + (len + i), tmp[i]);
      len += n;
      return ptr + idx;
    }
    for(u32 i = len; i > idx; --i) {
      const u32 src = i - 1;
      const u32 dst = src + n;
      if(dst >= len) {
        traits::construct(alloc, ptr + dst, std::move_if_noexcept(*(ptr + src)));
      } else {
        *(ptr + dst) = std::move_if_noexcept(*(ptr + src));
      }
    }
    for(u32 i = 0; i != n; ++i) {
      traits::destroy(alloc, ptr + (idx + i));
      traits::construct(alloc, ptr + (idx + i), tmp[i]);
    }
    len += n;
    return ptr + idx;
  }
  constexpr iterator insert(const const_iterator position, const std::initializer_list<T> il) { return insert(position, il.begin(), il.end()); }
  template<class... Args> constexpr iterator emplace(const_iterator position, Args&&... args) {
    const u32 idx = (len == 0 ? 0u : static_cast<u32>(position - ptr));
    if(len + 1 > cap) {
      u32 new_cap = cap;
      while(new_cap < len + 1) new_cap = new_cap * 2 + 8;
      T* new_ptr = traits::allocate(alloc, new_cap);
      for(u32 i = 0; i != idx; ++i) traits::construct(alloc, new_ptr + i, std::move_if_noexcept(*(ptr + i)));
      traits::construct(alloc, new_ptr + idx, std::forward<Args>(args)...);
      for(u32 i = idx; i != len; ++i) traits::construct(alloc, new_ptr + (i + 1), std::move_if_noexcept(*(ptr + i)));
      for(u32 i = 0; i != len; ++i) traits::destroy(alloc, ptr + i);
      if(cap != 0) traits::deallocate(alloc, ptr, cap);
      ptr = new_ptr, cap = new_cap;
      ++len;
      return ptr + idx;
    }
    if(idx == len) {
      traits::construct(alloc, ptr + len, std::forward<Args>(args)...);
      ++len;
      return ptr + idx;
    }
    for(u32 i = len; i > idx; --i) {
      const u32 src = i - 1;
      const u32 dst = src + 1;
      if(dst >= len) {
        traits::construct(alloc, ptr + dst, std::move_if_noexcept(*(ptr + src)));
      } else {
        *(ptr + dst) = std::move_if_noexcept(*(ptr + src));
      }
    }
    traits::destroy(alloc, ptr + idx);
    traits::construct(alloc, ptr + idx, std::forward<Args>(args)...);
    ++len;
    return ptr + idx;
  }
  constexpr iterator erase(const_iterator position) {
    const u32 idx = static_cast<u32>(position - ptr);
    for(u32 i = idx + 1; i != len; ++i) *(ptr + (i - 1)) = std::move_if_noexcept(*(ptr + i));
    traits::destroy(alloc, ptr + (len - 1));
    --len;
    return ptr + idx;
  }
  constexpr iterator erase(const_iterator first, const_iterator last) {
    if(first == last) return const_cast<iterator>(first);
    const u32 idx_f = static_cast<u32>(first - ptr);
    const u32 idx_l = static_cast<u32>(last - ptr);
    const u32 n = idx_l - idx_f;
    for(u32 i = idx_l; i != len; ++i) *(ptr + (i - n)) = std::move_if_noexcept(*(ptr + i));
    for(u32 i = len - n; i != len; ++i) traits::destroy(alloc, ptr + i);
    len -= n;
    return ptr + idx_f;
  }
  constexpr void swap(Vec& x) noexcept(traits::propagate_on_container_swap::value || traits::is_always_equal::value) {
    using std::swap;
    swap(ptr, x.ptr);
    swap(len, x.len);
    swap(cap, x.cap);
    if constexpr(traits::propagate_on_container_swap::value) swap(alloc, x.alloc);
  }
  constexpr void clear() {
    for(u32 i = 0; i != len; ++i) traits::destroy(alloc, ptr + i);
    len = 0;
  }
  constexpr void reset() {
    if(cap != 0) {
      traits::deallocate(alloc, ptr, cap);
      ptr = nullptr, len = 0, cap = 0;
    }
  }
  constexpr allocator_type get_allocator() const noexcept { return alloc; }
  friend constexpr void swap(Vec& x, Vec& y) noexcept(noexcept(x.swap(y))) { x.swap(y); }
};
template<std::input_iterator InputIter, class Alloc = std::allocator<typename std::iterator_traits<InputIter>::value_type>> Vec(InputIter, InputIter, Alloc = Alloc()) -> Vec<typename std::iterator_traits<InputIter>::value_type, Alloc>;
template<class T, class Alloc = std::allocator<T>> using Vec2 = Vec<Vec<T, Alloc>, typename std::allocator_traits<Alloc>::template rebind_alloc<Vec<T, Alloc>>>;
template<class T, class Alloc = std::allocator<T>> using Vec3 = Vec<Vec<Vec<T, Alloc>, typename std::allocator_traits<Alloc>::template rebind_alloc<Vec<T, Alloc>>>, typename std::allocator_traits<Alloc>::template rebind_alloc<Vec<Vec<T, Alloc>, typename std::allocator_traits<Alloc>::template rebind_alloc<Vec<T, Alloc>>>>>;
}
