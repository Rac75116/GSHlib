#pragma once
#include "Exception.hpp"
#include "TypeDef.hpp"
#include "Vec.hpp"
#include <bit>
#include <initializer_list>
#include <iterator>
#include <type_traits>
namespace gsh {
template<class T> class FenwickTree {
protected:
  Vec<T> bit;
public:
  using reference = T&;
  using const_reference = const T&;
  using size_type = u32;
  using difference_type = i32;
  using value_type = T;
  constexpr FenwickTree() = default;
  constexpr explicit FenwickTree(u32 n) : bit(n) {}
  constexpr FenwickTree(u32 n, const T& value) : bit() { assign(n, value); }
  template<class InputIter> constexpr FenwickTree(InputIter first, InputIter last) : bit() { assign(first, last); }
  constexpr FenwickTree(const FenwickTree&) = default;
  constexpr FenwickTree(FenwickTree&&) noexcept = default;
  constexpr FenwickTree(std::initializer_list<T> il) : FenwickTree(il.begin(), il.end()) {}
  constexpr FenwickTree& operator=(const FenwickTree&) = default;
  constexpr FenwickTree& operator=(FenwickTree&&) noexcept = default;
  constexpr FenwickTree& operator=(std::initializer_list<T> il) {
    assign(il);
    return *this;
  }
  constexpr u32 size() const noexcept { return bit.size(); }
  constexpr void resize(u32 sz) { resize(sz, value_type{}); }
  constexpr void resize(u32 sz, const value_type& c) {
    u32 n = bit.size();
    bit.resize(sz);
    if(n >= sz) return;
    if(n == 0) {
      for(u32 i = 0; i < sz; ++i) bit[i] = c * (static_cast<u32>(i + 1) & -(static_cast<i32>(i + 1)));
    } else {
      for(u32 i = n; i < sz; ++i) {
        for(u32 j = i + 1; j <= sz; j += (j & -j)) bit[j - 1] += c;
      }
    }
  }
  constexpr bool empty() const noexcept { return bit.empty(); }
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
    bit = Vec<value_type>(n);
    Vec<value_type> mul(std::bit_width(n));
    mul[0] = u;
    for(u32 i = 1, sz = mul.size(); i < sz; ++i) mul[i] = mul[i - 1], mul[i] += mul[i - 1];
    for(u32 i = 0; i != n; ++i) bit[i] = mul[std::countr_zero(i + 1)];
  }
  constexpr void assign(std::initializer_list<T> il) { assign(il.begin(), il.end()); }
  constexpr void swap(FenwickTree& x) noexcept { bit.swap(x.bit); };
  constexpr void clear() { bit.clear(); }
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
template<class U> constexpr void swap(FenwickTree<U>& x, FenwickTree<U>& y) noexcept(noexcept(x.swap(y))) { x.swap(y); }
template<class InputIterator> FenwickTree(InputIterator, InputIterator) -> FenwickTree<typename std::iterator_traits<InputIterator>::value_type>;
}
