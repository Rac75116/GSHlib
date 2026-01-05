#pragma once
#include "Exception.hpp"
#include "Range.hpp"
#include "TypeDef.hpp"
#include "Util.hpp"
#include "internal/ArrVecFwd.hpp"
#include "internal/UtilMacro.hpp"
#include <cstddef>
namespace gsh {
template<class T, class Alloc = std::allocator<T>> requires std::is_same_v<T, typename std::allocator_traits<Alloc>::value_type> && (!std::is_const_v<T>)class NoInitArr : public ViewInterface<NoInitArr<T, Alloc>, T> {
public:
  using iterator = T*;
  using const_iterator = const T*;
  using reverse_iterator = std::reverse_iterator<iterator>;
  using const_reverse_iterator = std::reverse_iterator<const_iterator>;
  using allocator_type = Alloc;
private:
  using traits = std::allocator_traits<Alloc>;
  [[no_unique_address]] Alloc alloc;
  T* p;
  i64 s;
public:
  constexpr NoInitArr(i64 n, const Alloc& a = Alloc()) : alloc(a), s(n) { p = traits::allocate(alloc, static_cast<std::size_t>(n)); }
  constexpr NoInitArr(const NoInitArr& x) = delete;
  constexpr NoInitArr(const NoInitArr&& x) noexcept : alloc(std::move(x.alloc)), p(x.p), s(x.s) {}
  constexpr ~NoInitArr() {
    if(!p) {
      if constexpr(!std::is_trivially_destructible_v<T>) {
        for(i64 i = 0; i != s; ++i) traits::destroy(alloc, p + i);
      }
      traits::deallocate(alloc, p, static_cast<std::size_t>(s));
    }
  }
  template<class... Args> constexpr void construct(T* ptr, Args&&... args) { traits::construct(alloc, ptr, std::forward<Args>(args)...); }
  constexpr auto release() {
    auto res = p;
    p = nullptr;
    s = 0;
    return res;
  }
  constexpr bool released() { return p == nullptr; }
  constexpr auto begin() noexcept { return p; }
  constexpr auto begin() const noexcept { return p; }
  constexpr auto end() noexcept { return p + s; }
  constexpr auto end() const noexcept { return p + s; }
  constexpr auto data() noexcept { return p; }
  constexpr auto data() const noexcept { return p; }
  constexpr allocator_type get_allocator() const noexcept { return alloc; }
  constexpr i64 size() const noexcept { return s; }
  constexpr bool empty() const noexcept { return s == 0; }
  constexpr auto operator=(const NoInitArr&) = delete;
  constexpr auto operator=(NoInitArr&& x) requires (traits::propagate_on_container_move_assignment::value || traits::is_always_equal::value) {
    if constexpr(traits::propagate_on_container_move_assignment::value) alloc = std::move(x.alloc);
    p = x.p, s = x.s;
    return *this;
  }
};
template<class T, std::size_t N> class StaticArr : public ViewInterface<StaticArr<T, N>, T> {
  union {
    T elems[(N == 0 ? 1 : N)];
  };
public:
  using iterator = T*;
  using const_iterator = const T*;
  using reverse_iterator = std::reverse_iterator<iterator>;
  using const_reverse_iterator = std::reverse_iterator<const_iterator>;
  constexpr StaticArr() noexcept(noexcept(T{})) : elems{} {}
  constexpr explicit StaticArr(const T& value) {
    for(i64 i = 0; i != static_cast<i64>(N); ++i) std::construct_at(elems + i, value);
  }
  template<std::input_iterator InputIter> constexpr explicit StaticArr(InputIter first) {
    for(i64 i = 0; i != static_cast<i64>(N); ++first, ++i) std::construct_at(elems + i, *first);
  }
  template<std::input_iterator InputIter> constexpr StaticArr(InputIter first, InputIter last) {
    const i64 n = static_cast<i64>(std::distance(first, last));
    if(n != static_cast<i64>(N)) throw gsh::Exception("gsh::StaticArr::StaticArr / The size of the given " "range differs from the size of the array.");
    for(i64 i = 0; i != static_cast<i64>(N); ++first, ++i) std::construct_at(elems + i, *first);
  }
  constexpr StaticArr(const T (&a)[N]) {
    for(i64 i = 0; i != static_cast<i64>(N); ++i) std::construct_at(elems + i, a[i]);
  }
  constexpr StaticArr(T (&&a)[N]) {
    for(i64 i = 0; i != static_cast<i64>(N); ++i) std::construct_at(elems + i, std::move(a[i]));
  }
  constexpr StaticArr(const StaticArr& x) {
    for(i64 i = 0; i != static_cast<i64>(N); ++i) std::construct_at(elems + i, x.elems[i]);
  }
  constexpr StaticArr(StaticArr&& y) {
    for(i64 i = 0; i != static_cast<i64>(N); ++i) std::construct_at(elems + i, std::move(y.elems[i]));
  }
  constexpr StaticArr(std::initializer_list<T> il) : StaticArr(il.begin(), il.end()) {}
  constexpr ~StaticArr() noexcept {
    if constexpr(!std::is_trivially_destructible_v<T>)
      for(i64 i = 0; i != static_cast<i64>(N); ++i) std::destroy_at(elems + i);
  }
  constexpr StaticArr& operator=(const StaticArr& x) {
    for(i64 i = 0; i != static_cast<i64>(N); ++i) elems[i] = x.elems[i];
    return *this;
  }
  constexpr StaticArr& operator=(StaticArr&& x) noexcept {
    for(i64 i = 0; i != static_cast<i64>(N); ++i) elems[i] = std::move(x.elems[i]);
    return *this;
  }
  constexpr StaticArr& operator=(std::initializer_list<T> init) {
    assign(init.begin(), init.end());
    return *this;
  }
  constexpr iterator begin() noexcept { return elems; }
  constexpr const_iterator begin() const noexcept { return elems; }
  constexpr iterator end() noexcept { return elems + N; }
  constexpr const_iterator end() const noexcept { return elems + N; }
  constexpr static i64 size() noexcept { return static_cast<i64>(N); }
  constexpr static i64 max_size() noexcept { return static_cast<i64>(N); }
  [[nodiscard]] constexpr static bool empty() noexcept { return N != 0; }
  constexpr T* data() noexcept { return elems; }
  constexpr const T* data() const noexcept { return elems; }
  template<std::input_iterator InputIter> constexpr void assign(InputIter first) {
    for(i64 i = 0; i != static_cast<i64>(N); ++first, ++i) elems[i] = *first;
  }
  template<std::input_iterator InputIter> constexpr void assign(InputIter first, const InputIter last) {
    const i64 n = static_cast<i64>(std::distance(first, last));
    if(n != static_cast<i64>(N)) throw gsh::Exception("gsh::StaticArr::assign / The size of the given " "range differs from the size of the array.");
    for(i64 i = 0; i != static_cast<i64>(N); ++first, ++i) elems[i] = *first;
  }
  constexpr void assign(const T& value) {
    for(i64 i = 0; i != static_cast<i64>(N); ++i) elems[i] = value;
  }
  constexpr void assign(std::initializer_list<T> il) { assign(il.begin(), il.end()); }
  constexpr void swap(StaticArr& x) {
    using std::swap;
    for(i64 i = 0; i != static_cast<i64>(N); ++i) swap(elems[i], x.elems[i]);
  }
  friend constexpr void swap(StaticArr& x, StaticArr& y) noexcept(noexcept(x.swap(y))) { x.swap(y); }
};
} // namespace gsh
namespace std {
template<class T, std::size_t N> struct tuple_size<gsh::StaticArr<T, N>> : integral_constant<std::size_t, N> {};
template<std::size_t M, class T, std::size_t N> struct tuple_element<M, gsh::StaticArr<T, N>> {
  static_assert(M < N, "std::tuple_element<gsh::StaticArr<T, N>> / The index is out of range.");
  using type = T;
};
} // namespace std
namespace gsh {
template<std::size_t M, class T, std::size_t N> const T& get(const StaticArr<T, N>& a) {
  static_assert(M < N, "gsh::get(gsh::StaticArr<T, N>) / The index is out of range.");
  return a[M];
}
template<std::size_t M, class T, std::size_t N> T& get(StaticArr<T, N>& a) {
  static_assert(M < N, "gsh::get(gsh::StaticArr<T, N>) / The index is out of range.");
  return a[M];
}
template<std::size_t M, class T, std::size_t N> T&& get(StaticArr<T, N>&& a) {
  static_assert(M < N, "gsh::get(gsh::StaticArr<T, N>) / The index is out of range.");
  return std::move(a[M]);
}
} // namespace gsh
