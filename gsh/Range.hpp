#pragma once
#include "Exception.hpp"
#include "Functional.hpp"
#include "Memory.hpp"
#include "TypeDef.hpp"
#include "internal/Operation.hpp"
#include "internal/VecFwd.hpp"
#include <algorithm>
#include <cctype>
#include <ranges>
#include <set>
namespace gsh {
enum class RangeKind { Sized, Unsized };
namespace internal {
template<class F, class T, class I, class U> concept IndirectlyBinaryLeftFoldableImpl = std::movable<T> && std::movable<U> && std::convertible_to<T, U> && std::invocable<F&, U, std::iter_reference_t<I>> && std::assignable_from<U&, std::invoke_result_t<F&, U, std::iter_reference_t<I>>>;
template<class F, class T, class I> concept IndirectlyBinaryLeftFoldable = std::copy_constructible<F> && std::indirectly_readable<I> && std::invocable<F&, T, std::iter_reference_t<I>> && std::convertible_to<std::invoke_result_t<F&, T, std::iter_reference_t<I>>, std::decay_t<std::invoke_result_t<F&, T, std::iter_reference_t<I>>>> && IndirectlyBinaryLeftFoldableImpl<F, T, I, std::decay_t<std::invoke_result_t<F&, T, std::iter_reference_t<I>>>>;
// These functions are defined in gsh/Algorithm.hpp
template<class R, class Comp, class Proj> constexpr auto MinImpl(R&& r, Comp&& comp, Proj&& proj);
template<class R, class Comp, class Proj> constexpr auto MaxImpl(R&& r, Comp&& comp, Proj&& proj);
template<class R, class T, class F> constexpr auto FoldImpl(R&& r, T init, F&& f);
template<class R, class F> constexpr auto SumImpl(R&& r, F&& f);
template<class R, class F> constexpr void AdjacentDifferenceImpl(R&& r, F&& f);
template<class R> constexpr void ReverseImpl(R&& r);
template<class R, class Comp, class Proj> constexpr void SortImpl(R&& r, Comp&& comp, Proj&& proj);
template<class R, class Comp, class Proj> constexpr auto SortIndexImpl(R&& r, Comp&& comp, Proj&& proj);
template<class R, class Comp, class Proj> constexpr auto OrderImpl(R&& r, Comp&& comp, Proj&& proj);
template<class R, class Comp, class Proj> constexpr auto IsSortedImpl(R&& r, Comp&& comp, Proj&& proj);
template<class R, class Comp, class Proj> constexpr auto IsSortedUntilImpl(R&& r, Comp&& comp, Proj&& proj);
template<class Iter, class Sent, class T, class Proj, class Comp> constexpr auto LowerBoundImpl(Iter first, Sent last, const T& value, Comp&& comp, Proj&& proj);
template<class Iter, class Sent, class T, class Proj, class Comp> constexpr auto UpperBoundImpl(Iter first, Sent last, const T& value, Comp&& comp, Proj&& proj);
template<class R, class Equal> constexpr auto IsPalindromeImpl(R&& r, Equal&& equal);
// These functions are defined in gsh/Numeric.hpp
template<class R, class Proj> constexpr auto GCDImpl(R&& r, Proj&& proj);
template<class R, class Proj> constexpr auto LCMImpl(R&& r, Proj&& proj);
} // namespace internal
template<std::input_or_output_iterator I, std::sentinel_for<I> S, RangeKind K> requires (K == RangeKind::Sized || !std::sized_sentinel_for<S, I>) class Subrange;
template<class D, class V> requires std::is_class_v<D> && std::same_as<D, std::remove_cv_t<D>> class ViewInterface {
  constexpr D& derived() noexcept { return *static_cast<D*>(this); }
  constexpr const D& derived() const noexcept { return *static_cast<const D*>(this); }
  constexpr u32 get_size() const noexcept { return std::ranges::size(derived()); }
  GSH_INTERNAL_INLINE constexpr void check_index(u32 n) const {
    u32 sz = get_size();
    if(n >= sz) [[unlikely]]
      throw Exception("gsh::ViewInterface::check_index / The index is out of range. ( n=", n, ", size=", sz, " )");
  }
  GSH_INTERNAL_INLINE constexpr void check_index_on_debug(u32 n) const {
#ifndef NDEBUG
    check_index(n);
#else
    Assume(n < get_size());
#endif
  }
  using derived_type = D;
  using vec_type = Vec<V, std::allocator<V>>;
public:
  using value_type = V;
  using reference = V&;
  using const_reference = const V&;
  using pointer = V*;
  using const_pointer = const V*;
  using size_type = u32;
  using difference_type = i32;
  constexpr auto begin() { return std::ranges::begin(derived()); }
  constexpr auto begin() const { return std::ranges::begin(derived()); }
  constexpr auto end() { return std::ranges::end(derived()); }
  constexpr auto end() const { return std::ranges::end(derived()); }
  constexpr auto rbegin() { return std::reverse_iterator(end()); }
  constexpr auto rbegin() const { return std::reverse_iterator(cend()); }
  constexpr auto rend() { return std::reverse_iterator(begin()); }
  constexpr auto rend() const { return std::reverse_iterator(cbegin()); }
  constexpr auto cbegin() const { return std::ranges::cbegin(derived()); }
  constexpr auto cend() const { return std::ranges::cend(derived()); }
  constexpr auto crbegin() const { return std::reverse_iterator(cend()); }
  constexpr auto crend() const { return std::reverse_iterator(cbegin()); }
  constexpr reference front() { return *begin(); }
  constexpr const_reference front() const { return *cbegin(); }
  constexpr reference back() { return *rbegin(); }
  constexpr const_reference back() const { return *rbegin(); }
  constexpr reference operator[](u32 n) {
    check_index_on_debug(n);
    return *std::ranges::next(begin(), n);
  }
  constexpr const_reference operator[](u32 n) const {
    check_index_on_debug(n);
    return *std::ranges::next(begin(), n);
  }
  constexpr reference at(u32 n) {
    check_index(n);
    return *std::ranges::next(begin(), n);
  }
  constexpr const_reference at(u32 n) const {
    check_index(n);
    return *std::ranges::next(begin(), n);
  }
  template<class Iter, std::sentinel_for<Iter> Sent> constexpr void assign(Iter first, Sent last) { derived() = derived_type(std::move(first), std::move(last)); }
  template<std::ranges::input_range R, class... Args> static constexpr derived_type from_range(R&& r, Args&&... args) { return derived_type(std::ranges::begin(r), std::ranges::end(r), std::forward<Args>(args)...); }
  template<std::ranges::input_range R> constexpr void assign_range(R&& r) { derived().assign(std::ranges::begin(r), std::ranges::end(r)); }
  constexpr void assign_range(std::initializer_list<value_type> init) { derived().assign(init); }
  constexpr derived_type copy() const& { return derived(); }
  constexpr derived_type copy() & { return derived(); }
  constexpr derived_type copy() && { return std::move(derived()); }
  constexpr vec_type as_vec() const { return vec_type::from_range(derived()); };
  constexpr vec_type as_vec() { return vec_type::from_range(derived()); };
  constexpr auto as_set() const { return std::set(begin(), end()); }
  constexpr auto as_set() { return std::set(begin(), end()); }
  constexpr auto as_multiset() const { return std::multiset(begin(), end()); }
  constexpr auto as_multiset() { return std::multiset(begin(), end()); }
  constexpr auto slice() { return Subrange(begin(), end()); }
  constexpr auto slice(u32 start) { return Subrange(std::ranges::next(begin(), start), end()); }
  constexpr auto slice(u32 start, u32 end) { return Subrange(std::ranges::next(begin(), start), std::ranges::next(begin(), end)); }
  constexpr auto slice() const { return Subrange(begin(), end()); }
  constexpr auto slice(u32 start) const { return Subrange(std::ranges::next(begin(), start), end()); }
  constexpr auto slice(u32 start, u32 end) const { return Subrange(std::ranges::next(begin(), start), std::ranges::next(begin(), end)); }
  constexpr auto drop(u32 n) { return slice(n); }
  constexpr auto drop(u32 n) const { return slice(n); }
  constexpr auto take(u32 n) { return Subrange(begin(), std::ranges::next(begin(), n)); }
  constexpr auto take(u32 n) const { return Subrange(begin(), std::ranges::next(begin(), n)); }
  template<class T> static constexpr derived_type iota(T&& end) { return from_range(std::views::iota(std::decay_t<T>(), end)); }
  template<class T, class U> static constexpr derived_type iota(T&& start, U&& end) {
    using ct = std::common_type_t<T, U>;
    return from_range(std::views::iota(static_cast<ct>(start), static_cast<ct>(end)));
  }
  template<class Proj> requires std::ranges::input_range<derived_type> constexpr void iterate(Proj&& proj = {}) {
    auto itr = begin();
    auto sent = end();
    for(u32 idx = 0; itr != sent; ++itr, ++idx) {
      if constexpr(requires { std::invoke(proj, *itr, idx, *this); }) std::invoke(proj, *itr, idx, *this);
      else if constexpr(requires { std::invoke(proj, *itr, idx); }) std::invoke(proj, *itr, idx);
      else if constexpr(requires { std::invoke(proj, *itr); }) std::invoke(proj, *itr);
      else proj();
    }
  }
  template<class Proj> requires std::ranges::input_range<derived_type> constexpr void iterate(Proj&& proj = {}) const {
    auto itr = begin();
    auto sent = end();
    for(u32 idx = 0; itr != sent; ++itr, ++idx) {
      if constexpr(requires { std::invoke(proj, *itr, idx, *this); }) std::invoke(proj, *itr, idx, *this);
      else if constexpr(requires { std::invoke(proj, *itr, idx); }) std::invoke(proj, *itr, idx);
      else if constexpr(requires { std::invoke(proj, *itr); }) std::invoke(proj, *itr);
      else std::invoke(proj);
    }
  }
  template<std::indirect_unary_predicate<std::ranges::iterator_t<derived_type>> Pred> requires std::ranges::input_range<derived_type> [[nodiscard]] constexpr auto filter_view(Pred&& pred = {}) const { return derived() | std::views::filter(std::forward<Pred>(pred)); }
  template<std::invocable<value_type> Proj> requires std::ranges::input_range<derived_type> [[nodiscard]] constexpr auto transform_view(Proj&& proj = {}) const { return derived() | std::views::transform(std::forward<Proj>(proj)); }
  template<std::indirect_unary_predicate<std::ranges::iterator_t<derived_type>> Pred> requires std::ranges::input_range<derived_type> [[nodiscard]] constexpr auto erase_if(Pred&& pred = {}) {
    auto view = derived().filter([&](auto&& x) { return !std::invoke(pred, x); });
    derived() = derived_type(std::ranges::begin(view), std::ranges::end(view));
  }
  template<std::invocable<value_type> Proj> requires std::ranges::input_range<derived_type> && std::ranges::output_range<derived_type, std::invoke_result_t<Proj, value_type&&>> constexpr void apply(Proj&& proj = {}) {
    for(auto&& x : derived()) x = std::invoke(proj, std::move(x));
  }
  template<class T> requires std::ranges::output_range<derived_type, const T&> constexpr void fill(const T& x) {
    for(auto& el : derived()) el = x;
  }
  /*
  template<class Proj = Identity, std::indirect_equivalence_relation<std::projected<std::ranges::iterator_t<derived_type>, Proj>> Comp = EqualTo> requires std::ranges::forward_range<derived_type> && std::permutable<std::ranges::iterator_t<derived_type>> constexpr void unique(Comp&& comp = {}, Proj&& proj = {}) {
    const auto sr = std::ranges::unique(begin(), end(), std::forward<Comp>(comp), std::forward<Proj>(proj));
    if constexpr(requires { derived().erase(sr.begin(), sr.end()); }) derived().erase(sr.begin(), sr.end());
    else derived() = derived_type(begin(), sr.begin());
  }
  */
  template<std::predicate<value_type> Pred> requires std::ranges::input_range<derived_type> constexpr bool all_of(Pred&& f = {}) const {
    for(auto&& el : derived())
      if(!std::invoke(f, std::forward<decltype(el)>(el))) return false;
    return true;
  }
  template<std::predicate<value_type> Pred> requires std::ranges::input_range<derived_type> constexpr bool any_of(Pred&& f = {}) const {
    for(auto&& el : derived())
      if(std::invoke(f, std::forward<decltype(el)>(el))) return true;
    return false;
  }
  template<std::predicate<value_type> Pred> requires std::ranges::input_range<derived_type> constexpr bool none_of(Pred f) const {
    for(auto&& el : derived())
      if(std::invoke(f, std::forward<decltype(el)>(el))) return false;
    return true;
  }
  template<class T, class Proj = Identity, std::indirect_equivalence_relation<std::projected<std::ranges::iterator_t<derived_type>, Proj>> Comp = EqualTo> requires std::ranges::input_range<derived_type> constexpr bool contains(const T& x, Comp&& comp = {}, Proj&& proj = {}) const {
    for(auto&& el : derived())
      if(std::invoke(comp, x, std::invoke(proj, std::forward<decltype(el)>(el)))) return true;
    return false;
  }
  template<class T, class Proj = Identity, std::indirect_equivalence_relation<std::projected<std::ranges::iterator_t<derived_type>, Proj>> Comp = EqualTo> requires std::ranges::input_range<derived_type> constexpr auto find(const T& x, Comp&& comp = {}, Proj&& proj = {}) const {
    const auto sent = end();
    auto itr = begin();
    for(; itr != sent; ++itr)
      if(std::invoke(comp, x, std::invoke(proj, *itr))) return itr;
    return itr;
  }
  template<class T, class Proj = Identity, std::indirect_equivalence_relation<std::projected<std::ranges::iterator_t<derived_type>, Proj>> Comp = EqualTo> requires std::ranges::input_range<derived_type> constexpr auto rfind(const T& x, Comp&& comp = {}, Proj&& proj = {}) const {
    if constexpr(std::ranges::bidirectional_range<derived_type>) {
      const auto sent = rend();
      auto itr = rbegin();
      for(; itr != sent; ++itr)
        if(std::invoke(comp, x, std::invoke(proj, *itr))) return --itr.base();
      return end();
    } else {
      const auto sent = end();
      auto itr = begin();
      decltype(itr) result = itr;
      bool found = false;
      for(; itr != sent; ++itr) {
        if(std::invoke(comp, x, std::invoke(proj, *itr))) {
          result = itr;
          found = true;
        }
      }
      if(!found) result = itr;
      return result;
    }
  }
  template<class T, class Proj = Identity, std::indirect_equivalence_relation<std::projected<std::ranges::iterator_t<derived_type>, Proj>> Comp = EqualTo> requires std::ranges::input_range<derived_type> constexpr u32 index_of(const T& x, Comp&& comp = {}, Proj&& proj = {}) const {
    const auto sent = end();
    u32 cnt = 0;
    for(auto itr = begin(); itr != sent; ++itr) {
      if(std::invoke(comp, x, std::invoke(proj, *itr))) return cnt;
      ++cnt;
    }
    return cnt;
  }
  template<class T, class Proj = Identity, std::indirect_equivalence_relation<std::projected<std::ranges::iterator_t<derived_type>, Proj>> Comp = EqualTo> requires std::ranges::input_range<derived_type> constexpr u32 count(const T& x, Comp&& comp = {}, Proj&& proj = {}) const {
    u32 res = 0;
    for(auto&& el : derived()) res += static_cast<bool>(std::invoke(comp, x, std::invoke(proj, std::forward<decltype(el)>(el))));
    return res;
  }
  template<class Proj = Identity, std::indirect_strict_weak_order<std::projected<std::ranges::iterator_t<derived_type>, Proj>> Comp = Less> requires std::ranges::forward_range<derived_type> && std::indirectly_copyable_storable<std::ranges::iterator_t<derived_type>, value_type*> constexpr auto min(Comp&& comp = {}, Proj&& proj = {}) const { return internal::MinImpl(derived(), std::forward<Comp>(comp), std::forward<Proj>(proj)); }
  template<class Proj = Identity, std::indirect_strict_weak_order<std::projected<std::ranges::iterator_t<derived_type>, Proj>> Comp = Less> requires std::ranges::forward_range<derived_type> && std::indirectly_copyable_storable<std::ranges::iterator_t<derived_type>, value_type*> constexpr auto max(Comp&& comp = {}, Proj&& proj = {}) const { return internal::MaxImpl(derived(), std::forward<Comp>(comp), std::forward<Proj>(proj)); }
  template<class T = value_type, internal::IndirectlyBinaryLeftFoldable<T, std::ranges::iterator_t<derived_type>> F = Plus> requires std::ranges::forward_range<derived_type> constexpr auto fold(T&& init = {}, F&& f = {}) const { return internal::FoldImpl(derived(), std::forward<T>(init), std::forward<F>(f)); }
  template<internal::IndirectlyBinaryLeftFoldable<value_type, std::ranges::iterator_t<derived_type>> F = Plus> requires std::ranges::forward_range<derived_type> constexpr auto sum(F&& f = {}) const { return internal::SumImpl(derived(), std::forward<F>(f)); }
  template<class F = Minus> requires std::ranges::forward_range<derived_type> && std::indirectly_writable<std::ranges::iterator_t<derived_type>, std::invoke_result_t<F, value_type, value_type>> constexpr void adjacent_difference(F&& f = {}) { internal::AdjacentDifferenceImpl(derived(), std::forward<F>(f)); }
  template<class F = Minus> constexpr auto adjacent_differenced(F&& f = {}) {
    auto res = derived();
    internal::AdjacentDifferenceImpl(res, std::forward<F>(f));
    return res;
  }
  void reverse() requires std::ranges::bidirectional_range<derived_type> && std::permutable<std::ranges::iterator_t<derived_type>> { internal::ReverseImpl(derived()); }
  [[nodiscard]] auto reversed() const requires std::permutable<std::ranges::iterator_t<vec_type>> {
    auto res = as_vec();
    internal::ReverseImpl(res);
    return res;
  }
  [[nodiscard]] auto reversed_view() const requires std::ranges::view<derived_type> && std::ranges::bidirectional_range<derived_type> { return derived() | std::views::reverse; }
  template<class Proj = Identity, class Comp = Less> requires std::ranges::forward_range<derived_type> && std::sortable<std::ranges::iterator_t<derived_type>, Comp, Proj> constexpr void sort(Comp&& comp = {}, Proj&& proj = {}) { internal::SortImpl(derived(), std::forward<Comp>(comp), std::forward<Proj>(proj)); }
  template<class Proj = Identity, class Comp = Less> requires std::sortable<std::ranges::iterator_t<vec_type>, Comp, Proj> [[nodiscard]] constexpr auto sorted(Comp&& comp = {}, Proj&& proj = {}) const {
    auto res = as_vec();
    internal::SortImpl(res, std::forward<Comp>(comp), std::forward<Proj>(proj));
    return res;
  }
  template<class Proj = Identity, class Comp = Less> requires std::ranges::random_access_range<derived_type> && std::sortable<std::ranges::iterator_t<derived_type>, Comp, Proj> constexpr auto sort_index(Comp&& comp = {}, Proj&& proj = {}) const { return internal::SortIndexImpl(derived(), std::forward<Comp>(comp), std::forward<Proj>(proj)); }
  template<class Proj = Identity, class Comp = Less> requires std::ranges::random_access_range<derived_type> && std::sortable<std::ranges::iterator_t<derived_type>, Comp, Proj> constexpr auto order(Comp&& comp = {}, Proj&& proj = {}) const { return internal::OrderImpl(derived(), std::forward<Comp>(comp), std::forward<Proj>(proj)); }
  template<class Proj = Identity, class Comp = Less, class R> requires std::ranges::random_access_range<derived_type> && std::ranges::forward_range<R> constexpr void sort_with(R&& r, Comp&& comp = {}, Proj&& proj = {}) {
#ifndef NDEBUG
    if(std::ranges::size(r) != std::ranges::size(derived())) { throw Exception("gsh::ViewInterface::sort_with / The size of the range passed as an argument differs from the size of the key."); }
#endif
    auto ord = derived().sort_index(std::forward<Comp>(comp), std::forward<Proj>(proj)).inverse();
    derived().permute(ord);
    Subrange{std::ranges::begin(r), std::ranges::end(r)}.permute(ord);
  };
  template<class Proj = Identity, class Comp = Less, class... Rs> requires std::ranges::random_access_range<derived_type> && (std::ranges::forward_range<Rs> && ...) constexpr void sort_with(std::tuple<Rs...> rs, Comp&& comp = {}, Proj&& proj = {}) {
#ifndef NDEBUG
    u32 sz = std::ranges::size(derived());
    auto check = [&](auto&&... r) {
      if((... || (std::ranges::size(r) != sz))) { throw Exception("gsh::ViewInterface::sort_with / The size of the range passed as an argument differs from the size of the key."); }
    };
    std::apply(check, rs);
#endif
    auto ord = derived().sort_index(std::forward<Comp>(comp), std::forward<Proj>(proj)).inverse();
    derived().permute(ord);
    std::apply([&](auto&&... r) { (Subrange{std::ranges::begin(r), std::ranges::end(r)}.permute(ord), ...); }, rs);
  };
  template<class Proj = Identity, std::indirect_strict_weak_order<std::projected<std::ranges::iterator_t<derived_type>, Proj>> Comp = Less> requires std::ranges::forward_range<derived_type> constexpr auto is_sorted(Comp&& comp = {}, Proj&& proj = {}) const { return internal::IsSortedImpl(derived(), std::forward<Comp>(comp), std::forward<Proj>(proj)); }
  template<class Proj = Identity, std::indirect_strict_weak_order<std::projected<std::ranges::iterator_t<derived_type>, Proj>> Comp = Less> requires std::ranges::forward_range<derived_type> constexpr auto is_sorted_until(Comp&& comp = {}, Proj&& proj = {}) const { return internal::IsSortedUntilImpl(derived(), std::forward<Comp>(comp), std::forward<Proj>(proj)); }
  template<class T, class Proj = Identity, std::indirect_strict_weak_order<const T*, std::projected<std::ranges::iterator_t<derived_type>, Proj>> Comp = Less> requires std::ranges::forward_range<derived_type> constexpr auto lower_bound(const T& value, Comp&& comp = {}, Proj&& proj = {}) const { return internal::LowerBoundImpl(begin(), end(), value, std::forward<Comp>(comp), std::forward<Proj>(proj)); }
  template<class T, class Proj = Identity, std::indirect_strict_weak_order<const T*, std::projected<std::ranges::iterator_t<derived_type>, Proj>> Comp = Less> requires std::ranges::forward_range<derived_type> constexpr auto lower_bound_index(const T& value, Comp&& comp = {}, Proj&& proj = {}) const { return std::ranges::distance(begin(), lower_bound(value, std::forward<Comp>(comp), std::forward<Proj>(proj))); }
  template<class T, class Proj = Identity, std::indirect_strict_weak_order<const T*, std::projected<std::ranges::iterator_t<derived_type>, Proj>> Comp = Less> requires std::ranges::forward_range<derived_type> constexpr auto upper_bound(const T& value, Comp&& comp = {}, Proj&& proj = {}) const { return internal::UpperBoundImpl(begin(), end(), value, std::forward<Comp>(comp), std::forward<Proj>(proj)); }
  template<class T, class Proj = Identity, std::indirect_strict_weak_order<const T*, std::projected<std::ranges::iterator_t<derived_type>, Proj>> Comp = Less> requires std::ranges::forward_range<derived_type> constexpr auto upper_bound_index(const T& value, Comp&& comp = {}, Proj&& proj = {}) const { return std::ranges::distance(begin(), upper_bound(value, std::forward<Comp>(comp), std::forward<Proj>(proj))); }
  template<std::indirect_binary_predicate<std::ranges::iterator_t<derived_type>, std::ranges::iterator_t<derived_type>> Equal = EqualTo> requires std::ranges::bidirectional_range<derived_type> constexpr auto is_palindrome(Equal&& equal = {}) const { return internal::IsPalindromeImpl(derived(), std::forward<Equal>(equal)); }
  template<class Proj = Identity> requires std::ranges::input_range<derived_type> && std::invocable<Proj, value_type> constexpr auto gcd(Proj&& proj = {}) const { return internal::GCDImpl(derived(), std::forward<Proj>(proj)); }
  template<class Proj = Identity> requires std::ranges::input_range<derived_type> && std::invocable<Proj, value_type> constexpr auto lcm(Proj&& proj = {}) const { return internal::LCMImpl(derived(), std::forward<Proj>(proj)); }
  constexpr bool is_capitalized() const requires std::ranges::input_range<derived_type> && std::same_as<value_type, c8> {
    auto itr = begin();
    auto sent = end();
    if(!(itr != sent) || !std::isupper(*itr)) return false;
    ++itr;
    while(itr != sent) {
      if(!std::islower(*itr)) return false;
      ++itr;
    }
    return true;
  }
  constexpr void lower() requires std::ranges::input_range<derived_type> && std::same_as<value_type, c8> {
    auto itr = begin();
    auto sent = end();
    while(itr != sent) {
      *itr = std::tolower(*itr);
      ++itr;
    }
  }
  constexpr void upper() requires std::ranges::input_range<derived_type> && std::same_as<value_type, c8> {
    auto itr = begin();
    auto sent = end();
    while(itr != sent) {
      *itr = std::toupper(*itr);
      ++itr;
    }
  }
  template<class R> requires std::ranges::forward_range<derived_type> && std::ranges::input_range<R> constexpr void permute(R&& location) {
    auto itr = begin();
    auto sent = end();
    auto loc_begin = std::ranges::begin(location);
    u32 size = std::ranges::distance(itr, sent);
    Mem<V> tmp(size);
    for(u32 i = 0; i != size; ++i) std::construct_at(&tmp[*(loc_begin++)], std::move(*(itr++)));
    itr = begin();
    for(u32 i = 0; i != size; ++i) *(itr++) = std::move(tmp[i]);
  }
  constexpr auto inverse() requires std::ranges::random_access_range<derived_type> {
    auto itr = begin();
    auto sent = end();
    u32 size = std::ranges::distance(itr, sent);
    vec_type tmp(size);
    for(auto i = static_cast<value_type>(0); i != size; ++i) std::construct_at(&tmp[*(itr++)], i);
    return tmp;
  }
};
template<std::ranges::forward_range T, std::ranges::forward_range U> requires (std::derived_from<T, ViewInterface<T, typename T::value_type>> || std::derived_from<U, ViewInterface<U, typename U::value_type>>) auto operator<=>(const T& a, const U& b) { return std::lexicographical_compare_three_way(std::ranges::begin(a), std::ranges::end(a), std::ranges::begin(b), std::ranges::end(b)); }
template<std::ranges::forward_range T, std::ranges::forward_range U> requires (std::derived_from<T, ViewInterface<T, typename T::value_type>> || std::derived_from<U, ViewInterface<U, typename U::value_type>>) bool operator==(const T& a, const U& b) { return std::ranges::equal(std::ranges::begin(a), std::ranges::end(a), std::ranges::begin(b), std::ranges::end(b)); }
namespace internal {
template<class T, class U> concept difference_from = std::same_as<std::remove_cvref_t<T>, std::remove_cvref_t<U>>;
template<class From, class To> concept convertible_to_non_slicing = std::convertible_to<From, To> && !(std::is_pointer_v<std::decay_t<From>> && std::is_pointer_v<std::decay_t<To>> && !std::convertible_to<std::remove_pointer_t<std::decay_t<From>> (*)[], std::remove_pointer_t<std::decay_t<To>> (*)[]>);
template<class T> concept pair_like = /* tuple-like<T> && */ std::tuple_size_v<std::remove_cvref_t<T>> == 2;
template<class T, class U, class V> concept pair_like_convertible_from = !std::ranges::range<T> && !std::is_reference_v<T> && pair_like<T> && std::constructible_from<T, U, V> && convertible_to_non_slicing<U, std::tuple_element_t<0, T>> && std::convertible_to<V, std::tuple_element_t<1, T>>;
} // namespace internal
template<std::input_or_output_iterator I, std::sentinel_for<I> S = I, RangeKind K = std::sized_sentinel_for<S, I> ? RangeKind::Sized : RangeKind::Unsized> requires (K == RangeKind::Sized || !std::sized_sentinel_for<S, I>) class Subrange : public ViewInterface<Subrange<I, S, K>, std::iter_value_t<I>> {
  I itr;
  S sent;
  static constexpr bool StoreSize = (K == RangeKind::Sized && !std::sized_sentinel_for<S, I>);
  struct empty_sz {};
  [[no_unique_address]] std::conditional_t<StoreSize, std::make_unsigned_t<std::iter_difference_t<I>>, empty_sz> sz;
public:
  constexpr Subrange() = default;
  constexpr Subrange(internal::convertible_to_non_slicing<I> auto i, S s) requires (!StoreSize) : itr(i), sent(s) {}
  constexpr Subrange(internal::convertible_to_non_slicing<I> auto i, S s, std::make_unsigned_t<std::iter_difference_t<I>> n) requires (K == RangeKind::Sized) : itr(i), sent(s) {
    if constexpr(StoreSize) sz = n;
  }
  template<internal::difference_from<Subrange> R> requires std::ranges::borrowed_range<R> && internal::convertible_to_non_slicing<std::ranges::iterator_t<R>, I> && std::convertible_to<std::ranges::sentinel_t<R>, S> constexpr Subrange(R&& r) requires (!StoreSize || std::ranges::sized_range<R>) : itr(std::ranges::begin(r)), sent(std::ranges::end(r)) {}
  template<std::ranges::borrowed_range R> requires internal::convertible_to_non_slicing<std::ranges::iterator_t<R>, I> && std::convertible_to<std::ranges::sentinel_t<R>, S> constexpr Subrange(R&& r, std::make_unsigned_t<std::iter_difference_t<I>> n) requires (K == RangeKind::Sized) : Subrange{std::ranges::begin(r), std::ranges::end(r), n} {}
  template<internal::difference_from<Subrange> PairLike> requires internal::pair_like_convertible_from<PairLike, const I&, const S&> constexpr operator PairLike() const { return PairLike(itr, sent); }
  constexpr I begin() const requires std::copyable<I> { return itr; }
  [[nodiscard]] constexpr I begin() requires (!std::copyable<I>) { return std::move(itr); }
  constexpr S end() const { return sent; }
  constexpr bool empty() const { return itr == sent; }
  constexpr u32 size() const requires (K == RangeKind::Sized) { return static_cast<u32>(std::ranges::distance(itr, sent)); }
  constexpr I data() const requires (std::is_pointer_v<I> && std::copyable<I>) { return itr; }
  constexpr I data() const requires (std::is_pointer_v<I> && !std::copyable<I>) { return std::move(itr); }
  constexpr decltype(auto) operator[](const std::iter_difference_t<I>& n) { return *std::ranges::next(begin(), n); }
  constexpr decltype(auto) operator[](const std::iter_difference_t<I>& n) const { return *std::ranges::next(begin(), n); }
  [[nodiscard]] constexpr Subrange next(std::iter_difference_t<I> n = 1) const& requires std::forward_iterator<I> {
    auto tmp = *this;
    tmp.advance(n);
    return tmp;
  }
  [[nodiscard]] constexpr Subrange next(std::iter_difference_t<I> n = 1) && {
    advance(n);
    return std::move(*this);
  }
  [[nodiscard]] constexpr Subrange prev(std::iter_difference_t<I> n = 1) const requires std::bidirectional_iterator<I> {
    auto tmp = *this;
    tmp.advance(-n);
    return tmp;
  }
  constexpr Subrange& advance(std::iter_difference_t<I> n) {
    if constexpr(StoreSize) {
      auto d = n - std::ranges::advance(itr, n, sent);
      if(d >= 0) sz -= static_cast<std::make_unsigned_t<std::remove_cvref_t<decltype(d)>>>(d);
      else sz += static_cast<std::make_unsigned_t<std::remove_cvref_t<decltype(d)>>>(d);
      return *this;
    } else {
      std::ranges::advance(itr, n, sent);
      return *this;
    }
  }
};
template<std::input_or_output_iterator I, std::sentinel_for<I> S> Subrange(I, S) -> Subrange<I, S>;
template<std::input_or_output_iterator I, std::sentinel_for<I> S> Subrange(I, S, std::make_unsigned_t<std::iter_difference_t<I>>) -> Subrange<I, S, RangeKind::Sized>;
template<std::ranges::borrowed_range R> Subrange(R&&) -> Subrange<std::ranges::iterator_t<R>, std::ranges::sentinel_t<R>, (std::ranges::sized_range<R> || std::sized_sentinel_for<std::ranges::sentinel_t<R>, std::ranges::iterator_t<R>>) ? RangeKind::Sized : RangeKind::Unsized>;
template<std::ranges::borrowed_range R> Subrange(R&&, std::make_unsigned_t<std::ranges::range_difference_t<R>>) -> Subrange<std::ranges::iterator_t<R>, std::ranges::sentinel_t<R>, RangeKind::Sized>;
} // namespace gsh
namespace std::ranges { template<class I, class S, gsh::RangeKind K> constexpr bool enable_borrowed_range<gsh::Subrange<I, S, K>> = true; }
