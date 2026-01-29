#pragma once
#include "Algorithm.hpp"
#include "Int128.hpp"
#include "Random.hpp"
#include "Range.hpp"
#include "TypeDef.hpp"
#include "Vec.hpp"
#include "internal/Operation.hpp"
#include <bit>
#include <cmath>
#include <type_traits>
namespace gsh {
template<class T> requires std::is_arithmetic_v<T> class Point2 : public internal::ArithmeticInterface<Point2<T>> {
public:
  T x{}, y{};
  constexpr Point2() {}
  constexpr Point2(const T& a, const T& b) : x(a), y(b) {}
  using value_type = T;
  constexpr Point2& operator+=(const Point2& p) noexcept {
    x += p.x, y += p.y;
    return *this;
  }
  constexpr Point2& operator-=(const Point2& p) noexcept {
    x -= p.x, y -= p.y;
    return *this;
  }
  friend constexpr bool operator==(const Point2& a, const Point2& b) { return a.x == b.x && a.y == b.y; }
};
template<class T = f64, class U> constexpr T Norm(const Point2<U>& a) { return std::hypot(static_cast<T>(a.x), static_cast<T>(a.y)); }
template<class T> constexpr T NormSquare(const Point2<T>& a) { return a.x * a.x + a.y * a.y; }
template<class T> constexpr T Dot(const Point2<T>& a, const Point2<T>& b) { return a.x * b.x + a.y * b.y; }
template<class T> constexpr T Cross(const Point2<T>& a, const Point2<T>& b) { return a.x * b.y + a.y * b.x; }
template<class T, class U> constexpr T NormSquare(const Point2<U>& a) { return static_cast<T>(a.x) * static_cast<T>(a.x) + static_cast<T>(a.y) * static_cast<T>(a.y); }
template<class T, class U> constexpr T Dot(const Point2<U>& a, const Point2<U>& b) { return static_cast<T>(a.x) * static_cast<T>(b.x) + static_cast<T>(a.y) * static_cast<T>(b.y); }
template<class T, class U> constexpr T Cross(const Point2<U>& a, const Point2<U>& b) { return static_cast<T>(a.x) * static_cast<T>(b.y) - static_cast<T>(a.y) * static_cast<T>(b.x); }
template<class T = f64, class U> constexpr T Angle(const Point2<U>& a) { return std::atan2(static_cast<T>(a.y), static_cast<T>(a.x)); }
template<class T = f64, class U, class V> constexpr T Angle(const Point2<U>& a, const Point2<V>& b) { return std::atan2(static_cast<T>(Cross(a, b)), static_cast<T>(Dot(a, b))); }
template<class T = f64, class U> constexpr Point2<T> Rotate(const Point2<U>& a, const T& theta) {
  const T cos_theta = std::cos(theta);
  const T sin_theta = std::sin(theta);
  return {static_cast<T>(a.x) * cos_theta - static_cast<T>(a.y) * sin_theta, static_cast<T>(a.x) * sin_theta + static_cast<T>(a.y) * cos_theta};
}
template<class T> requires std::is_arithmetic_v<T> class Point3 : public internal::ArithmeticInterface<Point3<T>> {
public:
  T x{}, y{}, z{};
  constexpr Point3() {}
  constexpr Point3(const T& a, const T& b, const T& c) : x(a), y(b), z(c) {}
  using value_type = T;
  constexpr Point3& operator+=(const Point3& p) {
    x += p.x, y += p.y, z += p.z;
    return *this;
  }
  constexpr Point3& operator-=(const Point3& p) {
    x -= p.x, y -= p.y, z -= p.z;
    return *this;
  }
  friend constexpr bool operator==(const Point3& a, const Point3& b) { return a.x == b.x && a.y == b.y && a.z == b.z; }
};
template<class T = f64, class U> constexpr T Norm(const Point3<U>& a) { return std::hypot(static_cast<T>(a.x), static_cast<T>(a.y), static_cast<T>(a.z)); }
template<class T> constexpr T NormSquare(const Point3<T>& a) { return a.x * a.x + a.y * a.y + a.z * a.z; }
template<class T> constexpr T Dot(const Point3<T>& a, const Point3<T>& b) { return a.x * b.x + a.y * b.y + a.z * b.z; }
template<class T> constexpr Point3<T> Cross(const Point3<T>& a, const Point3<T>& b) { return {a.y * b.z - a.z * b.y, a.z * b.x - a.x * b.z, a.x * b.y - a.y * b.x}; }
template<class T, class U> constexpr T NormSquare(const Point3<U>& a) { return static_cast<T>(a.x) * static_cast<T>(a.x) + static_cast<T>(a.y) * static_cast<T>(a.y) + static_cast<T>(a.z) * static_cast<T>(a.z); }
template<class T, class U> constexpr T Dot(const Point3<U>& a, const Point3<U>& b) { return static_cast<T>(a.x) * static_cast<T>(b.x) + static_cast<T>(a.y) * static_cast<T>(b.y) + static_cast<T>(a.z) * static_cast<T>(b.z); }
template<class T, class U> constexpr Point3<T> Cross(const Point3<U>& a, const Point3<U>& b) { return {static_cast<T>(a.y) * static_cast<T>(b.z) - static_cast<T>(a.z) * static_cast<T>(b.y), static_cast<T>(a.z) * static_cast<T>(b.x) - static_cast<T>(a.x) * static_cast<T>(b.z), static_cast<T>(a.x) * static_cast<T>(b.y) - static_cast<T>(a.y) * static_cast<T>(b.x)}; }
constexpr u64 ArgumentOrder(i32 x, i32 y) {
  u64 ord = 0;
  const bool xs = (x >= 0), ys = (y >= 0);
  const u64 xu = (xs ? x : -x), yu = (ys ? y : -y);
  const u64 mx = (xu < yu ? yu : xu), mn = (xu < yu ? xu : yu);
  const bool rev = (xs ^ ys) ^ (xu < yu);
  const u64 id = ys * 4ull + (xs ^ ys) * 2ull + rev;
  if(x == 0 && y == 0) ord = 4ull << 61;
  else {
    constexpr u64 li = (1ull << 61) - 1;
    u128 m = u128(mn) * li;
    u64 hi = m >> 64, lo = m;
    const u64 tmp = internal::Divu128(hi, lo, mx).first;
    ord = id << 61 | (rev ? li - tmp : tmp);
  }
  return ord;
}
constexpr u64 ArgumentOrder(const Point2<i32>& p) { return ArgumentOrder(p.x, p.y); }
template<std::ranges::input_range T> requires std::same_as<std::ranges::range_value_t<T>, Point2<i32>> constexpr Vec<Point2<i32>> ArgumentSort(T&& r) {
  Vec<u128> v(std::ranges::size(r));
  for(u32 i = 0; const auto& p : r) v[i++] = static_cast<u128>(std::bit_cast<u64>(p)) << 64 | ArgumentOrder(p);
  v.sort({}, [](u128 x) { return static_cast<u64>(x); });
  Vec<Point2<i32>> res(v.size());
  for(u32 i = 0, j = v.size(); i != j; ++i) res[i] = std::bit_cast<Point2<i32>>(static_cast<u64>(v[i] >> 64));
  return res;
}
template<std::ranges::input_range T> requires std::same_as<std::remove_cvref_t<std::ranges::range_value_t<T>>, Point2<i32>> constexpr Vec<Point2<i32>> ConvexHull(T&& r) {
  const u32 n = std::ranges::size(r);
  if(n <= 1) return r;
  u32 m = 1;
  Vec<Point2<i32>> p(n);
  {
    Vec<u64> sorted(n);
    for(u32 i = 0; auto&& e : r) sorted[i++] = std::bit_cast<u64>(e) ^ 0x8000000080000000;
    sorted.sort();
    p[0] = std::bit_cast<Point2<i32>>(sorted[0] ^ 0x8000000080000000);
    for(u32 i = 1; i != n; ++i) {
      p[m] = std::bit_cast<Point2<i32>>(sorted[i] ^ 0x8000000080000000);
      m += sorted[i] != sorted[i - 1];
    }
  }
  if(m <= 2) {
    p.resize(m);
    return p;
  }
  Vec<Point2<i32>> ch(2 * m);
  u32 k = 0;
  for(u32 i = 0; i < m; ch[k++] = p[i++]) {
    while(k >= 2 && Cross<i64>(ch[k - 1] - ch[k - 2], p[i] - ch[k - 2]) <= 0) --k;
  }
  for(u32 i = m - 1, t = k + 1; i > 0; ch[k++] = p[--i]) {
    while(k >= t && Cross<i64>(ch[k - 1] - ch[k - 2], p[i - 1] - ch[k - 2]) <= 0) --k;
  }
  ch.resize(k - 1);
  return ch;
}
template<std::ranges::random_access_range T> requires std::same_as<std::remove_cvref_t<std::ranges::range_value_t<T>>, Point2<i32>> constexpr auto ConvexDiameter(T&& p) {
  struct result_type {
    Point2<i32> a, b;
    constexpr auto distance() const noexcept { return Norm<f64>(a - b); }
    constexpr const auto& first() const noexcept { return a; }
    constexpr const auto& second() const noexcept { return b; }
  };
  const u32 n = std::ranges::size(p);
  if(n == 0) throw Exception("gsh::ConvexDiameter / Input is empty.");
  const auto bg = std::ranges::begin(p);
  if(n <= 2) {
    if(n == 1) return result_type{*bg, *bg};
    else return result_type{*bg, *std::ranges::next(bg)};
  }
  u32 is = 0, js = 0;
  for(u32 i = 1; i != n; i++) {
    auto a = std::ranges::next(bg, i)->y, b = std::ranges::next(bg, is)->y, c = std::ranges::next(bg, js)->y;
    is = (a > b ? i : is);
    js = (a < c ? i : js);
  }
  i64 maxdis = NormSquare<i64>(*std::ranges::next(bg, is) - *std::ranges::next(bg, js));
  u32 maxi = is, maxj = js, i = is, j = js;
  do {
    const u32 in = (i + 1 == n ? 0 : i + 1), jn = (j + 1 == n ? 0 : j + 1);
    const bool f = Cross<i64>(*std::ranges::next(bg, in) - *std::ranges::next(bg, i), *std::ranges::next(bg, jn) - *std::ranges::next(bg, j)) > 0;
    j = f ? jn : j;
    i = f ? i : in;
    const i64 tmp = NormSquare<i64>(*std::ranges::next(bg, i) - *std::ranges::next(bg, j));
    const bool g = tmp > maxdis;
    maxdis = g ? tmp : maxdis;
    maxi = g ? i : maxi;
    maxj = g ? j : maxj;
  } while(i != is || j != js);
  return result_type{*std::ranges::next(bg, maxi), *std::ranges::next(bg, maxj)};
}
template<std::ranges::range T> auto FurthestPair(T&& r) { return ConvexDiameter(ConvexHull(r)); }
}
