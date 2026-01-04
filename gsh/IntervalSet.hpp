#pragma once
#include "Memory.hpp"
#include "Range.hpp"
#include "Vec.hpp"
#include <algorithm>
#include <functional>
#include <optional>
#include <ranges>
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
  using allocator_type = Alloc;
  class value_compare {
    friend class IntervalSet;
    [[no_unique_address]] mutable Comp comp;
    constexpr value_compare() = default;
    constexpr value_compare(const Comp& comp) : comp(comp) {}
  public:
    using is_transparent = void;
    constexpr bool operator()(const value_type& a, const value_type& b) const { return std::invoke(comp, a.first, b.first); }
    constexpr bool operator()(const value_type& a, const T& b) const { return std::invoke(comp, a.first, b); }
    constexpr bool operator()(const T& a, const value_type& b) const { return std::invoke(comp, a, b.first); }
    constexpr bool operator()(const T& a, const T& b) const { return std::invoke(comp, a, b); }
  };
  using set_type = std::set<value_type, value_compare, Alloc>;
  using iterator = typename set_type::const_iterator;
  using const_iterator = iterator;
  using reverse_iterator = typename set_type::const_reverse_iterator;
  using const_reverse_iterator = reverse_iterator;
private:
  [[no_unique_address]] mutable Comp comp;
  set_type s;
  using empty_hook = internal::IntervalSetEmptyHook;
  template<class H> constexpr static bool is_empty_hook = std::is_same_v<std::remove_cvref_t<H>, internal::IntervalSetEmptyHook>;
  static constexpr bool lt_(const Comp& comp, const T& a, const T& b) { return std::invoke(comp, a, b); }
  static constexpr bool le_(const Comp& comp, const T& a, const T& b) { return !std::invoke(comp, b, a); }
  static constexpr bool eq_(const Comp& comp, const T& a, const T& b) { return !std::invoke(comp, a, b) && !std::invoke(comp, b, a); }
  static constexpr const T& min_(const Comp& comp, const T& a, const T& b) { return std::invoke(comp, b, a) ? b : a; }
  static constexpr const T& max_(const Comp& comp, const T& a, const T& b) { return std::invoke(comp, a, b) ? b : a; }
  GSH_INTERNAL_INLINE const_iterator first_maybe_touching_(const T& l) const {
    auto it = s.lower_bound(l);
    if(it != s.begin()) {
      auto pit = std::prev(it);
      if(le_(comp, l, pit->second)) it = pit;
    }
    return it;
  }
  template<class OnDel> GSH_INTERNAL_INLINE void erase_and_hook_(const_iterator it, OnDel& on_del) {
    if constexpr(!is_empty_hook<OnDel>) std::invoke(on_del, it->first, it->second);
    s.erase(it);
  }
public:
  IntervalSet() : comp(), s(value_compare(comp), Alloc()) {}
  explicit IntervalSet(const Comp& comp, const Alloc& alloc = Alloc()) : comp(comp), s(value_compare(comp), alloc) {}
  explicit IntervalSet(const Alloc& alloc) : comp(), s(value_compare(comp), alloc) {}
  template<class Iter> IntervalSet(Iter first, Iter last, const Comp& comp = Comp(), const Alloc& alloc = Alloc()) : comp(comp), s(value_compare(comp), alloc) {
    for(; first != last; ++first) { insert(static_cast<T>(first->first), static_cast<T>(first->second)); }
  }
  template<class Iter> IntervalSet(Iter first, Iter last, const Alloc& alloc) : IntervalSet(first, last, Comp(), alloc) {}
  IntervalSet(const IntervalSet&) = default;
  IntervalSet(IntervalSet&&) = default;
  IntervalSet(const IntervalSet& x, const Alloc& alloc) : comp(x.comp), s(x.s, alloc) {}
  IntervalSet(IntervalSet&& x, const Alloc& alloc) : comp(std::move(x.comp)), s(std::move(x.s), alloc) {}
  IntervalSet(std::initializer_list<value_type> init, const Comp& comp = Comp(), const Alloc& alloc = Alloc()) : IntervalSet(init.begin(), init.end(), comp, alloc) {}
  IntervalSet(std::initializer_list<value_type> init, const Alloc& alloc) : IntervalSet(init.begin(), init.end(), Comp(), alloc) {}
  IntervalSet& operator=(const IntervalSet& x) {
    if(this == &x) return *this;
    comp = x.comp;
    s = x.s;
    return *this;
  }
  IntervalSet& operator=(IntervalSet&& x) noexcept(noexcept(s = std::move(x.s)) && noexcept(comp = std::move(x.comp))) {
    if(this == &x) return *this;
    comp = std::move(x.comp);
    s = std::move(x.s);
    return *this;
  }
  IntervalSet& operator=(std::initializer_list<value_type> init) {
    clear();
    insert_range(init);
    return *this;
  }
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
  u32 max_size() const noexcept { return static_cast<u32>(std::min<std::size_t>(0xffffffffu, s.max_size())); }
  void clear() noexcept { s.clear(); }
  template<class OnAdd = empty_hook, class OnDel = empty_hook> void insert(const T& l, const T& r, OnAdd on_add = OnAdd(), OnDel on_del = OnDel()) {
    if(!lt_(comp, l, r)) return;
    T nl = l;
    T nr = r;
    auto it = first_maybe_touching_(l);
    while(it != s.end() && le_(comp, it->first, nr)) {
      nl = min_(comp, nl, it->first);
      nr = max_(comp, nr, it->second);
      auto cur = it++;
      if constexpr(!is_empty_hook<OnDel>) std::invoke(on_del, cur->first, cur->second);
      s.erase(cur);
    }
    if constexpr(!is_empty_hook<OnAdd>) std::invoke(on_add, nl, nr);
    s.emplace_hint(it, nl, nr);
  }
  template<class OnAdd = empty_hook, class OnDel = empty_hook> void insert(const value_type& v, OnAdd on_add = OnAdd(), OnDel on_del = OnDel()) { insert(v.first, v.second, on_add, on_del); }
  template<class OnAdd = empty_hook, class OnDel = empty_hook> void insert(std::initializer_list<value_type> init, OnAdd on_add = OnAdd(), OnDel on_del = OnDel()) { insert_range(init, on_add, on_del); }
  template<std::ranges::input_range R, class OnAdd = empty_hook, class OnDel = empty_hook> void insert_range(R&& r, OnAdd on_add = OnAdd(), OnDel on_del = OnDel()) {
    for(const auto& [first, second] : r) insert(static_cast<T>(first), static_cast<T>(second), on_add, on_del);
  }
  template<class OnAdd = empty_hook, class OnDel = empty_hook> void erase(const T& l, const T& r, OnAdd on_add = OnAdd(), OnDel on_del = OnDel()) {
    if(!lt_(comp, l, r)) return;
    auto it = s.lower_bound(l);
    if(it != s.begin()) {
      auto pit = std::prev(it);
      if(lt_(comp, l, pit->second)) it = pit;
    }
    while(it != s.end() && lt_(comp, it->first, r)) {
      const auto a = it->first;
      const auto b = it->second;
      auto cur = it++;
      if constexpr(!is_empty_hook<OnDel>) std::invoke(on_del, a, b);
      s.erase(cur);
      if(lt_(comp, a, l)) {
        if constexpr(!is_empty_hook<OnAdd>) std::invoke(on_add, a, l);
        s.emplace_hint(it, a, l);
      }
      if(lt_(comp, r, b)) {
        if constexpr(!is_empty_hook<OnAdd>) std::invoke(on_add, r, b);
        s.emplace_hint(it, r, b);
        break;
      }
    }
  }
  template<class OnAdd = empty_hook, class OnDel = empty_hook> void erase(const value_type& v, OnAdd on_add = OnAdd(), OnDel on_del = OnDel()) { erase(v.first, v.second, on_add, on_del); }
  const_iterator erase(const_iterator pos) { return s.erase(pos); }
  const_iterator erase(const_iterator first, const_iterator last) { return s.erase(first, last); }
  template<std::ranges::input_range R, class OnAdd = empty_hook, class OnDel = empty_hook> void erase_range(R&& r, OnAdd on_add = OnAdd(), OnDel on_del = OnDel()) {
    for(const auto& [first, second] : r) erase(static_cast<T>(first), static_cast<T>(second), on_add, on_del);
  }
  void swap(IntervalSet& x) noexcept(noexcept(s.swap(x.s)) && noexcept(std::swap(comp, x.comp))) {
    using std::swap;
    swap(comp, x.comp);
    s.swap(x.s);
  }
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
  template<class OnAdd = empty_hook, class OnDel = empty_hook> void split(const T& p, OnAdd on_add = OnAdd(), OnDel on_del = OnDel()) {
    auto it = find(p);
    if(it == s.end()) return;
    const auto a = it->first;
    const auto b = it->second;
    if(eq_(comp, a, p) || eq_(comp, b, p)) return;
    auto nxt = std::next(it);
    if constexpr(!is_empty_hook<OnDel>) std::invoke(on_del, a, b);
    s.erase(it);
    if constexpr(!is_empty_hook<OnAdd>) std::invoke(on_add, a, p);
    s.emplace_hint(nxt, a, p);
    if constexpr(!is_empty_hook<OnAdd>) std::invoke(on_add, p, b);
    s.emplace_hint(nxt, p, b);
  }
  template<class OnAdd = empty_hook, class OnDel = empty_hook> void split(const T& l, const T& r, OnAdd on_add = OnAdd(), OnDel on_del = OnDel()) { split(l, on_add, on_del), split(r, on_add, on_del); }
  template<class OnAdd = empty_hook, class OnDel = empty_hook> void split(const value_type& v, OnAdd on_add = OnAdd(), OnDel on_del = OnDel()) { split(v.first, v.second, on_add, on_del); }
  template<class K, class F = Plus> void slide(const K& k, F func = F()) {
    Vec<value_type> v;
    v.reserve(size());
    for(const auto& [l, r] : s) {
      const T nl = static_cast<T>(std::invoke(func, l, k));
      const T nr = static_cast<T>(std::invoke(func, r, k));
      v.emplace_back(nl, nr);
    }
    s.clear();
    auto hint = s.end();
    for(const auto& [l, r] : v) { hint = s.emplace_hint(hint, l, r); }
  }
  const_iterator find(const T& p) const {
    auto it = s.upper_bound(p);
    if(it == s.begin()) return s.end();
    --it;
    if(le_(comp, it->first, p) && lt_(comp, p, it->second)) return it;
    return s.end();
  }
  const_iterator lower_bound(const T& p) const { return s.lower_bound(p); }
  const_iterator upper_bound(const T& p) const { return s.upper_bound(p); }
  bool contains(const T& p) const { return find(p) != s.end(); }
  bool intersects(const T& l, const T& r) const {
    if(!lt_(comp, l, r)) return false;
    auto it = s.lower_bound(l);
    if(it != s.begin()) {
      auto pit = std::prev(it);
      if(lt_(comp, l, pit->second) && lt_(comp, pit->first, r)) return true;
    }
    if(it != s.end() && lt_(comp, it->first, r) && lt_(comp, l, it->second)) return true;
    return false;
  }
  bool intersects(const value_type& v) const { return intersects(v.first, v.second); }
  bool covered(const T& l, const T& r) const {
    if(!lt_(comp, l, r)) return true;
    auto it = find(l);
    if(it == s.end()) return false;
    return le_(comp, r, it->second);
  }
  bool covered(const value_type& v) const { return covered(v.first, v.second); }
  Subrange<const_iterator> intersecting_intervals(const T& l, const T& r) const {
    if(!lt_(comp, l, r)) return Subrange(s.end(), s.end());
    auto first = s.lower_bound(l);
    if(first != s.begin()) {
      auto pit = std::prev(first);
      if(lt_(comp, l, pit->second)) first = pit;
    }
    auto last = s.lower_bound(r);
    return Subrange(first, last);
  }
  Subrange<const_iterator> intersecting_intervals(const value_type& v) const { return intersecting_intervals(v.first, v.second); }
  Subrange<const_iterator> included_intervals(const T& l, const T& r) const {
    if(!lt_(comp, l, r)) return Subrange(s.end(), s.end());
    auto first = s.lower_bound(l);
    auto it = first;
    while(it != s.end()) {
      if(!lt_(comp, it->first, r)) break;
      if(lt_(comp, r, it->second)) break;
      ++it;
    }
    return Subrange(first, it);
  }
  Subrange<const_iterator> included_intervals(const value_type& v) const { return included_intervals(v.first, v.second); }
  template<class F> void visit(const T& l, const T& r, F callback) const {
    for(const auto& seg : this->intersecting_intervals(l, r)) {
      const T il = static_cast<T>(max_(comp, seg.first, l));
      const T ir = static_cast<T>(min_(comp, seg.second, r));
      if(!lt_(comp, il, ir)) continue;
      if constexpr(requires { std::invoke(callback, il, ir); }) {
        std::invoke(callback, il, ir);
      } else {
        std::invoke(callback, value_type{il, ir});
      }
    }
  }
  template<class F> void visit(const value_type& v, F callback) const { visit(v.first, v.second, std::move(callback)); }
  std::optional<value_type> get(const T& p) const {
    auto it = find(p);
    if(it == s.end()) return std::nullopt;
    return *it;
  }
  T mex() const {
    auto itr = find(T{});
    if(itr == end()) return T{};
    else return itr->second;
  }
  T mex(const T& p) const {
    auto itr = find(p);
    if(itr == end()) return p;
    else return itr->second;
  }
  key_compare key_comp() const { return comp; }
  value_compare value_comp() const { return value_compare(comp); }
};
template<class Iter, class Comp = Less, class Alloc = SingleAllocator<typename std::iterator_traits<Iter>::value_type>> requires std::same_as<typename std::iterator_traits<Iter>::value_type::first_type, typename std::iterator_traits<Iter>::value_type::second_type> IntervalSet(Iter, Iter, Comp = Comp(), Alloc = Alloc()) -> IntervalSet<typename std::iterator_traits<Iter>::value_type::first_type, Comp, Alloc>;
template<class T, class Comp = Less, class Alloc = SingleAllocator<std::pair<T, T>>> IntervalSet(std::initializer_list<std::pair<T, T>>, Comp = Comp(), Alloc = Alloc()) -> IntervalSet<T, Comp, Alloc>;
template<class Iter, class Alloc = SingleAllocator<typename std::iterator_traits<Iter>::value_type>> requires std::same_as<typename std::iterator_traits<Iter>::value_type::first_type, typename std::iterator_traits<Iter>::value_type::second_type> IntervalSet(Iter, Iter, Alloc) -> IntervalSet<typename std::iterator_traits<Iter>::value_type::first_type, Less, Alloc>;
template<class T, class Alloc = SingleAllocator<std::pair<T, T>>> IntervalSet(std::initializer_list<std::pair<T, T>>, Alloc) -> IntervalSet<T, Less, Alloc>;
} // namespace gsh
