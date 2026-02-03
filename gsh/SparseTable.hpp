#pragma once
#include "Algorithm.hpp"
#include "Exception.hpp"
#include "Functional.hpp"
#include "Numeric.hpp"
#include "Range.hpp"
#include "TypeDef.hpp"
#include "Vec.hpp"
#include <bit>
#include <concepts>
#include <functional>
#include <initializer_list>
#include <iterator>
#include <limits>
#include <type_traits>
namespace gsh {
namespace internal {
template<class Op, class Id> concept IsValidSparseSpec = std::invocable<Id> && requires(Op op, const std::invoke_result_t<Id>& a, const std::invoke_result_t<Id>& b) {
  { op(a, b) } -> std::convertible_to<std::remove_cvref_t<std::invoke_result_t<Id>>>;
};
template<class Spec> concept IsSparseSpecImplemented = requires(Spec spec) {
  typename Spec::value_type;
  { Spec::block_shift } -> std::same_as<const u32&>;
  { spec.op(std::declval<typename Spec::value_type>(), std::declval<typename Spec::value_type>()) } -> std::same_as<typename Spec::value_type>;
  { spec.e() } -> std::same_as<typename Spec::value_type>;
};
template<class T, u32 BlockShift, class Op, class Id> requires IsValidSparseSpec<Op, Id> class DefaultSparseSpec {
  [[no_unique_address]] mutable Op op_func;
  [[no_unique_address]] mutable Id id_func;
public:
  using value_type = T;
  static constexpr u32 block_shift = BlockShift;
  constexpr DefaultSparseSpec() = default;
  constexpr DefaultSparseSpec(const Op& op, const Id& id) : op_func(op), id_func(id) {}
  constexpr value_type op(const value_type& a, const value_type& b) const noexcept(noexcept(std::is_nothrow_invocable_v<Op, const value_type&, const value_type&>)) { return static_cast<value_type>(std::invoke(op_func, a, b)); }
  constexpr value_type e() const noexcept(noexcept(std::is_nothrow_invocable_v<Id>)) { return static_cast<value_type>(std::invoke(id_func)); }
};
}
template<class T, u32 BlockShift = 2, class Op, class Id> constexpr internal::DefaultSparseSpec<T, BlockShift, Op, Id> MakeSparseSpec(const Op& op = Op(), const Id& id = Id()) { return {op, id}; }
namespace sparse_table_specs {
template<class T, u32 BlockShift = 2> class RangeOr : public decltype(MakeSparseSpec<T, BlockShift>(Or, []() -> T { return static_cast<T>(0); })) {};
template<class T, u32 BlockShift = 2> class RangeAnd : public decltype(MakeSparseSpec<T, BlockShift>(And, []() -> T { return ~static_cast<T>(0); })) {};
template<class T, u32 BlockShift = 2> class RangeMin : public decltype(MakeSparseSpec<T, BlockShift>(Min, []() -> T { return std::numeric_limits<T>::max(); })) {};
template<class T, u32 BlockShift = 2> class RangeMax : public decltype(MakeSparseSpec<T, BlockShift>(Max, []() -> T { return std::numeric_limits<T>::min(); })) {};
template<class T, u32 BlockShift = 2> class RangeGCD : public decltype(MakeSparseSpec<T, BlockShift>(GCD, []() -> T { return static_cast<T>(0); })) {};
template<class T, u32 BlockShift = 2> class RangeLCM : public decltype(MakeSparseSpec<T, BlockShift>(LCM, []() -> T { return static_cast<T>(1); })) {};
}
template<class Spec> requires internal::IsSparseSpecImplemented<Spec> class SparseTable : public ViewInterface<SparseTable<Spec>, typename Spec::value_type> {
  [[no_unique_address]] Spec spec;
public:
  using value_type = typename Spec::value_type;
  using size_type = u32;
  using difference_type = i32;
private:
  static constexpr size_type block_shift = Spec::block_shift;
  static constexpr size_type block_size = 1u << block_shift;
  size_type n = 0;
  size_type block_count = 0;
  size_type log = 0;
  Vec<value_type> data;
  Vec<value_type> pre;
  Vec<value_type> suf;
  Vec<value_type> table;
  Vec<size_type> offset;
  constexpr const value_type& table_at(size_type k, size_type i) const { return table[offset[k] + i]; }
  constexpr value_type& table_at(size_type k, size_type i) { return table[offset[k] + i]; }
  constexpr void build() {
    if(n == 0) {
      block_count = 0;
      log = 0;
      pre.clear();
      suf.clear();
      table.clear();
      offset.clear();
      return;
    }
    pre.assign(n, spec.e());
    suf.assign(n, spec.e());
    block_count = (n + block_size - 1) >> block_shift;
    log = std::bit_width(block_count);
    offset.assign(log, 0);
    size_type total = 0;
    for(size_type k = 0; k < log; ++k) {
      offset[k] = total;
      total += block_count - (1u << k) + 1;
    }
    table.assign(total, spec.e());
    for(size_type b = 0; b < block_count; ++b) {
      const size_type start = b * block_size;
      const size_type end = (start + block_size < n) ? (start + block_size) : n;
      value_type cur = data[start];
      for(size_type i = start; i < end; ++i) {
        if(i != start) cur = spec.op(cur, data[i]);
        suf[i] = cur;
      }
      cur = data[end - 1];
      for(size_type i = end; i-- > start;) {
        if(i != end - 1) cur = spec.op(data[i], cur);
        pre[i] = cur;
      }
      table_at(0, b) = suf[end - 1];
    }
    for(size_type k = 1; k < log; ++k) {
      const size_type len = block_count - (1u << k) + 1;
      const size_type shift = 1u << (k - 1);
      for(size_type i = 0; i < len; ++i) { table_at(k, i) = spec.op(table_at(k - 1, i), table_at(k - 1, i + shift)); }
    }
  }
public:
  constexpr SparseTable() = default;
  constexpr SparseTable(Spec spec) : spec(spec) {}
  constexpr SparseTable(size_type n, Spec spec = Spec()) : spec(spec) { assign(n, spec.e()); }
  template<class InputIt> requires std::forward_iterator<InputIt> constexpr SparseTable(InputIt first, InputIt last, Spec spec = Spec()) : spec(spec) { assign(first, last); }
  constexpr SparseTable(size_type n, const value_type& value, Spec spec = Spec()) : spec(spec) { assign(n, value); }
  constexpr SparseTable(std::initializer_list<value_type> init, Spec spec = Spec()) : spec(spec) { assign(init.begin(), init.end()); }
  constexpr SparseTable& operator=(std::initializer_list<value_type> il) {
    assign(il);
    return *this;
  }
  constexpr auto begin() const { return data.cbegin(); }
  constexpr auto end() const { return data.cend(); }
  constexpr auto cbegin() const { return data.cbegin(); }
  constexpr auto cend() const { return data.cend(); }
  constexpr void clear() {
    n = 0;
    block_count = 0;
    log = 0;
    data.clear();
    pre.clear();
    suf.clear();
    table.clear();
    offset.clear();
  }
  constexpr bool empty() const { return n == 0; }
  constexpr size_type size() const { return n; }
  constexpr void resize(size_type n) { resize(n, spec.e()); }
  constexpr void resize(size_type n, const value_type& c) {
    Vec<value_type> tmp;
    tmp.reserve(this->n);
    for(size_type i = 0; i < this->n; ++i) tmp.emplace_back(operator[](i));
    tmp.resize(n, c);
    assign(tmp.begin(), tmp.end());
  }
  template<class InputIt> requires std::forward_iterator<InputIt> constexpr void assign(InputIt first, InputIt last) {
    data.assign(first, last);
    n = data.size();
    build();
  }
  constexpr void assign(size_type n, const value_type& u) {
    data.assign(n, u);
    this->n = n;
    build();
  }
  constexpr void assign(std::initializer_list<value_type> il) { assign(il.begin(), il.end()); }
  constexpr void swap(SparseTable& r) {
    using std::swap;
    swap(spec, r.spec);
    swap(n, r.n);
    swap(block_count, r.block_count);
    swap(log, r.log);
    swap(data, r.data);
    swap(pre, r.pre);
    swap(suf, r.suf);
    swap(table, r.table);
    swap(offset, r.offset);
  }
  constexpr value_type prod(size_type l, size_type r) const {
#ifndef NDEBUG
    if(l > r || r > n) throw Exception("SparseTable::prod: invalid range [", l, ", ", r, ") with size ", n);
#endif
    if(l == r) return spec.e();
    const size_type L = l >> block_shift;
    const size_type R = (r - 1) >> block_shift;
    if(L == R) {
      value_type res = data[l];
      for(size_type i = l + 1; i < r; ++i) res = spec.op(res, data[i]);
      return res;
    }
    value_type res = pre[l];
    if(L + 1 < R) {
      const size_type left = L + 1;
      const size_type right = R;
      const size_type len = right - left;
      const size_type k = std::bit_width(len) - 1;
      const value_type mid = spec.op(table_at(k, left), table_at(k, right - (1u << k)));
      res = spec.op(res, mid);
    }
    res = spec.op(res, suf[r - 1]);
    return res;
  }
  constexpr value_type all_prod() const { return n > 0 ? prod(0, n) : spec.e(); }
  constexpr const value_type& operator[](size_type i) const {
#ifndef NDEBUG
    if(i >= n) throw Exception("SparseTable::operator[]: index ", i, " is out of range [0, ", n, ")");
#endif
    return data[i];
  }
  constexpr const value_type& get(size_type i) const { return (*this)[i]; }
};
}
