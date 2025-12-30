#pragma once
#include "Arr.hpp"
#include "TypeDef.hpp"
#include "internal/UtilMacro.hpp"
#include <bit>
#include <initializer_list>
#include <iterator>
#include <type_traits>
namespace gsh {
template<class T, class Alloc = std::allocator<T>> class RangeSumQuery {
protected:
  Arr<T, Alloc> bit;
public:
  using reference = T&;
  using const_reference = const T&;
  using size_type = u32;
  using difference_type = i32;
  using value_type = T;
  using allocator_type = Alloc;
  using pointer = typename std::allocator_traits<Alloc>::pointer;
  using const_pointer = typename std::allocator_traits<Alloc>::const_pointer;
  constexpr RangeSumQuery() noexcept(noexcept(Alloc())) : RangeSumQuery(Alloc()) {}
  constexpr explicit RangeSumQuery(const Alloc& alloc) noexcept : bit(alloc) {}
  constexpr explicit RangeSumQuery(u32 n, const Alloc& alloc = Alloc()) : bit(n, alloc) {}
  constexpr RangeSumQuery(u32 n, const T& value, const Alloc& alloc = Alloc()) : bit(alloc) { assign(n, value); }
  template<class InputIter> constexpr RangeSumQuery(InputIter first, InputIter last, const Alloc& alloc = Alloc()) : bit(alloc) { assign(first, last); }
  constexpr RangeSumQuery(const RangeSumQuery&) = default;
  constexpr RangeSumQuery(RangeSumQuery&&) noexcept = default;
  constexpr RangeSumQuery(const RangeSumQuery& x, const Alloc& alloc) : bit(x.bit, alloc) {}
  constexpr RangeSumQuery(RangeSumQuery&& x, const Alloc& alloc) : bit(std::move(x.bit), alloc) {}
  constexpr RangeSumQuery(std::initializer_list<T> il, const Alloc& alloc = Alloc()) : RangeSumQuery(il.begin(), il.end(), alloc) {}
  constexpr RangeSumQuery& operator=(const RangeSumQuery&) = default;
  constexpr RangeSumQuery& operator=(RangeSumQuery&&) noexcept(std::allocator_traits<Alloc>::propagate_on_container_move_assignment::value || std::allocator_traits<Alloc>::is_always_equal::value) = default;
  constexpr RangeSumQuery& operator=(std::initializer_list<T> il) {
    assign(il);
    return *this;
  }
  constexpr u32 size() const noexcept { return bit.size(); }
  constexpr void resize(u32 sz) { resize(sz, value_type{}); }
  /*
    constexpr void resize(u32 sz, const value_type& c) {
        u32 n = bit.size();
        bit.resize(sz);
        if (n >= sz) return;
        // TODO
    }
    */
  [[nodiscard]] constexpr bool empty() const noexcept { return bit.empty(); }
  constexpr value_type operator[](u32 n) const {
    value_type res = bit[n];
    if(!(n & 1)) return res;
    u32 tmp = n & (n + 1);
    for(u32 i = n; i != tmp; i &= i - 1) res -= bit[i - 1];
    return res;
  }
  constexpr value_type at(u32 n) const {
    if(n >= size()) throw Exception("gsh::RangeSumQuery::at / Index is out of range.");
    return operator[](n);
  }
  template<class InputIterator> constexpr void assign(InputIterator first, InputIterator last) {
    bit.assign(first, last);
    u32 n = bit.size();
    if(n == 0) return;
    const auto tmp = bit[0];
    for(u32 i = 0; i != n - 1; ++i) {
      const u32 j = i + ((i + 1) & -(i + 1));
      bit[j < n ? j : 0] += bit[i];
    }
    bit[0] = tmp;
  }
  constexpr void assign(u32 n, const T& u) {
    if(n == 0) return;
    bit = Arr<value_type, Alloc>(n, get_allocator());
    Arr<value_type, Alloc> mul(std::bit_width(n), get_allocator());
    mul[0] = u;
    for(u32 i = 1, sz = mul.size(); i < sz; ++i) mul[i] = mul[i - 1], mul[i] += mul[i - 1];
    for(u32 i = 0; i != n; ++i) bit[i] = mul[std::countr_zero(i + 1)];
  }
  constexpr void assign(std::initializer_list<T> il) { assign(il.begin(), il.end()); }
  constexpr void swap(RangeSumQuery& x) noexcept(std::allocator_traits<Alloc>::propagate_on_container_swap::value || std::allocator_traits<Alloc>::is_always_equal::value) { bit.swap(x.bit); };
  constexpr void clear() { bit.clear(); }
  constexpr allocator_type get_allocator() const noexcept { return bit.get_allocator(); }
  constexpr void add(u32 n, const value_type& x) {
    for(u32 i = n + 1, sz = size(); i <= sz; i += (i & -i)) bit[i - 1] += x;
  }
  constexpr void sub(u32 n, const value_type& x) {
    for(u32 i = n + 1, sz = size(); i <= sz; i += (i & -i)) bit[i - 1] -= x;
  }
  constexpr void inc(u32 n) {
    for(u32 i = n + 1, sz = size(); i <= sz; i += (i & (-i))) ++bit[i - 1];
  }
  constexpr void dec(u32 n) {
    for(u32 i = n + 1, sz = size(); i <= sz; i += (i & (-i))) --bit[i - 1];
  }
  constexpr value_type sum(u32 n) const {
    value_type res = {};
    for(u32 i = n; i != 0; i &= i - 1) res += bit[i - 1];
    return res;
  }
  constexpr value_type sum(u32 l, u32 r) const {
    u32 n = l & ~((std::bit_floor(l ^ r) << 1) - 1);
    value_type res1 = {}, res2 = {};
    for(u32 i = r; i != n; i &= i - 1) res1 += bit[i - 1];
    for(u32 i = l; i != n; i &= i - 1) res2 += bit[i - 1];
    return res1 - res2;
  }
  constexpr u32 lower_bound(value_type x) const {
    static_assert(std::is_unsigned_v<value_type>, "gsh::RangeSumQuery::lower_bound / value_type must be unsigned.");
    u32 res = 0, n = size();
    for(u32 len = std::bit_floor(n); len != 0; len >>= 1) {
      if(res + len <= n && bit[res + len - 1] < x) {
        x -= bit[res + len - 1];
        res += len;
      }
    }
    return res;
  }
  constexpr u32 upper_bound(value_type x) const {
    static_assert(std::is_unsigned_v<value_type>, "gsh::RangeSumQuery::upper_bound / value_type must be unsigned.");
    u32 res = 0, n = size();
    for(u32 len = std::bit_floor(n); len != 0; len >>= 1) {
      if(res + len <= n && !(x < bit[res + len - 1])) {
        x -= bit[res + len - 1];
        res += len;
      }
    }
    return res;
  }
};
template<class U, class Alloc> constexpr void swap(RangeSumQuery<U, Alloc>& x, RangeSumQuery<U, Alloc>& y) noexcept(noexcept(x.swap(y))) { x.swap(y); }
template<class InputIterator, class Alloc = std::allocator<typename std::iterator_traits<InputIterator>::value_type>> RangeSumQuery(InputIterator, InputIterator, Alloc = Alloc()) -> RangeSumQuery<typename std::iterator_traits<InputIterator>::value_type, Alloc>;
} // namespace gsh
