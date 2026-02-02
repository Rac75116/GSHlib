#pragma once
#include "Functional.hpp"
#include "Memory.hpp"
#include "Range.hpp"
#include "TypeDef.hpp"
#include "Vec.hpp"
#include "internal/UtilMacro.hpp"
#include <algorithm>
#include <concepts>
#include <cstddef>
#include <functional>
#include <initializer_list>
#include <iterator>
#include <memory>
#include <new>
#include <optional>
#include <ranges>
#include <set>
#include <type_traits>
#include <utility>
#include <variant>
namespace gsh {
namespace internal {
struct IntervalSetEmptyHook {
  template<class T, class U> constexpr void operator()(T&&, U&&) {}
  template<class T, class U, class V> constexpr void operator()(T&&, U&&, V&&) {}
};
}
template<class Key, class Value = std::monostate> struct Interval {
  Key left, right;
  [[no_unique_address]] Value value;
};
template<class Key, class Value = std::monostate, class Comp = Less, class ValueComp = EqualTo, class Alloc = SingleAllocator<Interval<Key, Value>>> class IntervalSet : public ViewInterface<IntervalSet<Key, Value, Comp, ValueComp, Alloc>, Interval<Key, Value>> {
public:
  using interval_type = Interval<Key, Value>;
  using bounds_type = Interval<Key, std::monostate>;
  using value_type = interval_type; // [left, right)
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
    constexpr bool operator()(const value_type& a, const value_type& b) const { return std::invoke(comp, a.right, b.right); }
    constexpr bool operator()(const value_type& a, const Key& b) const { return std::invoke(comp, a.right, b); }
    constexpr bool operator()(const Key& a, const value_type& b) const { return std::invoke(comp, a, b.right); }
    constexpr bool operator()(const Key& a, const Key& b) const { return std::invoke(comp, a, b); }
  };
  using set_type = std::set<value_type, value_compare, Alloc>;
  using iterator = typename set_type::const_iterator;
  using const_iterator = iterator;
  using reverse_iterator = typename set_type::const_reverse_iterator;
  using const_reverse_iterator = reverse_iterator;
private:
  [[no_unique_address]] mutable Comp comp;
  [[no_unique_address]] mutable ValueComp vcomp;
  set_type s;
  using empty_hook = internal::IntervalSetEmptyHook;
  template<class H> constexpr static bool is_empty_hook = std::is_same_v<std::remove_cvref_t<H>, internal::IntervalSetEmptyHook>;
  GSH_INTERNAL_INLINE static constexpr bool lt_(const Comp& comp, const Key& a, const Key& b) { return std::invoke(comp, a, b); }
  GSH_INTERNAL_INLINE static constexpr bool le_(const Comp& comp, const Key& a, const Key& b) { return !std::invoke(comp, b, a); }
  GSH_INTERNAL_INLINE static constexpr bool eq_(const Comp& comp, const Key& a, const Key& b) { return !std::invoke(comp, a, b) && !std::invoke(comp, b, a); }
  GSH_INTERNAL_INLINE static constexpr const Key& min_(const Comp& comp, const Key& a, const Key& b) { return std::invoke(comp, b, a) ? b : a; }
  GSH_INTERNAL_INLINE static constexpr const Key& max_(const Comp& comp, const Key& a, const Key& b) { return std::invoke(comp, a, b) ? b : a; }
  GSH_INTERNAL_INLINE constexpr bool eq_value_(const Value& a, const Value& b) const {
    if constexpr(std::is_same_v<Value, std::monostate>) {
      return true;
    } else if constexpr(std::is_invocable_r_v<bool, ValueComp&, const Value&, const Value&>) {
      return std::invoke(vcomp, a, b);
    } else {
      static_assert(std::is_invocable_r_v<bool, ValueComp&, const Value&, const Value&>, "ValueComp must be invocable as bool(Value, Value)");
      return false;
    }
  }
  template<class OnAdd> GSH_INTERNAL_INLINE static constexpr void add_hook_(OnAdd& on_add, const Key& l, const Key& r, const Value& value) {
    if constexpr(!is_empty_hook<OnAdd>) {
      if constexpr(requires { std::invoke(on_add, l, r, value); }) {
        std::invoke(on_add, l, r, value);
      } else {
        std::invoke(on_add, l, r);
      }
    }
  }
  template<class OnDel> GSH_INTERNAL_INLINE static constexpr void del_hook_(OnDel& on_del, const value_type& v) {
    if constexpr(!is_empty_hook<OnDel>) {
      if constexpr(requires { std::invoke(on_del, v.left, v.right, v.value); }) {
        std::invoke(on_del, v.left, v.right, v.value);
      } else {
        std::invoke(on_del, v.left, v.right);
      }
    }
  }
  struct shard_slot {
    bool has = false;
    alignas(value_type) unsigned char buf[sizeof(value_type)];
    GSH_INTERNAL_INLINE value_type* ptr() { return std::launder(reinterpret_cast<value_type*>(buf)); }
    GSH_INTERNAL_INLINE const value_type* ptr() const { return std::launder(reinterpret_cast<const value_type*>(buf)); }
    GSH_INTERNAL_INLINE void reset() {
      if(has) {
        ptr()->~value_type();
        has = false;
      }
    }
    GSH_INTERNAL_INLINE void emplace(const Key& l, const Key& r, const Value& v) {
      reset();
      ::new((void*)buf) value_type(l, r, v);
      has = true;
    }
    GSH_INTERNAL_INLINE value_type& get() { return *ptr(); }
    GSH_INTERNAL_INLINE const value_type& get() const { return *ptr(); }
    ~shard_slot() { reset(); }
  };
public:
  IntervalSet() : comp(), vcomp(), s(value_compare(comp), Alloc()) {}
  explicit IntervalSet(const Comp& comp, const Alloc& alloc = Alloc()) : comp(comp), vcomp(), s(value_compare(comp), alloc) {}
  explicit IntervalSet(const Comp& comp, const ValueComp& vcomp, const Alloc& alloc = Alloc()) : comp(comp), vcomp(vcomp), s(value_compare(comp), alloc) {}
  explicit IntervalSet(const Alloc& alloc) : comp(), vcomp(), s(value_compare(comp), alloc) {}
  template<class Iter> IntervalSet(Iter first, Iter last, const Comp& comp = Comp(), const ValueComp& vcomp = ValueComp(), const Alloc& alloc = Alloc()) : comp(comp), vcomp(vcomp), s(value_compare(comp), alloc) { insert_range(Subrange(first, last)); }
  template<class Iter> IntervalSet(Iter first, Iter last, const Alloc& alloc) : IntervalSet(first, last, Comp(), ValueComp(), alloc) {}
  IntervalSet(std::initializer_list<value_type> init, const Comp& comp = Comp(), const ValueComp& vcomp = ValueComp(), const Alloc& alloc = Alloc()) : IntervalSet(init.begin(), init.end(), comp, vcomp, alloc) {}
  IntervalSet(std::initializer_list<value_type> init, const Alloc& alloc) : IntervalSet(init.begin(), init.end(), Comp(), ValueComp(), alloc) {}
  IntervalSet(const IntervalSet&) = default;
  IntervalSet(IntervalSet&&) = default;
  IntervalSet(const IntervalSet& x, const Alloc& alloc) : comp(x.comp), vcomp(x.vcomp), s(x.s, alloc) {}
  IntervalSet(IntervalSet&& x, const Alloc& alloc) : comp(std::move(x.comp)), vcomp(std::move(x.vcomp)), s(std::move(x.s), alloc) {}
  IntervalSet& operator=(const IntervalSet& x) {
    if(this == &x) return *this;
    comp = x.comp;
    vcomp = x.vcomp;
    s = x.s;
    return *this;
  }
  IntervalSet& operator=(IntervalSet&& x) noexcept(noexcept(s = std::move(x.s)) && noexcept(comp = std::move(x.comp)) && noexcept(vcomp = std::move(x.vcomp))) {
    if(this == &x) return *this;
    comp = std::move(x.comp);
    vcomp = std::move(x.vcomp);
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
  template<class OnAdd = empty_hook, class OnDel = empty_hook> void insert(const Key& l, const Key& r, OnAdd on_add = OnAdd(), OnDel on_del = OnDel()) requires std::same_as<Value, std::monostate> { insert(l, r, Value{}, on_add, on_del); }
  template<class OnAdd = empty_hook, class OnDel = empty_hook> void insert(const Key& l, const Key& r, const Value& value, OnAdd on_add = OnAdd(), OnDel on_del = OnDel()) {
    if(!lt_(comp, l, r)) return;
    Key new_l = l;
    Key new_r = r;
    auto it = s.lower_bound(new_l);
    if(it != s.begin()) {
      auto prev = std::prev(it);
      bool prev_same = true;
      if constexpr(!std::same_as<Value, std::monostate>) { prev_same = eq_value_(prev->value, value); }
      if(eq_(comp, prev->right, new_l) && prev_same) {
        new_l = prev->left;
        it = prev;
      }
    }
    shard_slot left_shard;
    shard_slot right_shard;
    auto start_erase = it;
    while(it != s.end()) {
      if(lt_(comp, new_r, it->left)) break;
      bool same = true;
      if constexpr(!std::same_as<Value, std::monostate>) { same = eq_value_(it->value, value); }
      if(eq_(comp, new_r, it->left) && !same) break;
      if(lt_(comp, it->left, new_l)) {
        if(same) {
          new_l = it->left;
        } else {
          left_shard.emplace(it->left, new_l, it->value);
        }
      }
      if(lt_(comp, new_r, it->right)) {
        if(same) {
          new_r = it->right;
        } else {
          right_shard.emplace(new_r, it->right, it->value);
        }
      }
      del_hook_(on_del, *it);
      it++;
    }
    auto hint = s.erase(start_erase, it);
    if(right_shard.has) {
      const auto& rs = right_shard.get();
      add_hook_(on_add, rs.left, rs.right, rs.value);
      hint = s.emplace_hint(hint, rs.left, rs.right, rs.value);
    }
    add_hook_(on_add, new_l, new_r, value);
    auto it_new = s.emplace_hint(hint, new_l, new_r, value);
    if(left_shard.has) {
      const auto& ls = left_shard.get();
      add_hook_(on_add, ls.left, ls.right, ls.value);
      s.emplace_hint(it_new, ls.left, ls.right, ls.value);
    }
  }
  template<class OnAdd = empty_hook, class OnDel = empty_hook> void insert(const value_type& v, OnAdd on_add = OnAdd(), OnDel on_del = OnDel()) { insert(v.left, v.right, v.value, on_add, on_del); }
  template<class OnAdd = empty_hook, class OnDel = empty_hook> void insert(const bounds_type& v, OnAdd on_add = OnAdd(), OnDel on_del = OnDel()) requires (!std::same_as<Value, std::monostate>) { insert(v.left, v.right, on_add, on_del); }
  template<class OnAdd = empty_hook, class OnDel = empty_hook> void insert(std::initializer_list<value_type> init, OnAdd on_add = OnAdd(), OnDel on_del = OnDel()) { insert_range(init, on_add, on_del); }
  template<std::ranges::input_range R, class OnAdd = empty_hook, class OnDel = empty_hook> void insert_range(R&& r, OnAdd on_add = OnAdd(), OnDel on_del = OnDel()) {
    for(const auto& e : r) {
      constexpr bool is_interval = requires {
        e.left;
        e.right;
      };
      if constexpr(is_interval) {
        if constexpr(requires { e.value; } && std::is_convertible_v<decltype(e.value), Value>) {
          insert(static_cast<Key>(e.left), static_cast<Key>(e.right), static_cast<Value>(e.value), on_add, on_del);
        } else {
          insert(static_cast<Key>(e.left), static_cast<Key>(e.right), Value{}, on_add, on_del);
        }
      } else {
        insert(static_cast<Key>(e.first), static_cast<Key>(e.second), Value{}, on_add, on_del);
      }
    }
  }
  template<class OnAdd = empty_hook, class OnDel = empty_hook> void erase(const Key& l, const Key& r, OnAdd on_add = OnAdd(), OnDel on_del = OnDel()) {
    if(!lt_(comp, l, r)) return;
    auto it = s.lower_bound(l);
    shard_slot left_shard;
    shard_slot right_shard;
    auto start_erase = it;
    while(it != s.end()) {
      if(!lt_(comp, it->left, r)) break;
      if(lt_(comp, it->left, l)) { left_shard.emplace(it->left, l, it->value); }
      if(lt_(comp, r, it->right)) { right_shard.emplace(r, it->right, it->value); }
      del_hook_(on_del, *it);
      it++;
    }
    auto hint = s.erase(start_erase, it);
    if(right_shard.has) {
      const auto& rs = right_shard.get();
      add_hook_(on_add, rs.left, rs.right, rs.value);
      hint = s.emplace_hint(hint, rs.left, rs.right, rs.value);
    }
    if(left_shard.has) {
      const auto& ls = left_shard.get();
      add_hook_(on_add, ls.left, ls.right, ls.value);
      s.emplace_hint(hint, ls.left, ls.right, ls.value);
    }
  }
  template<class OnAdd = empty_hook, class OnDel = empty_hook> void erase(const bounds_type& v, OnAdd on_add = OnAdd(), OnDel on_del = OnDel()) { erase(v.left, v.right, on_add, on_del); }
  const_iterator erase(const_iterator pos) { return s.erase(pos); }
  const_iterator erase(const_iterator first, const_iterator last) { return s.erase(first, last); }
  template<std::ranges::input_range R, class OnAdd = empty_hook, class OnDel = empty_hook> void erase_range(R&& r, OnAdd on_add = OnAdd(), OnDel on_del = OnDel()) {
    for(const auto& e : r) {
      constexpr bool is_interval = requires {
        e.left;
        e.right;
      };
      if constexpr(is_interval) {
        erase(static_cast<Key>(e.left), static_cast<Key>(e.right), on_add, on_del);
      } else {
        erase(static_cast<Key>(e.first), static_cast<Key>(e.second), on_add, on_del);
      }
    }
  }
  void swap(IntervalSet& x) noexcept(noexcept(s.swap(x.s)) && noexcept(std::swap(comp, x.comp)) && noexcept(std::swap(vcomp, x.vcomp))) {
    using std::swap;
    swap(comp, x.comp);
    swap(vcomp, x.vcomp);
    s.swap(x.s);
  }
  template<class OnAdd = empty_hook, class OnDel = empty_hook> void merge(IntervalSet& source, OnAdd on_add = OnAdd(), OnDel on_del = OnDel()) {
    if constexpr(std::is_same_v<Value, std::monostate> && is_empty_hook<OnAdd> && is_empty_hook<OnDel> && std::allocator_traits<Alloc>::is_always_equal::value) {
      if(source.size() <= size()) {
        insert_range(source);
        source.clear();
      } else {
        source.insert_range(*this);
        (*this) = std::move(source);
      }
    } else {
      insert_range(source, on_add, on_del);
      source.clear();
    }
  }
  template<class OnAdd = empty_hook, class OnDel = empty_hook> void merge(IntervalSet&& source, OnAdd on_add = OnAdd(), OnDel on_del = OnDel()) { merge(source, on_add, on_del); }
  template<class OnAdd = empty_hook, class OnDel = empty_hook> void split(const Key& p, OnAdd on_add = OnAdd(), OnDel on_del = OnDel()) {
    auto it = find(p);
    if(it == s.end()) return;
    const auto a = it->left;
    const auto b = it->right;
    const Value old_value = it->value;
    if(eq_(comp, a, p) || eq_(comp, b, p)) return;
    auto nxt = std::next(it);
    del_hook_(on_del, *it);
    s.erase(it);
    {
      add_hook_(on_add, a, p, old_value);
      s.emplace_hint(nxt, value_type{a, p, old_value});
    }
    {
      add_hook_(on_add, p, b, old_value);
      s.emplace_hint(nxt, value_type{p, b, old_value});
    }
  }
  template<class K, class F = PlusFunc> void slide(const K& k, F func = F()) {
    Vec<value_type> v;
    v.reserve(size());
    for(const auto& seg : s) {
      const Key nl = static_cast<Key>(std::invoke(func, seg.left, k));
      const Key nr = static_cast<Key>(std::invoke(func, seg.right, k));
      v.emplace_back(value_type{nl, nr, seg.value});
    }
    s.clear();
    s.insert_range(v);
  }
  const_iterator find(const Key& p) const {
    auto it = s.upper_bound(p);
    if(it == s.end()) return s.end();
    if(le_(comp, it->left, p)) return it;
    return s.end();
  }
  const_iterator lower_bound(const Key& p) const { return s.lower_bound(p); }
  const_iterator upper_bound(const Key& p) const { return s.upper_bound(p); }
  bool contains(const Key& p) const { return find(p) != s.end(); }
  bool intersects(const Key& l, const Key& r) const {
    if(!lt_(comp, l, r)) return false;
    auto it = s.upper_bound(l);
    return it != s.end() && lt_(comp, it->left, r);
  }
  bool intersects(const bounds_type& v) const { return intersects(v.left, v.right); }
  bool covered(const Key& l, const Key& r) const {
    if(!lt_(comp, l, r)) return true;
    auto it = s.upper_bound(l);
    if(it == s.end() || lt_(comp, l, it->left)) return false;
    Key current_end = it->right;
    while(lt_(comp, current_end, r)) {
      it++;
      if(it == s.end()) return false;
      if(lt_(comp, current_end, it->left)) return false;
      current_end = max_(comp, current_end, it->right);
    }
    return true;
  }
  bool covered(const bounds_type& v) const { return covered(v.left, v.right); }
  Subrange<const_iterator> intersecting_intervals(const Key& l, const Key& r) const {
    if(!lt_(comp, l, r)) return {s.end(), s.end()};
    auto first = s.upper_bound(l);
    auto last = s.lower_bound(r);
    if(last != s.end() && lt_(comp, last->left, r)) { last = std::next(last); }
    return {first, last};
  }
  Subrange<const_iterator> intersecting_intervals(const bounds_type& v) const { return intersecting_intervals(v.left, v.right); }
  Subrange<const_iterator> included_intervals(const Key& l, const Key& r) const {
    if(!lt_(comp, l, r)) return {s.end(), s.end()};
    auto first = s.lower_bound(l);
    if(first != s.end() && lt_(comp, first->left, l)) { first = std::next(first); }
    auto last = s.upper_bound(r);
    return {first, last};
  }
  Subrange<const_iterator> included_intervals(const bounds_type& v) const { return included_intervals(v.left, v.right); }
  template<class F> void visit(const Key& l, const Key& r, F callback) const {
    for(const auto& seg : this->intersecting_intervals(l, r)) {
      const Key il = static_cast<Key>(max_(comp, seg.left, l));
      const Key ir = static_cast<Key>(min_(comp, seg.right, r));
      if(!lt_(comp, il, ir)) continue;
      if constexpr(requires { std::invoke(callback, il, ir, seg.value); }) {
        std::invoke(callback, il, ir, seg.value);
      } else if constexpr(requires { std::invoke(callback, il, ir); }) {
        std::invoke(callback, il, ir);
      } else if constexpr(requires { std::invoke(callback, value_type{il, ir, seg.value}); }) {
        std::invoke(callback, value_type{il, ir, seg.value});
      } else {
        static_assert(requires { std::invoke(callback, il, ir, seg.value); } || requires { std::invoke(callback, il, ir); } || requires { std::invoke(callback, value_type{il, ir, seg.value}); }, "visit callback must be invocable with (l, r, value), (l, r), or (Interval)");
      }
    }
  }
  template<class F> void visit(const bounds_type& v, F callback) const { visit(v.left, v.right, std::move(callback)); }
  std::optional<value_type> get(const Key& p) const {
    auto it = find(p);
    if(it == s.end()) return std::nullopt;
    return *it;
  }
  key_compare key_comp() const { return comp; }
  value_compare value_comp() const { return value_compare(comp); }
};
template<class InputIterator, class Compare = Less, class Allocator = SingleAllocator<typename std::iterator_traits<InputIterator>::value_type>> requires requires {
  typename std::iterator_traits<InputIterator>::value_type;
  requires requires(const typename std::iterator_traits<InputIterator>::value_type& v) {
    v.left;
    v.right;
    v.value;
  };
} IntervalSet(InputIterator, InputIterator, Compare = Compare(), Allocator = Allocator()) -> IntervalSet<std::remove_cvref_t<decltype(std::declval<typename std::iterator_traits<InputIterator>::value_type>().left)>, std::remove_cvref_t<decltype(std::declval<typename std::iterator_traits<InputIterator>::value_type>().value)>, Compare, EqualTo, Allocator>;
template<class InputIterator, class Compare, class ValueCompare, class Allocator> requires requires {
  typename std::iterator_traits<InputIterator>::value_type;
  requires requires(const typename std::iterator_traits<InputIterator>::value_type& v) {
    v.left;
    v.right;
    v.value;
  };
} IntervalSet(InputIterator, InputIterator, Compare, ValueCompare, Allocator) -> IntervalSet<std::remove_cvref_t<decltype(std::declval<typename std::iterator_traits<InputIterator>::value_type>().left)>, std::remove_cvref_t<decltype(std::declval<typename std::iterator_traits<InputIterator>::value_type>().value)>, Compare, ValueCompare, Allocator>;
template<class Key, class Value = std::monostate, class Compare = Less, class Allocator = SingleAllocator<Interval<Key, Value>>> IntervalSet(std::initializer_list<Interval<Key, Value>>, Compare = Compare(), Allocator = Allocator()) -> IntervalSet<Key, Value, Compare, EqualTo, Allocator>;
template<class Key, class Value, class Compare, class ValueCompare, class Allocator> IntervalSet(std::initializer_list<Interval<Key, Value>>, Compare, ValueCompare, Allocator) -> IntervalSet<Key, Value, Compare, ValueCompare, Allocator>;
template<class InputIterator, class Allocator> requires requires {
  typename std::iterator_traits<InputIterator>::value_type;
  requires requires(const typename std::iterator_traits<InputIterator>::value_type& v) {
    v.left;
    v.right;
    v.value;
  };
} IntervalSet(InputIterator, InputIterator, Allocator) -> IntervalSet<std::remove_cvref_t<decltype(std::declval<typename std::iterator_traits<InputIterator>::value_type>().left)>, std::remove_cvref_t<decltype(std::declval<typename std::iterator_traits<InputIterator>::value_type>().value)>, Less, EqualTo, Allocator>;
template<class Key, class Value = std::monostate, class Allocator = SingleAllocator<Interval<Key, Value>>> IntervalSet(std::initializer_list<Interval<Key, Value>>, Allocator) -> IntervalSet<Key, Value, Less, EqualTo, Allocator>;
template<class InputIterator, class Compare = Less, class Pair = typename std::iterator_traits<InputIterator>::value_type, class Key = std::remove_cvref_t<decltype(std::declval<Pair>().first)>, class Allocator = SingleAllocator<Interval<Key, std::monostate>>> requires requires {
  typename std::iterator_traits<InputIterator>::value_type;
  requires std::same_as<std::remove_cvref_t<decltype(std::declval<Pair>().first)>, std::remove_cvref_t<decltype(std::declval<Pair>().second)>>;
  requires requires(const Pair& v) {
    v.first;
    v.second;
  };
} IntervalSet(InputIterator, InputIterator, Compare = Compare(), Allocator = Allocator()) -> IntervalSet<Key, std::monostate, Compare, EqualTo, Allocator>;
template<class InputIterator, class Compare, class ValueCompare, class Pair = typename std::iterator_traits<InputIterator>::value_type, class Key = std::remove_cvref_t<decltype(std::declval<Pair>().first)>, class Allocator = SingleAllocator<Interval<Key, std::monostate>>> requires requires {
  typename std::iterator_traits<InputIterator>::value_type;
  requires std::same_as<std::remove_cvref_t<decltype(std::declval<Pair>().first)>, std::remove_cvref_t<decltype(std::declval<Pair>().second)>>;
  requires requires(const Pair& v) {
    v.first;
    v.second;
  };
} IntervalSet(InputIterator, InputIterator, Compare, ValueCompare, Allocator) -> IntervalSet<Key, std::monostate, Compare, ValueCompare, Allocator>;
template<class Key, class Compare = Less, class Allocator = SingleAllocator<Interval<Key, std::monostate>>> IntervalSet(std::initializer_list<std::pair<Key, Key>>, Compare = Compare(), Allocator = Allocator()) -> IntervalSet<Key, std::monostate, Compare, EqualTo, Allocator>;
template<class Key, class Compare, class ValueCompare, class Allocator> IntervalSet(std::initializer_list<std::pair<Key, Key>>, Compare, ValueCompare, Allocator) -> IntervalSet<Key, std::monostate, Compare, ValueCompare, Allocator>;
template<class InputIterator, class Pair = typename std::iterator_traits<InputIterator>::value_type, class Key = std::remove_cvref_t<decltype(std::declval<Pair>().first)>, class Allocator = SingleAllocator<Interval<Key, std::monostate>>> requires requires {
  typename std::iterator_traits<InputIterator>::value_type;
  requires std::same_as<std::remove_cvref_t<decltype(std::declval<Pair>().first)>, std::remove_cvref_t<decltype(std::declval<Pair>().second)>>;
  requires requires(const Pair& v) {
    v.first;
    v.second;
  };
} IntervalSet(InputIterator, InputIterator, Allocator) -> IntervalSet<Key, std::monostate, Less, EqualTo, Allocator>;
template<class Key, class Allocator = SingleAllocator<Interval<Key, std::monostate>>> IntervalSet(std::initializer_list<std::pair<Key, Key>>, Allocator) -> IntervalSet<Key, std::monostate, Less, EqualTo, Allocator>;
}
