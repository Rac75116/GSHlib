#pragma once
#include "Arr.hpp"

namespace gsh {

template<class T, class Allocator>
    requires std::is_same_v<T, typename AllocatorTraits<Allocator>::value_type> && (!std::is_const_v<T>)
class Vec;

template<class T, class F = gsh::Plus, class I = gsh::Negate, class Alloc = Allocator<T>> class StaticSum {
    Arr<T, Alloc> prefix_sum;
    [[no_unique_address]] F func;
    [[no_unique_address]] I inv;
    constexpr void build() {
        for (u32 i = 0; i + 1 < prefix_sum.size(); ++i) {
            prefix_sum[i + 1] = std::invoke(func, prefix_sum[i], prefix_sum[i + 1]);
        }
    }
public:
    using reference = T&;
    using const_reference = const T&;
    using iterator = T*;
    using const_iterator = const T*;
    using size_type = u32;
    using difference_type = i32;
    using value_type = T;
    using allocator_type = Alloc;
    constexpr StaticSum() : func(), inv() {}
    constexpr StaticSum(const allocator_type& a) noexcept : prefix_sum(a) {}
    constexpr StaticSum(const F& func, const I& inv, const allocator_type& a = allocator_type()) : prefix_sum(a), func(func), inv(inv) {}
    constexpr StaticSum(Arr<T, Alloc>&& v, const F& func = F(), const I& inv = I(), const allocator_type& a = allocator_type()) noexcept : prefix_sum(std::move(v), a), func(func), inv(inv) { build(); }
    constexpr StaticSum(Vec<T, Alloc>&& v, const F& func = F(), const I& inv = I(), const allocator_type& a = allocator_type()) noexcept : prefix_sum(a), func(func), inv(inv) { set(std::move(v)); }
    template<std::ranges::input_range R> constexpr StaticSum(R&& r, const F& func = F(), const I& inv = I(), const allocator_type& a = allocator_type()) : prefix_sum(std::ranges::begin(r), std::ranges::end(r), a), func(func), inv(inv) { build(); }
    constexpr void set(Arr<T, Alloc>&& v) noexcept {
        prefix_sum = std::move(v);
        build();
    }
    constexpr void set(Vec<T, Alloc>&& v) noexcept {
        prefix_sum.assign(ArrNoInit, v.data(), v.size());
        v.abandon();
        build();
    }
    template<std::ranges::input_range R> constexpr void set(R&& r) {
        prefix_sum.assign(std::ranges::begin(r), std::ranges::end(r));
        build();
    }
    constexpr T sum(u32 r) {
        if constexpr (std::is_default_constructible_v<T>) {
            if (r == 0) [[unlikely]]
                return T{};
        }
        return prefix_sum[r - 1];
    }
    constexpr T sum(u32 l, u32 r) {
        if constexpr (std::is_default_constructible_v<T>) {
            if (l >= r) [[unlikely]]
                return T{};
        }
        if (l == 0) [[unlikely]]
            return prefix_sum[r - 1];
        return std::invoke(func, prefix_sum[r - 1], std::invoke(inv, prefix_sum[l - 1]));
    }
};
template<std::ranges::input_range R, class F = gsh::Plus, class I = gsh::Negate, class Alloc = Allocator<std::ranges::range_value_t<R>>> StaticSum(R&&, F = F(), I = I(), Alloc = Alloc()) -> StaticSum<std::ranges::range_value_t<R>, F, I, Alloc>;

}  // namespace gsh