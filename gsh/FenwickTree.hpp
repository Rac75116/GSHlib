#pragma once
#include "TypeDef.hpp"
#include "Vec.hpp"
#include "internal/UtilMacro.hpp"
#include <bit>
#include <initializer_list>
#include <iterator>
#include <type_traits>
namespace gsh {
template<class T, class Alloc = std::allocator<T>> class RangeSumQuery {
protected:
  Vec<T, Alloc> bit;
public:
  using reference = T&;
  using const_reference = const T&;
  using size_type = i64;
  using difference_type = i64;
  using value_type = T;
  using allocator_type = Alloc;
  using pointer = typename std::allocator_traits<Alloc>::pointer;
  using const_pointer = typename std::allocator_traits<Alloc>::const_pointer;
  constexpr RangeSumQuery() noexcept(noexcept(Alloc())) : RangeSumQuery(Alloc()) {}
  constexpr explicit RangeSumQuery(const Alloc& alloc) noexcept : bit(alloc) {}
  constexpr explicit RangeSumQuery(i64 n, const Alloc& alloc = Alloc()) : bit(n, alloc) {}
  constexpr RangeSumQuery(i64 n, const T& value, const Alloc& alloc = Alloc()) : bit(alloc) { assign(n, value); }
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
  constexpr i64 size() const noexcept { return bit.size(); }
  constexpr void resize(i64 sz) { resize(sz, value_type{}); }
  /*
    constexpr void resize(i64 sz, const value_type& c) {
      i64 n = bit.size();
        bit.resize(sz);
        if (n >= sz) return;
        // TODO
    }
    */
  [[nodiscard]] constexpr bool empty() const noexcept { return bit.empty(); }
  constexpr value_type operator[](i64 n) const {
    value_type res = bit[n];
    if(!(n & 1)) return res;
    i64 tmp = n & (n + 1);
    for(i64 i = n; i != tmp; i &= i - 1) res -= bit[i - 1];
    return res;
  }
  constexpr value_type at(i64 n) const {
    if(n >= size()) throw Exception("gsh::RangeSumQuery::at / Index is out of range.");
    return operator[](n);
  }
  template<class InputIterator> constexpr void assign(InputIterator first, InputIterator last) {
    bit.assign(first, last);
    i64 n = bit.size();
    if(n == 0) return;
    const auto tmp = bit[0];
    for(i64 i = 0; i != n - 1; ++i) {
      const i64 j = i + ((i + 1) & -(i + 1));
      bit[j < n ? j : 0] += bit[i];
    }
    bit[0] = tmp;
  }
  constexpr void assign(i64 n, const T& u) {
    if(n == 0) return;
    bit = Vec<value_type, Alloc>(n, get_allocator());
    Vec<value_type, Alloc> mul(static_cast<i64>(std::bit_width(static_cast<u64>(n))), get_allocator());
    mul[0] = u;
    for(i64 i = 1, sz = mul.size(); i < sz; ++i) mul[i] = mul[i - 1], mul[i] += mul[i - 1];
    for(i64 i = 0; i != n; ++i) bit[i] = mul[static_cast<i64>(std::countr_zero(static_cast<u64>(i + 1)))];
  }
  constexpr void assign(std::initializer_list<T> il) { assign(il.begin(), il.end()); }
  constexpr void swap(RangeSumQuery& x) noexcept(std::allocator_traits<Alloc>::propagate_on_container_swap::value || std::allocator_traits<Alloc>::is_always_equal::value) { bit.swap(x.bit); };
  constexpr void clear() { bit.clear(); }
  constexpr allocator_type get_allocator() const noexcept { return bit.get_allocator(); }
  constexpr void add(i64 n, const value_type& x) {
    for(i64 i = n + 1, sz = size(); i <= sz; i += (i & -i)) bit[i - 1] += x;
  }
  constexpr void sub(i64 n, const value_type& x) {
    for(i64 i = n + 1, sz = size(); i <= sz; i += (i & -i)) bit[i - 1] -= x;
  }
  constexpr void inc(i64 n) {
    for(i64 i = n + 1, sz = size(); i <= sz; i += (i & (-i))) ++bit[i - 1];
  }
  constexpr void dec(i64 n) {
    for(i64 i = n + 1, sz = size(); i <= sz; i += (i & (-i))) --bit[i - 1];
  }
  constexpr value_type sum(i64 n) const {
    value_type res = {};
    for(i64 i = n; i != 0; i &= i - 1) res += bit[i - 1];
    return res;
  }
  constexpr value_type sum(i64 l, i64 r) const {
    const u64 mask = (std::bit_floor(static_cast<u64>(l) ^ static_cast<u64>(r)) << 1) - 1;
    const i64 n = l & ~static_cast<i64>(mask);
    value_type res1 = {}, res2 = {};
    for(i64 i = r; i != n; i &= i - 1) res1 += bit[i - 1];
    for(i64 i = l; i != n; i &= i - 1) res2 += bit[i - 1];
    return res1 - res2;
  }
  constexpr i64 lower_bound(value_type x) const {
    static_assert(std::is_unsigned_v<value_type>, "gsh::RangeSumQuery::lower_bound / value_type must be unsigned.");
    i64 res = 0, n = size();
    for(i64 len = static_cast<i64>(std::bit_floor(static_cast<u64>(n))); len != 0; len >>= 1) {
      if(res + len <= n && bit[res + len - 1] < x) {
        x -= bit[res + len - 1];
        res += len;
      }
    }
    return res;
  }
  constexpr i64 upper_bound(value_type x) const {
    static_assert(std::is_unsigned_v<value_type>, "gsh::RangeSumQuery::upper_bound / value_type must be unsigned.");
    i64 res = 0, n = size();
    for(i64 len = static_cast<i64>(std::bit_floor(static_cast<u64>(n))); len != 0; len >>= 1) {
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
