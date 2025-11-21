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
    template<std::ranges::forward_range R> constexpr StaticSum(R&& r, const F& func = F(), const I& inv = I(), const allocator_type& a = allocator_type()) : prefix_sum(std::ranges::begin(r), std::ranges::end(r), a), func(func), inv(inv) { build(); }
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
    constexpr T sum(u32 r) const {
        if constexpr (std::is_default_constructible_v<T>) {
            if (r == 0) [[unlikely]]
                return T{};
        }
        return prefix_sum[r - 1];
    }
    constexpr T sum(u32 l, u32 r) const {
        if constexpr (std::is_default_constructible_v<T>) {
            if (l >= r) [[unlikely]]
                return T{};
        }
        if (l == 0) [[unlikely]]
            return prefix_sum[r - 1];
        return std::invoke(func, prefix_sum[r - 1], std::invoke(inv, prefix_sum[l - 1]));
    }
};
template<std::ranges::forward_range R, class F = gsh::Plus, class I = gsh::Negate, class Alloc = Allocator<std::ranges::range_value_t<R>>> StaticSum(R&&, F = F(), I = I(), Alloc = Alloc()) -> StaticSum<std::ranges::range_value_t<R>, F, I, Alloc>;

namespace internal {
    template<class R> concept Range2D = std::ranges::forward_range<R> && std::ranges::forward_range<std::ranges::range_value_t<R>>;
}
template<class T, class F = gsh::Plus, class I = gsh::Negate, class Alloc = Allocator<T>> class StaticSum2D {
    Arr<T, Alloc> prefix_sum;
    u32 row_size = 0;
    [[no_unique_address]] F func;
    [[no_unique_address]] I inv;
public:
    using reference = T&;
    using const_reference = const T&;
    using iterator = T*;
    using const_iterator = const T*;
    using size_type = u32;
    using difference_type = i32;
    using value_type = T;
    using allocator_type = Alloc;
    constexpr StaticSum2D() : func(), inv() {}
    constexpr StaticSum2D(const allocator_type& a) noexcept : prefix_sum(a) {}
    constexpr StaticSum2D(const F& func, const I& inv, const allocator_type& a = allocator_type()) : prefix_sum(a), func(func), inv(inv) {}
    template<internal::Range2D R> constexpr StaticSum2D(R&& r, const F& func = F(), const I& inv = I(), const allocator_type& a = allocator_type()) : prefix_sum(a), func(func), inv(inv) { set(std::forward<R>(r)); }
    template<internal::Range2D R> constexpr void set(R&& r) {
        auto itr = std::ranges::begin(r);
        auto sent = std::ranges::end(r);
        if (!(itr != sent)) {
            prefix_sum.clear();
            row_size = 0;
            return;
        }
        u32 h = std::ranges::distance(itr, sent);
        u32 w = std::ranges::size(*itr);
        prefix_sum.assign(ArrNoInit, w * h);
        row_size = w;
        u32 idx = 0;
        while (itr != sent) {
#ifndef NDEBUG
            u32 sz = std::ranges::size(*itr);
            if (sz != row_size) [[unlikely]]
                throw Exception("gsh::StaticSum2D::set / The sizes of the rows are not the same.");
#endif
            for (auto&& val : *itr) {
                prefix_sum[idx++] = std::forward<decltype(val)>(val);
            }
            ++itr;
        }

        for (u32 i = 0; i < h; ++i) {
            for (u32 j = 0; j + 1 < w; ++j) {
                u32 idx = i * w + j;
                prefix_sum[idx + 1] = std::invoke(func, prefix_sum[idx], prefix_sum[idx + 1]);
            }
        }
        for (u32 i = 0; i + 1 < h; ++i) {
            for (u32 j = 0; j < w; ++j) {
                u32 idx = i * w + j;
                u32 next_idx = (i + 1) * w + j;
                prefix_sum[next_idx] = std::invoke(func, prefix_sum[idx], prefix_sum[next_idx]);
            }
        }
    }
    constexpr T sum(u32 x, u32 y) const {
        if constexpr (std::is_default_constructible_v<T>) {
            if (x == 0 || y == 0) [[unlikely]]
                return T{};
        }
        return prefix_sum[(x - 1) * row_size + (y - 1)];
    }
    constexpr T sum(u32 lx, u32 ly, u32 rx, u32 ry) const {
        if constexpr (std::is_default_constructible_v<T>) {
            if (lx >= rx || ly >= ry) [[unlikely]]
                return T{};
        }
        T res = prefix_sum[(rx - 1) * row_size + (ry - 1)];
        if (lx > 0) res = std::invoke(func, res, std::invoke(inv, prefix_sum[(lx - 1) * row_size + (ry - 1)]));
        if (ly > 0) res = std::invoke(func, res, std::invoke(inv, prefix_sum[(rx - 1) * row_size + (ly - 1)]));
        if (lx > 0 && ly > 0) res = std::invoke(func, res, prefix_sum[(lx - 1) * row_size + (ly - 1)]);
        return res;
    }
};
template<internal::Range2D R, class F = gsh::Plus, class I = gsh::Negate, class Alloc = Allocator<std::ranges::range_value_t<std::ranges::range_value_t<R>>>> StaticSum2D(R&&, F = F(), I = I(), Alloc = Alloc()) -> StaticSum2D<std::ranges::range_value_t<std::ranges::range_value_t<R>>, F, I, Alloc>;

}  // namespace gsh
