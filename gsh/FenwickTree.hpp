#pragma once
#include <type_traits>              // std::is_unsigned_v
#include <bit>                      // std::bit_floor, std::bit_width, std::countr_zero
#include <iterator>                 // std::iterator_traits
#include <initializer_list>         // std::initializer_list
#include <gsh/internal/Pragma.hpp>  // GSH_INTERNAL_UNROLL
#include <gsh/Memory.hpp>           // gsh::Allocator, gsh::AllocatorTraits
#include <gsh/Arr.hpp>              // gsh::Arr
#include <gsh/TypeDef.hpp>          // gsh::itype

namespace gsh {

template<class T, class Alloc = Allocator<T>> class RangeSumQuery {
    Arr<T, Alloc> bit;
public:
    using reference = T&;
    using const_reference = const T&;
    using size_type = itype::u32;
    using difference_type = itype::i32;
    using value_type = T;
    using allocator_type = Alloc;
    using pointer = typename AllocatorTraits<Alloc>::pointer;
    using const_pointer = typename AllocatorTraits<Alloc>::const_pointer;
    constexpr RangeSumQuery() noexcept(noexcept(Alloc())) : RangeSumQuery(Alloc()) {}
    constexpr explicit RangeSumQuery(const Alloc& alloc) noexcept : bit(alloc) {}
    constexpr explicit RangeSumQuery(size_type n, const Alloc& alloc = Alloc()) : bit(n, alloc) {}
    constexpr RangeSumQuery(size_type n, const T& value, const Alloc& alloc = Alloc()) : bit(alloc) { assign(n, value); }
    template<class InputIter> constexpr RangeSumQuery(InputIter first, InputIter last, const Alloc& alloc = Alloc()) : bit(alloc) { assign(first, last); }
    constexpr RangeSumQuery(const RangeSumQuery&) = default;
    constexpr RangeSumQuery(RangeSumQuery&&) noexcept = default;
    constexpr RangeSumQuery(const RangeSumQuery& x, const Alloc& alloc) : bit(x.bit, alloc) {}
    constexpr RangeSumQuery(RangeSumQuery&& x, const Alloc& alloc) : bit(std::move(x.bit), alloc) {}
    constexpr RangeSumQuery(std::initializer_list<T> il, const Alloc& alloc = Alloc()) : RangeSumQuery(il.begin(), il.end(), alloc) {}
    constexpr RangeSumQuery& operator=(const RangeSumQuery&) = default;
    constexpr RangeSumQuery& operator=(RangeSumQuery&&) noexcept(AllocatorTraits<Alloc>::propagate_on_container_move_assignment::value || AllocatorTraits<Alloc>::is_always_equal::value) = default;
    constexpr RangeSumQuery& operator=(std::initializer_list<T> il) {
        assign(il);
        return *this;
    }
    constexpr size_type size() const noexcept { return bit.size(); }
    constexpr void resize(size_type sz) { resize(sz, value_type{}); }
    constexpr void resize(size_type sz, const value_type& c) {
        size_type n = bit.size();
        bit.resize(sz);
        if (n >= sz) return;
        // TODO
    }
    [[nodiscard]] constexpr bool empty() const noexcept { return bit.empty(); }
    constexpr value_type operator[](size_type n) const {
        value_type res = bit[n];
        if (!(n & 1)) return res;
        size_type tmp = n & (n + 1);
        for (size_type i = n; i != tmp; i &= i - 1) res -= bit[i - 1];
        return res;
    }
    constexpr value_type at(size_type n) const {
        if (n >= size()) throw Exception("gsh::RangeSumQuery::at / Index is out of range.");
        return operator[](n);
    }
    template<class InputIterator> constexpr void assign(InputIterator first, InputIterator last) {
        bit.assign(first, last);
        size_type n = bit.size();
        for (size_type i = 1; i != n; ++i) {
            const size_type a = i - 1;
            const size_type b = i & -i;
            bit[a + b] += (a + b < n ? bit[a] : value_type{});
        }
    }
    constexpr void assign(size_type n, const T& u) {
        if (n == 0) return;
        bit = Vec<value_type, Alloc>(n, get_allocator());
        Vec<value_type, Alloc> mul(std::bit_width(n), get_allocator());
        mul[0] = u;
        for (size_type i = 1, sz = mul.size(); i < sz; ++i) mul[i] = mul[i - 1], mul[i] += mul[i - 1];
        for (size_type i = 1; i <= n; ++i) bit[i - 1] = mul[std::countr_zero(i)];
    }
    constexpr void assign(std::initializer_list<T> il) { assign(il.begin(), il.end()); }
    constexpr void swap(RangeSumQuery& x) noexcept(AllocatorTraits<Alloc>::propagate_on_container_swap::value || AllocatorTraits<Alloc>::is_always_equal::value) { bit.swap(x.bit); };
    constexpr void clear() { bit.clear(); }
    constexpr allocator_type get_allocator() const noexcept { return bit.get_allocator(); }
    constexpr void add(size_type n, const value_type& x) {
        GSH_INTERNAL_UNROLL(32)
        for (size_type i = n + 1, sz = size(); i <= sz; i += (i & -i)) bit[i - 1] += x;
    }
    constexpr void minus(size_type n, const value_type& x) {
        GSH_INTERNAL_UNROLL(32)
        for (size_type i = n + 1, sz = size(); i <= sz; i += (i & -i)) bit[i - 1] -= x;
    }
    constexpr void increme(size_type n) {
        GSH_INTERNAL_UNROLL(32)
        for (size_type i = n + 1, sz = size(); i <= sz; i += (i & (-i))) ++bit[i - 1];
    }
    constexpr void decreme(size_type n) {
        GSH_INTERNAL_UNROLL(32)
        for (size_type i = n + 1, sz = size(); i <= sz; i += (i & (-i))) --bit[i - 1];
    }
    constexpr value_type sum(size_type n) const {
        value_type res = {};
        GSH_INTERNAL_UNROLL(32)
        for (size_type i = n; i != 0; i &= i - 1) res += bit[i - 1];
        return res;
    }
    constexpr value_type sum(size_type l, size_type r) const {
        size_type n = l & ~((std::bit_floor(l ^ r) << 1) - 1);
        value_type res = {};
        GSH_INTERNAL_UNROLL(32)
        for (size_type i = r; i != n; i &= i - 1) res += bit[i - 1];
        GSH_INTERNAL_UNROLL(32)
        for (size_type i = l; i != n; i &= i - 1) res -= bit[i - 1];
        return res;
    }
    constexpr size_type lower_bound(value_type x) const {
        static_assert(std::is_unsigned_v<value_type>, "gsh::RangeSumQuery::lower_bound / value_type must be unsigned.");
        size_type res = 0, n = size();
        for (size_type len = std::bit_floor(n); len != 0; len >>= 1) {
            if (res + len <= n && bit[res + len - 1] < x) {
                x -= bit[res + len - 1];
                res += len;
            }
        }
        return res;
    }
    constexpr size_type upper_bound(value_type x) const {
        static_assert(std::is_unsigned_v<value_type>, "gsh::RangeSumQuery::upper_bound / value_type must be unsigned.");
        size_type res = 0, n = size();
        for (size_type len = std::bit_floor(n); len != 0; len >>= 1) {
            if (res + len <= n && !(x < bit[res + len - 1])) {
                x -= bit[res + len - 1];
                res += len;
            }
        }
        return res;
    }
};
template<class U, class Alloc> constexpr void swap(RangeSumQuery<U, Alloc>& x, RangeSumQuery<U, Alloc>& y) noexcept(noexcept(x.swap(y))) {
    x.swap(y);
}
template<class InputIterator, class Alloc = Allocator<typename std::iterator_traits<InputIterator>::value_type>> RangeSumQuery(InputIterator, InputIterator, Alloc = Alloc()) -> RangeSumQuery<typename std::iterator_traits<InputIterator>::value_type, Alloc>;

}  // namespace gsh