#pragma once
#include "Memory.hpp"
#include "Range.hpp"
#include "Vec.hpp"
#include <set>
namespace gsh {
namespace internal {
struct IntervalSetEmptyHook {
  template<class T, class U> constexpr void operator()(T&&, U&&) {}
};
}
template<class T, class Comp = Less, class Alloc = SingleAllocator<std::pair<T, T>>> class IntervalSet : public ViewInterface<IntervalSet<T, Comp, Alloc>, std::pair<T, T>> {
public:
  using value_type = std::pair<T, T>; // [first, second)
  using key_type = value_type;
  using key_compare = Comp;
  using iterator = typename std::set<value_type>::const_iterator;
  using const_iterator = iterator;
  using reverse_iterator = typename std::set<value_type>::const_reverse_iterator;
  using const_reverse_iterator = reverse_iterator;
  using allocator_type = Alloc;
  struct value_compare {
    [[no_unique_address]] mutable Comp comp;
    constexpr value_compare() = default;
    constexpr value_compare(const Comp& comp) : comp(comp) {}
    constexpr bool operator()(const value_type& a, const value_type& b) const { return std::invoke(comp, a.first, b.first); }
    constexpr bool operator()(const value_type& a, const T& b) const { return std::invoke(comp, a.first, b); }
    constexpr bool operator()(const T& a, const value_type& b) const { return std::invoke(comp, a, b.first); }
    constexpr bool operator()(const T& a, const T& b) const { return std::invoke(comp, a, b); }
  };
  std::set<value_type, value_compare, Alloc> s;
  using empty_hook = internal::IntervalSetEmptyHook;
  template<class H> constexpr static bool is_empty_hook = std::is_same_v<std::remove_cvref_t<H>, internal::IntervalSetEmptyHook>;
public:
  IntervalSet() = default;
  explicit IntervalSet(const Comp& comp, const Alloc& alloc = Alloc()) : s(value_compare(comp), alloc) {}
  explicit IntervalSet(const Alloc& alloc) : s(alloc) {}
  template<class Iter> IntervalSet(Iter first, Iter last, const Comp& comp = Comp(), const Alloc& alloc = Alloc());
  template<class Iter> IntervalSet(Iter first, Iter last, const Alloc& alloc) : IntervalSet(first, last, Comp(), alloc) {}
  IntervalSet(const IntervalSet&) = default;
  IntervalSet(IntervalSet&&) = default;
  IntervalSet(const IntervalSet& x, const Alloc& alloc) : s(x.s, alloc) {}
  IntervalSet(IntervalSet&& x, const Alloc& alloc) : s(std::move(x.s), alloc) {}
  IntervalSet(std::initializer_list<value_type> init, const Comp& comp = Comp(), const Alloc& alloc = Alloc()) : IntervalSet(init.begin(), init.end(), comp, alloc) {}
  IntervalSet(std::initializer_list<value_type> init, const Alloc& alloc) : IntervalSet(init.begin(), init.end(), Comp(), alloc) {}
  IntervalSet& operator=(const IntervalSet& x) { return s = x.s, *this; }
  IntervalSet& operator=(IntervalSet&& x) noexcept(noexcept(s = std::move(x.s))) { return s = x.s, *this; }
  IntervalSet& operator=(std::initializer_list<value_type> init) { return s = init, *this; }
  allocator_type get_allocator() const noexcept { return s.get_allocator(); }
  auto begin() const noexcept { return s.begin(); }
  auto cbegin() const noexcept { return s.cbegin(); }
  auto end() const noexcept { return s.end(); }
  auto cend() const noexcept { return s.cend(); }
  auto rbegin() const noexcept { return s.rbegin(); }
  auto crbegin() const noexcept { return s.crbegin(); }
  auto rend() const noexcept { return s.rend(); }
  auto crend() const noexcept { return s.crend(); }
  bool empty() const noexcept { return s.empty(); }
  u32 size() const noexcept { return s.size(); }
  u32 max_size() const noexcept { return std::min(0xffffffffu, (u32)s.max_size()); }
  void clear() noexcept { s.clear(); }
  template<class OnAdd = empty_hook, class OnDel = empty_hook> void insert(const T& l, const T& r, OnAdd on_add = OnAdd(), OnDel on_del = OnDel());
  template<class OnAdd = empty_hook, class OnDel = empty_hook> void insert(const value_type& v, OnAdd on_add = OnAdd(), OnDel on_del = OnDel()) { insert(v.first, v.second, on_add, on_del); }
  template<class OnAdd = empty_hook, class OnDel = empty_hook> void insert(std::initializer_list<value_type> init, OnAdd on_add = OnAdd(), OnDel on_del = OnDel()) { insert_range(init, on_add, on_del); }
  template<std::ranges::input_range R, class OnAdd = empty_hook, class OnDel = empty_hook> void insert_range(R&& r, OnAdd on_add = OnAdd(), OnDel on_del = OnDel()) {
    for(const auto& [first, second] : r) insert(static_cast<T>(first), static_cast<T>(second), on_add, on_del);
  }
  template<class OnAdd = empty_hook, class OnDel = empty_hook> void erase(const T& l, const T& r, OnAdd on_add = OnAdd(), OnDel on_del = OnDel());
  template<class OnAdd = empty_hook, class OnDel = empty_hook> void erase(const value_type& v, OnAdd on_add = OnAdd(), OnDel on_del = OnDel()) { erase(v.first, v.second, on_add, on_del); }
  const_iterator erase(const_iterator pos) { return s.erase(pos); }
  const_iterator erase(const_iterator first, const_iterator last) { return s.erase(first, last); }
  template<std::ranges::input_range R, class OnAdd, class OnDel> void erase_range(R&& r, OnAdd on_add = OnAdd(), OnDel on_del = OnDel()) {
    for(const auto& [first, second] : r) erase(static_cast<T>(first), static_cast<T>(second), on_add, on_del);
  }
  void swap(IntervalSet& x) noexcept(noexcept(s.swap(x.s))) { s.swap(x.s); }
  template<class C2> void merge(IntervalSet<T, C2, Alloc>& source) {
    if(std::allocator_traits<Alloc>::is_always_equal::value && s.size() < source.s.size()) {
      source.insert_range(s);
      (*this) = std::move(source);
    } else {
      insert_range(source);
      source.clear();
    }
  }
  template<class C2> void merge(IntervalSet<T, C2, Alloc>&& source) { merge(source); }
  template<class OnAdd = empty_hook, class OnDel = empty_hook> void split(const T& p, OnAdd on_add = OnAdd(), OnDel on_del = OnDel());
  template<class OnAdd = empty_hook, class OnDel = empty_hook> void split(const T& l, const T& r, OnAdd on_add = OnAdd(), OnDel on_del = OnDel()) { split(l, on_add, on_del), split(r, on_add, on_del); }
  template<class OnAdd = empty_hook, class OnDel = empty_hook> void split(const value_type& v, OnAdd on_add = OnAdd(), OnDel on_del = OnDel()) { split(v.first, v.second, on_add, on_del); }
  template<class K, class F = Plus> void slide(const K& k, F func = F());
  const_iterator find(const T& p) const;
  const_iterator lower_bound(const T& p) const;
  const_iterator upper_bound(const T& p) const;
  bool contains(const T& p) const;
  bool intersects(const T& l, const T& r) const;
  bool intersects(const value_type& v) const { return intersects(v.first, v.second); }
  bool covered(const T& l, const T& r) const;
  bool covered(const value_type& v) const { return intersects(v.first, v.second); }
  Subrange<const_iterator> intersecting_ranges(const T& l, const T& r) const;
  Subrange<const_iterator> intersecting_ranges(const value_type& v) const { return intersects_range(v.first, v.second); }
  Subrange<const_iterator> included_ranges(const T& l, const T& r) const;
  Subrange<const_iterator> included_ranges(const value_type& v) const { return included_ranges(v.first, v.second); }
  template<class F> void visit(const T& l, const T& r, F callback) const;
  template<class F> void visit(const value_type& v, F callback) const { visit(v.first, v.second, std::move(callback)); }
  T mex() const {
    auto itr = find(T{});
    if(itr == end()) return T{};
    else itr->second;
  }
  T mex(const T& p) const {
    auto itr = find(p);
    if(itr == end()) return p;
    else itr->second;
  }
  key_compare key_comp() const { return s.key_comp().comp; }
  value_compare value_comp() const { return s.key_comp(); }
};
} // namespace gsh
