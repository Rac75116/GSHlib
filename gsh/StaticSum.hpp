#pragma once
#include "Vec.hpp"
namespace gsh {
template<class T, class F = gsh::Plus, class I = gsh::Negate, class Alloc = std::allocator<T>> class StaticSum {
  Vec<T, Alloc> prefix_sum;
  [[no_unique_address]] F func;
  [[no_unique_address]] I inv;
  constexpr void build() {
    for(i64 i = 0; i + 1 < prefix_sum.size(); ++i) { prefix_sum[i + 1] = std::invoke(func, prefix_sum[i], prefix_sum[i + 1]); }
  }
public:
  using reference = T&;
  using const_reference = const T&;
  using iterator = T*;
  using const_iterator = const T*;
  using size_type = i64;
  using difference_type = i64;
  using value_type = T;
  using allocator_type = Alloc;
  constexpr StaticSum() : func(), inv() {}
  constexpr StaticSum(const allocator_type& a) noexcept : prefix_sum(a) {}
  constexpr StaticSum(const F& func, const I& inv, const allocator_type& a = allocator_type()) : prefix_sum(a), func(func), inv(inv) {}
  constexpr StaticSum(Vec<T, Alloc>&& v, const F& func = F(), const I& inv = I(), const allocator_type& a = allocator_type()) noexcept : prefix_sum(std::move(v), a), func(func), inv(inv) { build(); }
  template<std::ranges::forward_range R> constexpr StaticSum(R&& r, const F& func = F(), const I& inv = I(), const allocator_type& a = allocator_type()) : prefix_sum(std::ranges::begin(r), std::ranges::end(r), a), func(func), inv(inv) { build(); }
  constexpr void set(Vec<T, Alloc>&& v) noexcept {
    prefix_sum = std::move(v);
    build();
  }
  template<std::ranges::input_range R> constexpr void set(R&& r) {
    prefix_sum.assign(std::ranges::begin(r), std::ranges::end(r));
    build();
  }
  constexpr T sum(i64 r) const {
    if constexpr(std::is_default_constructible_v<T>) {
      if(r == 0) [[unlikely]]
        return T{};
    }
    return prefix_sum[r - 1];
  }
  constexpr T sum(i64 l, i64 r) const {
    if constexpr(std::is_default_constructible_v<T>) {
      if(l >= r) [[unlikely]]
        return T{};
    }
    if(l == 0) [[unlikely]]
      return prefix_sum[r - 1];
    return std::invoke(func, prefix_sum[r - 1], std::invoke(inv, prefix_sum[l - 1]));
  }
};
template<std::ranges::forward_range R, class F = gsh::Plus, class I = gsh::Negate, class Alloc = std::allocator<std::ranges::range_value_t<R>>> StaticSum(R&&, F = F(), I = I(), Alloc = Alloc()) -> StaticSum<std::ranges::range_value_t<R>, F, I, Alloc>;
namespace internal {
template<class R> concept Range2D = std::ranges::forward_range<R> && std::ranges::forward_range<std::ranges::range_value_t<R>>;
template<class R> concept Range3D = std::ranges::forward_range<R> && Range2D<std::ranges::range_value_t<R>>;
} // namespace internal
template<class T, class F = gsh::Plus, class I = gsh::Negate, class Alloc = std::allocator<T>> class StaticSum2D {
  Vec<T, Alloc> prefix_sum;
  i64 row_size = 0;
  [[no_unique_address]] F func;
  [[no_unique_address]] I inv;
public:
  using reference = T&;
  using const_reference = const T&;
  using iterator = T*;
  using const_iterator = const T*;
  using size_type = i64;
  using difference_type = i64;
  using value_type = T;
  using allocator_type = Alloc;
  constexpr StaticSum2D() : func(), inv() {}
  constexpr StaticSum2D(const allocator_type& a) noexcept : prefix_sum(a) {}
  constexpr StaticSum2D(const F& func, const I& inv, const allocator_type& a = allocator_type()) : prefix_sum(a), func(func), inv(inv) {}
  template<internal::Range2D R> constexpr StaticSum2D(R&& r, const F& func = F(), const I& inv = I(), const allocator_type& a = allocator_type()) : prefix_sum(a), func(func), inv(inv) { set(std::forward<R>(r)); }
  template<internal::Range2D R> constexpr void set(R&& r) {
    auto itr = std::ranges::begin(r);
    auto sent = std::ranges::end(r);
    if(!(itr != sent)) {
      prefix_sum.clear();
      row_size = 0;
      return;
    }
    i64 h = static_cast<i64>(std::ranges::distance(itr, sent));
    i64 w = static_cast<i64>(std::ranges::size(*itr));
    prefix_sum.assign(w * h);
    row_size = w;
    i64 idx = 0;
    while(itr != sent) {
#ifndef NDEBUG
      i64 sz = static_cast<i64>(std::ranges::size(*itr));
      if(sz != row_size) [[unlikely]]
        throw Exception("gsh::StaticSum2D::set / The sizes of the rows are not the same.");
#endif
      for(auto&& val : *itr) { prefix_sum[idx++] = std::forward<decltype(val)>(val); }
      ++itr;
    }
    for(i64 i = 0; i < h; ++i) {
      for(i64 j = 0; j + 1 < w; ++j) {
        i64 idx = i * w + j;
        prefix_sum[idx + 1] = std::invoke(func, prefix_sum[idx], prefix_sum[idx + 1]);
      }
    }
    for(i64 i = 0; i + 1 < h; ++i) {
      for(i64 j = 0; j < w; ++j) {
        i64 idx = i * w + j;
        i64 next_idx = (i + 1) * w + j;
        prefix_sum[next_idx] = std::invoke(func, prefix_sum[idx], prefix_sum[next_idx]);
      }
    }
  }
  constexpr T sum(i64 x, i64 y) const {
    if constexpr(std::is_default_constructible_v<T>) {
      if(x == 0 || y == 0) [[unlikely]]
        return T{};
    }
    return prefix_sum[(x - 1) * row_size + (y - 1)];
  }
  constexpr T sum(i64 lx, i64 ly, i64 rx, i64 ry) const {
    if constexpr(std::is_default_constructible_v<T>) {
      if(lx >= rx || ly >= ry) [[unlikely]]
        return T{};
    }
    T res = prefix_sum[(rx - 1) * row_size + (ry - 1)];
    if(lx > 0) res = std::invoke(func, res, std::invoke(inv, prefix_sum[(lx - 1) * row_size + (ry - 1)]));
    if(ly > 0) res = std::invoke(func, res, std::invoke(inv, prefix_sum[(rx - 1) * row_size + (ly - 1)]));
    if(lx > 0 && ly > 0) res = std::invoke(func, res, prefix_sum[(lx - 1) * row_size + (ly - 1)]);
    return res;
  }
};
template<internal::Range2D R, class F = gsh::Plus, class I = gsh::Negate, class Alloc = std::allocator<std::ranges::range_value_t<std::ranges::range_value_t<R>>>> StaticSum2D(R&&, F = F(), I = I(), Alloc = Alloc()) -> StaticSum2D<std::ranges::range_value_t<std::ranges::range_value_t<R>>, F, I, Alloc>;
template<class T, class F = gsh::Plus, class I = gsh::Negate, class Alloc = std::allocator<T>> class StaticSum3D {
  Vec<T, Alloc> prefix_sum;
  i64 y_size = 0;
  i64 z_size = 0;
  [[no_unique_address]] F func;
  [[no_unique_address]] I inv;
public:
  using reference = T&;
  using const_reference = const T&;
  using iterator = T*;
  using const_iterator = const T*;
  using size_type = i64;
  using difference_type = i64;
  using value_type = T;
  using allocator_type = Alloc;
  constexpr StaticSum3D() : func(), inv() {}
  constexpr StaticSum3D(const allocator_type& a) noexcept : prefix_sum(a) {}
  constexpr StaticSum3D(const F& func, const I& inv, const allocator_type& a = allocator_type()) : prefix_sum(a), func(func), inv(inv) {}
  template<internal::Range3D R> constexpr StaticSum3D(R&& r, const F& func = F(), const I& inv = I(), const allocator_type& a = allocator_type()) : prefix_sum(a), func(func), inv(inv) { set(std::forward<R>(r)); }
  template<internal::Range3D R> constexpr void set(R&& r) {
    auto itr = std::ranges::begin(r);
    auto sent = std::ranges::end(r);
    if(!(itr != sent)) {
      prefix_sum.clear();
      y_size = 0;
      z_size = 0;
      return;
    }
    i64 h = static_cast<i64>(std::ranges::distance(itr, sent));
    i64 w = static_cast<i64>(std::ranges::size(*itr));
    i64 d = static_cast<i64>(std::ranges::size(*std::ranges::begin(*itr)));
    prefix_sum.assign(h * w * d);
    y_size = w;
    z_size = d;
    i64 idx = 0;
    while(itr != sent) {
#ifndef NDEBUG
      i64 sz_w = static_cast<i64>(std::ranges::size(*itr));
      if(sz_w != y_size) [[unlikely]]
        throw Exception("gsh::StaticSum3D::set / The sizes of the rows are not the same.");
#endif
      auto itr2 = std::ranges::begin(*itr);
      auto sent2 = std::ranges::end(*itr);
      while(itr2 != sent2) {
#ifndef NDEBUG
        i64 sz_d = static_cast<i64>(std::ranges::size(*itr2));
        if(sz_d != z_size) [[unlikely]]
          throw Exception("gsh::StaticSum3D::set / The sizes of the depths are not the same.");
#endif
        for(auto&& val : *itr2) { prefix_sum[idx++] = std::forward<decltype(val)>(val); }
        ++itr2;
      }
      ++itr;
    }
    for(i64 i = 0; i < h; ++i) {
      for(i64 j = 0; j < w; ++j) {
        for(i64 k = 0; k + 1 < d; ++k) {
          i64 idx = (i * w + j) * d + k;
          prefix_sum[idx + 1] = std::invoke(func, prefix_sum[idx], prefix_sum[idx + 1]);
        }
      }
    }
    for(i64 i = 0; i < h; ++i) {
      for(i64 j = 0; j + 1 < w; ++j) {
        for(i64 k = 0; k < d; ++k) {
          i64 idx = (i * w + j) * d + k;
          i64 next_idx = (i * w + (j + 1)) * d + k;
          prefix_sum[next_idx] = std::invoke(func, prefix_sum[idx], prefix_sum[next_idx]);
        }
      }
    }
    for(i64 i = 0; i + 1 < h; ++i) {
      for(i64 j = 0; j < w; ++j) {
        for(i64 k = 0; k < d; ++k) {
          i64 idx = (i * w + j) * d + k;
          i64 next_idx = ((i + 1) * w + j) * d + k;
          prefix_sum[next_idx] = std::invoke(func, prefix_sum[idx], prefix_sum[next_idx]);
        }
      }
    }
  }
  constexpr T sum(i64 x, i64 y, i64 z) const {
    if constexpr(std::is_default_constructible_v<T>) {
      if(x == 0 || y == 0 || z == 0) [[unlikely]]
        return T{};
    }
    return prefix_sum[((x - 1) * y_size + (y - 1)) * z_size + (z - 1)];
  }
  constexpr T sum(i64 lx, i64 ly, i64 lz, i64 rx, i64 ry, i64 rz) const {
    if constexpr(std::is_default_constructible_v<T>) {
      if(lx >= rx || ly >= ry || lz >= rz) [[unlikely]]
        return T{};
    }
    T res = prefix_sum[((rx - 1) * y_size + (ry - 1)) * z_size + (rz - 1)];
    if(lx > 0) res = std::invoke(func, res, std::invoke(inv, prefix_sum[((lx - 1) * y_size + (ry - 1)) * z_size + (rz - 1)]));
    if(ly > 0) res = std::invoke(func, res, std::invoke(inv, prefix_sum[((rx - 1) * y_size + (ly - 1)) * z_size + (rz - 1)]));
    if(lz > 0) res = std::invoke(func, res, std::invoke(inv, prefix_sum[((rx - 1) * y_size + (ry - 1)) * z_size + (lz - 1)]));
    if(lx > 0 && ly > 0) res = std::invoke(func, res, prefix_sum[((lx - 1) * y_size + (ly - 1)) * z_size + (rz - 1)]);
    if(lx > 0 && lz > 0) res = std::invoke(func, res, prefix_sum[((lx - 1) * y_size + (ry - 1)) * z_size + (lz - 1)]);
    if(ly > 0 && lz > 0) res = std::invoke(func, res, prefix_sum[((rx - 1) * y_size + (ly - 1)) * z_size + (lz - 1)]);
    if(lx > 0 && ly > 0 && lz > 0) res = std::invoke(func, res, std::invoke(inv, prefix_sum[((lx - 1) * y_size + (ly - 1)) * z_size + (lz - 1)]));
    return res;
  }
};
template<internal::Range3D R, class F = gsh::Plus, class I = gsh::Negate, class Alloc = std::allocator<std::ranges::range_value_t<std::ranges::range_value_t<std::ranges::range_value_t<R>>>>> StaticSum3D(R&&, F = F(), I = I(), Alloc = Alloc()) -> StaticSum3D<std::ranges::range_value_t<std::ranges::range_value_t<std::ranges::range_value_t<R>>>, F, I, Alloc>;
} // namespace gsh
