#pragma once
#include "Memory.hpp"
#include "Range.hpp"
#include "Vec.hpp"
#include <optional>
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
template<class Key, class Value = std::monostate, class Comp = Less, class Alloc = SingleAllocator<Interval<Key, Value>>> class IntervalSet : public ViewInterface<IntervalSet<Key, Value, Comp, Alloc>, Interval<Key, Value>> {
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
    constexpr bool operator()(const value_type& a, const value_type& b) const { return std::invoke(comp, a.left, b.left); }
    constexpr bool operator()(const value_type& a, const Key& b) const { return std::invoke(comp, a.left, b); }
    constexpr bool operator()(const Key& a, const value_type& b) const { return std::invoke(comp, a, b.left); }
    constexpr bool operator()(const Key& a, const Key& b) const { return std::invoke(comp, a, b); }
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
  static constexpr bool lt_(const Comp& comp, const Key& a, const Key& b) { return std::invoke(comp, a, b); }
  static constexpr bool le_(const Comp& comp, const Key& a, const Key& b) { return !std::invoke(comp, b, a); }
  static constexpr bool eq_(const Comp& comp, const Key& a, const Key& b) { return !std::invoke(comp, a, b) && !std::invoke(comp, b, a); }
  static constexpr const Key& min_(const Comp& comp, const Key& a, const Key& b) { return std::invoke(comp, b, a) ? b : a; }
  static constexpr const Key& max_(const Comp& comp, const Key& a, const Key& b) { return std::invoke(comp, a, b) ? b : a; }
  template<class OnAdd> static constexpr Value make_value_(OnAdd& on_add, const Key& l, const Key& r) {
    if constexpr(is_empty_hook<OnAdd>) {
      return Value{};
    } else if constexpr(requires { std::invoke(on_add, l, r); }) {
      if constexpr(std::is_void_v<std::invoke_result_t<OnAdd&, const Key&, const Key&>>) {
        std::invoke(on_add, l, r);
        return Value{};
      } else {
        return static_cast<Value>(std::invoke(on_add, l, r));
      }
    } else {
      static_assert(requires { std::invoke(on_add, l, r); }, "OnAdd must be invocable with (l, r)");
    }
  }
  template<class OnDel> static constexpr void del_hook_(OnDel& on_del, const value_type& v) {
    if constexpr(!is_empty_hook<OnDel>) {
      if constexpr(requires { std::invoke(on_del, v.left, v.right, v.value); }) {
        std::invoke(on_del, v.left, v.right, v.value);
      } else {
        std::invoke(on_del, v.left, v.right);
      }
    }
  }
  GSH_INTERNAL_INLINE const_iterator first_maybe_touching_(const Key& l) const {
    auto it = s.lower_bound(l);
    if(it != s.begin()) {
      auto pit = std::prev(it);
      if(le_(comp, l, pit->right)) it = pit;
    }
    return it;
  }
public:
  IntervalSet() : comp(), s(value_compare(comp), Alloc()) {}
  explicit IntervalSet(const Comp& comp, const Alloc& alloc = Alloc()) : comp(comp), s(value_compare(comp), alloc) {}
  explicit IntervalSet(const Alloc& alloc) : comp(), s(value_compare(comp), alloc) {}
  IntervalSet(const IntervalSet&) = default;
  IntervalSet(IntervalSet&&) = default;
  IntervalSet(const IntervalSet& x, const Alloc& alloc) : comp(x.comp), s(x.s, alloc) {}
  IntervalSet(IntervalSet&& x, const Alloc& alloc) : comp(std::move(x.comp)), s(std::move(x.s), alloc) {}
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
  template<class OnAdd = empty_hook, class OnDel = empty_hook> void insert(const Key& l, const Key& r, OnAdd on_add = OnAdd(), OnDel on_del = OnDel()) {
    if(!lt_(comp, l, r)) return;
    Key nl = l;
    Key nr = r;
    auto it = first_maybe_touching_(l);
    while(it != s.end() && le_(comp, it->left, nr)) {
      nl = min_(comp, nl, it->left);
      nr = max_(comp, nr, it->right);
      auto cur = it++;
      del_hook_(on_del, *cur);
      s.erase(cur);
    }
    value_type v{nl, nr, make_value_(on_add, nl, nr)};
    s.emplace_hint(it, std::move(v));
  }
  template<class OnAdd = empty_hook, class OnDel = empty_hook> void insert(const bounds_type& v, OnAdd on_add = OnAdd(), OnDel on_del = OnDel()) { insert(v.left, v.right, on_add, on_del); }
  template<class OnAdd = empty_hook, class OnDel = empty_hook> void insert(std::initializer_list<value_type> init, OnAdd on_add = OnAdd(), OnDel on_del = OnDel()) requires std::is_same_v<Value, std::monostate> { insert_range(init, on_add, on_del); }
  template<std::ranges::input_range R, class OnAdd = empty_hook, class OnDel = empty_hook> void insert_range(R&& r, OnAdd on_add = OnAdd(), OnDel on_del = OnDel()) {
    for(const auto& e : r) {
      constexpr bool is_interval = requires {
        e.left;
        e.right;
      };
      if constexpr(is_interval) {
        insert(static_cast<Key>(e.left), static_cast<Key>(e.right), on_add, on_del);
      } else {
        insert(static_cast<Key>(e.first), static_cast<Key>(e.second), on_add, on_del);
      }
    }
  }
  template<class OnAdd = empty_hook, class OnDel = empty_hook> void erase(const Key& l, const Key& r, OnAdd on_add = OnAdd(), OnDel on_del = OnDel()) {
    if(!lt_(comp, l, r)) return;
    auto it = s.lower_bound(l);
    if(it != s.begin()) {
      auto pit = std::prev(it);
      if(lt_(comp, l, pit->right)) it = pit;
    }
    while(it != s.end() && lt_(comp, it->left, r)) {
      const auto a = it->left;
      const auto b = it->right;
      auto cur = it++;
      del_hook_(on_del, *cur);
      s.erase(cur);
      if(lt_(comp, a, l)) {
        value_type v{a, l, make_value_(on_add, a, l)};
        s.emplace_hint(it, std::move(v));
      }
      if(lt_(comp, r, b)) {
        value_type v{r, b, make_value_(on_add, r, b)};
        s.emplace_hint(it, std::move(v));
        break;
      }
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
  void swap(IntervalSet& x) noexcept(noexcept(s.swap(x.s)) && noexcept(std::swap(comp, x.comp))) {
    using std::swap;
    swap(comp, x.comp);
    s.swap(x.s);
  }
  template<class C2, class OnAdd = empty_hook, class OnDel = empty_hook> void merge(IntervalSet<Key, Value, C2, Alloc>& source, OnAdd on_add = OnAdd(), OnDel on_del = OnDel()) {
    if constexpr(std::is_same_v<Value, std::monostate> && is_empty_hook<OnAdd> && is_empty_hook<OnDel> && std::allocator_traits<Alloc>::is_always_equal::value) {
      if(source.size() <= s.size()) {
        insert_range(source.s);
        source.clear();
      } else {
        source.insert_range(s);
        (*this) = std::move(source);
      }
    } else {
      insert_range(source.s, on_add, on_del);
      source.clear();
    }
  }
  template<class C2, class OnAdd = empty_hook, class OnDel = empty_hook> void merge(IntervalSet<Key, Value, C2, Alloc>&& source, OnAdd on_add = OnAdd(), OnDel on_del = OnDel()) { merge(source, on_add, on_del); }
  template<class OnAdd = empty_hook, class OnDel = empty_hook> void split(const Key& p, OnAdd on_add = OnAdd(), OnDel on_del = OnDel()) {
    auto it = find(p);
    if(it == s.end()) return;
    const auto a = it->left;
    const auto b = it->right;
    if(eq_(comp, a, p) || eq_(comp, b, p)) return;
    auto nxt = std::next(it);
    del_hook_(on_del, *it);
    s.erase(it);
    {
      value_type v{a, p, make_value_(on_add, a, p)};
      s.emplace_hint(nxt, std::move(v));
    }
    {
      value_type v{p, b, make_value_(on_add, p, b)};
      s.emplace_hint(nxt, std::move(v));
    }
  }
  template<class OnAdd = empty_hook, class OnDel = empty_hook> void split(const Key& l, const Key& r, OnAdd on_add = OnAdd(), OnDel on_del = OnDel()) { split(l, on_add, on_del), split(r, on_add, on_del); }
  template<class OnAdd = empty_hook, class OnDel = empty_hook> void split(const bounds_type& v, OnAdd on_add = OnAdd(), OnDel on_del = OnDel()) { split(v.left, v.right, on_add, on_del); }
  template<class K, class F = Plus> void slide(const K& k, F func = F()) {
    Vec<value_type> v;
    v.reserve(size());
    for(const auto& seg : s) {
      const Key nl = static_cast<Key>(std::invoke(func, seg.left, k));
      const Key nr = static_cast<Key>(std::invoke(func, seg.right, k));
      v.emplace_back(value_type{nl, nr, seg.value});
    }
    s.clear();
    auto hint = s.end();
    for(auto& seg : v) { hint = s.emplace_hint(hint, std::move(seg)); }
  }
  const_iterator find(const Key& p) const {
    auto it = s.upper_bound(p);
    if(it == s.begin()) return s.end();
    --it;
    if(le_(comp, it->left, p) && lt_(comp, p, it->right)) return it;
    return s.end();
  }
  const_iterator lower_bound(const Key& p) const { return s.lower_bound(p); }
  const_iterator upper_bound(const Key& p) const { return s.upper_bound(p); }
  bool contains(const Key& p) const { return find(p) != s.end(); }
  bool intersects(const Key& l, const Key& r) const {
    if(!lt_(comp, l, r)) return false;
    auto it = s.lower_bound(l);
    if(it != s.begin()) {
      auto pit = std::prev(it);
      if(lt_(comp, l, pit->right) && lt_(comp, pit->left, r)) return true;
    }
    if(it != s.end() && lt_(comp, it->left, r) && lt_(comp, l, it->right)) return true;
    return false;
  }
  bool intersects(const bounds_type& v) const { return intersects(v.left, v.right); }
  bool covered(const Key& l, const Key& r) const {
    if(!lt_(comp, l, r)) return true;
    auto it = find(l);
    if(it == s.end()) return false;
    return le_(comp, r, it->right);
  }
  bool covered(const bounds_type& v) const { return covered(v.left, v.right); }
  Subrange<const_iterator> intersecting_intervals(const Key& l, const Key& r) const {
    if(!lt_(comp, l, r)) return Subrange(s.end(), s.end());
    auto first = s.lower_bound(l);
    if(first != s.begin()) {
      auto pit = std::prev(first);
      if(lt_(comp, l, pit->right)) first = pit;
    }
    auto last = s.lower_bound(r);
    return Subrange(first, last);
  }
  Subrange<const_iterator> intersecting_intervals(const bounds_type& v) const { return intersecting_intervals(v.left, v.right); }
  Subrange<const_iterator> included_intervals(const Key& l, const Key& r) const {
    if(!lt_(comp, l, r)) return Subrange(s.end(), s.end());
    auto first = s.lower_bound(l);
    auto it = first;
    while(it != s.end()) {
      if(!lt_(comp, it->left, r)) break;
      if(lt_(comp, r, it->right)) break;
      ++it;
    }
    return Subrange(first, it);
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
  Key mex() const {
    auto itr = find(Key{});
    if(itr == end()) return Key{};
    else return itr->right;
  }
  Key mex(const Key& p) const {
    auto itr = find(p);
    if(itr == end()) return p;
    else return itr->right;
  }
  key_compare key_comp() const { return comp; }
  value_compare value_comp() const { return value_compare(comp); }
};
} // namespace gsh
