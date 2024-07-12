#pragma once
#include <type_traits>                 // std::is_unsigned_v
#include <bit>                         // std::bit_floor, std::bit_width, std::countr_zero
#include <iterator>                    // std::iterator_traits
#include <initializer_list>            // std::initializer_list
#include <gsh/internal/UtilMacro.hpp>  // GSH_INTERNAL_UNROLL
#include <gsh/Memory.hpp>              // gsh::Allocator, gsh::AllocatorTraits
#include <gsh/Arr.hpp>                 // gsh::Arr
#include <gsh/TypeDef.hpp>             // gsh::itype

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
    constexpr explicit RangeSumQuery(itype::u32 n, const Alloc& alloc = Alloc()) : bit(n, alloc) {}
    constexpr RangeSumQuery(itype::u32 n, const T& value, const Alloc& alloc = Alloc()) : bit(alloc) { assign(n, value); }
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
    constexpr itype::u32 size() const noexcept { return bit.size(); }
    constexpr void resize(itype::u32 sz) { resize(sz, value_type{}); }
    constexpr void resize(itype::u32 sz, const value_type& c) {
        itype::u32 n = bit.size();
        bit.resize(sz);
        if (n >= sz) return;
        // TODO
    }
    [[nodiscard]] constexpr bool empty() const noexcept { return bit.empty(); }
    constexpr value_type operator[](itype::u32 n) const {
        value_type res = bit[n];
        if (!(n & 1)) return res;
        itype::u32 tmp = n & (n + 1);
        for (itype::u32 i = n; i != tmp; i &= i - 1) res -= bit[i - 1];
        return res;
    }
    constexpr value_type at(itype::u32 n) const {
        if (n >= size()) throw Exception("gsh::RangeSumQuery::at / Index is out of range.");
        return operator[](n);
    }
    template<class InputIterator> constexpr void assign(InputIterator first, InputIterator last) {
        bit.assign(first, last);
        itype::u32 n = bit.size();
        if (n == 0) return;
        const auto tmp = bit[0];
        for (itype::u32 i = 0; i != n - 1; ++i) {
            const itype::u32 j = i + ((i + 1) & -(i + 1));
            bit[j < n ? j : 0] += bit[i];
        }
        bit[0] = tmp;
    }
    constexpr void assign(itype::u32 n, const T& u) {
        if (n == 0) return;
        bit = Arr<value_type, Alloc>(n, get_allocator());
        Arr<value_type, Alloc> mul(std::bit_width(n), get_allocator());
        mul[0] = u;
        for (itype::u32 i = 1, sz = mul.size(); i < sz; ++i) mul[i] = mul[i - 1], mul[i] += mul[i - 1];
        for (itype::u32 i = 0; i != n; ++i) bit[i] = mul[std::countr_zero(i + 1)];
    }
    constexpr void assign(std::initializer_list<T> il) { assign(il.begin(), il.end()); }
    constexpr void swap(RangeSumQuery& x) noexcept(AllocatorTraits<Alloc>::propagate_on_container_swap::value || AllocatorTraits<Alloc>::is_always_equal::value) { bit.swap(x.bit); };
    constexpr void clear() { bit.clear(); }
    constexpr allocator_type get_allocator() const noexcept { return bit.get_allocator(); }
    constexpr void add(itype::u32 n, const value_type& x) {
        GSH_INTERNAL_UNROLL(32)
        for (itype::u32 i = n + 1, sz = size(); i <= sz; i += (i & -i)) bit[i - 1] += x;
    }
    constexpr void minus(itype::u32 n, const value_type& x) {
        GSH_INTERNAL_UNROLL(32)
        for (itype::u32 i = n + 1, sz = size(); i <= sz; i += (i & -i)) bit[i - 1] -= x;
    }
    constexpr void increme(itype::u32 n) {
        GSH_INTERNAL_UNROLL(32)
        for (itype::u32 i = n + 1, sz = size(); i <= sz; i += (i & (-i))) ++bit[i - 1];
    }
    constexpr void decreme(itype::u32 n) {
        GSH_INTERNAL_UNROLL(32)
        for (itype::u32 i = n + 1, sz = size(); i <= sz; i += (i & (-i))) --bit[i - 1];
    }
    constexpr value_type sum(itype::u32 n) const {
        value_type res = {};
        GSH_INTERNAL_UNROLL(32)
        for (itype::u32 i = n; i != 0; i &= i - 1) res += bit[i - 1];
        return res;
    }
    constexpr value_type sum(itype::u32 l, itype::u32 r) const {
        itype::u32 n = l & ~((std::bit_floor(l ^ r) << 1) - 1);
        value_type res1 = {}, res2 = {};
        GSH_INTERNAL_UNROLL(32)
        for (itype::u32 i = r; i != n; i &= i - 1) res1 += bit[i - 1];
        GSH_INTERNAL_UNROLL(32)
        for (itype::u32 i = l; i != n; i &= i - 1) res2 += bit[i - 1];
        return res1 - res2;
    }
    constexpr itype::u32 lower_bound(value_type x) const {
        static_assert(std::is_unsigned_v<value_type>, "gsh::RangeSumQuery::lower_bound / value_type must be unsigned.");
        itype::u32 res = 0, n = size();
        for (itype::u32 len = std::bit_floor(n); len != 0; len >>= 1) {
            if (res + len <= n && bit[res + len - 1] < x) {
                x -= bit[res + len - 1];
                res += len;
            }
        }
        return res;
    }
    constexpr itype::u32 upper_bound(value_type x) const {
        static_assert(std::is_unsigned_v<value_type>, "gsh::RangeSumQuery::upper_bound / value_type must be unsigned.");
        itype::u32 res = 0, n = size();
        for (itype::u32 len = std::bit_floor(n); len != 0; len >>= 1) {
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