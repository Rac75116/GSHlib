#pragma once
#include "Exception.hpp"
#include "Range.hpp"
#include "TypeDef.hpp"
#include "Util.hpp"
#include "internal/ArrVecFwd.hpp"
#include "internal/UtilMacro.hpp"
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
  u32 s;
public:
  constexpr NoInitArr(u32 n, const Alloc& a = Alloc()) : alloc(a), s(n) { p = traits::allocate(alloc, n); }
  constexpr NoInitArr(const NoInitArr& x) = delete;
  constexpr NoInitArr(const NoInitArr&& x) noexcept : alloc(std::move(x.alloc)), p(x.p), s(x.s) {}
  constexpr ~NoInitArr() {
    if(!p) {
      if constexpr(!std::is_trivially_destructible_v<T>) {
        for(u32 i = 0; i != s; ++i) traits::destroy(alloc, p + i);
      }
      traits::deallocate(alloc, p, s);
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
  constexpr u32 size() const noexcept { return s; }
  constexpr bool empty() const noexcept { return s == 0; }
  constexpr auto operator=(const NoInitArr&) = delete;
  constexpr auto operator=(NoInitArr&& x) requires (traits::propagate_on_container_move_assignment::value || traits::is_always_equal::value) {
    if constexpr(traits::propagate_on_container_move_assignment::value) alloc = std::move(x.alloc);
    p = x.p, s = x.s;
    return *this;
  }
};
template<class T, u32 N> class StaticArr : public ViewInterface<StaticArr<T, N>, T> {
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
    for(u32 i = 0; i != N; ++i) std::construct_at(elems + i, value);
  }
  template<std::input_iterator InputIter> constexpr explicit StaticArr(InputIter first) {
    for(u32 i = 0; i != N; ++first, ++i) std::construct_at(elems + i, *first);
  }
  template<std::input_iterator InputIter> constexpr StaticArr(InputIter first, InputIter last) {
    const u32 n = std::distance(first, last);
    if(n != N) throw gsh::Exception("gsh::StaticArr::StaticArr / The size of the given " "range differs from the size of the array.");
    for(u32 i = 0; i != N; ++first, ++i) std::construct_at(elems + i, *first);
  }
  constexpr StaticArr(const T (&a)[N]) {
    for(u32 i = 0; i != N; ++i) std::construct_at(elems + i, a[i]);
  }
  constexpr StaticArr(T (&&a)[N]) {
    for(u32 i = 0; i != N; ++i) std::construct_at(elems + i, std::move(a[i]));
  }
  constexpr StaticArr(const StaticArr& x) {
    for(u32 i = 0; i != N; ++i) std::construct_at(elems + i, x.elems[i]);
  }
  constexpr StaticArr(StaticArr&& y) {
    for(u32 i = 0; i != N; ++i) std::construct_at(elems + i, std::move(y.elems[i]));
  }
  constexpr StaticArr(std::initializer_list<T> il) : StaticArr(il.begin(), il.end()) {}
  constexpr ~StaticArr() noexcept {
    if constexpr(!std::is_trivially_destructible_v<T>)
      for(u32 i = 0; i != N; ++i) std::destroy_at(elems + i);
  }
  constexpr StaticArr& operator=(const StaticArr& x) {
    for(u32 i = 0; i != N; ++i) elems[i] = x.elems[i];
    return *this;
  }
  constexpr StaticArr& operator=(StaticArr&& x) noexcept {
    for(u32 i = 0; i != N; ++i) elems[i] = std::move(x.elems[i]);
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
  constexpr u32 size() const noexcept { return N; }
  constexpr u32 max_size() const noexcept { return N; }
  [[nodiscard]] constexpr bool empty() const noexcept { return N != 0; }
  constexpr T* data() noexcept { return elems; }
  constexpr const T* data() const noexcept { return elems; }
  template<std::input_iterator InputIter> constexpr void assign(InputIter first) {
    for(u32 i = 0; i != N; ++first, ++i) elems[i] = *first;
  }
  template<std::input_iterator InputIter> constexpr void assign(InputIter first, const InputIter last) {
    const u32 n = std::distance(first, last);
    if(n != N) throw gsh::Exception("gsh::StaticArr::assign / The size of the given " "range differs from the size of the array.");
    for(u32 i = 0; i != N; ++first, ++i) elems[i] = *first;
  }
  constexpr void assign(const T& value) {
    for(u32 i = 0; i != N; ++i) elems[i] = value;
  }
  constexpr void assign(std::initializer_list<T> il) { assign(il.begin(), il.end()); }
  constexpr void swap(StaticArr& x) {
    using std::swap;
    for(u32 i = 0; i != N; ++i) swap(elems[i], x.elems[i]);
  }
  friend constexpr bool operator==(const StaticArr& x, const StaticArr& y) {
    bool res = true;
    for(u32 i = 0; i != N;) {
      const bool f = x.elems[i] == y.elems[i];
      res &= f;
      i = f ? i + 1 : N;
    }
    return res;
  }
  friend constexpr auto operator<=>(const StaticArr& x, const StaticArr& y) { return std::lexicographical_compare_three_way(x.begin(), x.end(), y.begin(), y.end()); }
  friend constexpr void swap(StaticArr& x, StaticArr& y) noexcept(noexcept(x.swap(y))) { x.swap(y); }
};
} // namespace gsh
namespace std {
template<class T, gsh::u32 N> struct tuple_size<gsh::StaticArr<T, N>> : integral_constant<size_t, N> {};
template<std::size_t M, class T, gsh::u32 N> struct tuple_element<M, gsh::StaticArr<T, N>> {
  static_assert(M < N, "std::tuple_element<gsh::StaticArr<T, N>> / The index is out of range.");
  using type = T;
};
} // namespace std
namespace gsh {
template<std::size_t M, class T, u32 N> const T& get(const StaticArr<T, N>& a) {
  static_assert(M < N, "gsh::get(gsh::StaticArr<T, N>) / The index is out of range.");
  return a[M];
}
template<std::size_t M, class T, u32 N> T& get(StaticArr<T, N>& a) {
  static_assert(M < N, "gsh::get(gsh::StaticArr<T, N>) / The index is out of range.");
  return a[M];
}
template<std::size_t M, class T, u32 N> T&& get(StaticArr<T, N>&& a) {
  static_assert(M < N, "gsh::get(gsh::StaticArr<T, N>) / The index is out of range.");
  return std::move(a[M]);
}
} // namespace gsh
