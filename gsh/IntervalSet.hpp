#pragma once
#include "Range.hpp"
#include "Vec.hpp"
#include <set>
namespace gsh {
template<class T> class IntervalSet : public ViewInterface<IntervalSet<T>, std::pair<T, T>> {
public:
  using value_type = std::pair<T, T>; // [first, second)
  using key_type = value_type;
  using key_compare = Less;
  using value_compare = key_compare;
  using size_type = std::size_t;
  using difference_type = std::ptrdiff_t;
  using reference = value_type&;
  using const_reference = const value_type&;
  using iterator = typename std::set<value_type>::iterator;
  using const_iterator = typename std::set<value_type>::const_iterator;
private:
  std::set<value_type, key_compare> s_;
public:
  IntervalSet() = default;
  bool empty() const noexcept { return s_.empty(); }
  size_type size() const noexcept { return s_.size(); }
  void clear() noexcept { s_.clear(); }
  iterator begin() noexcept { return s_.begin(); }
  const_iterator begin() const noexcept { return s_.begin(); }
  const_iterator cbegin() const noexcept { return s_.cbegin(); }
  iterator end() noexcept { return s_.end(); }
  const_iterator end() const noexcept { return s_.end(); }
  const_iterator cend() const noexcept { return s_.cend(); }
  std::pair<iterator, bool> insert(T l, T r) {
    if(!(l < r)) return {s_.end(), false};
    auto it = s_.lower_bound({l, l});
    if(it != s_.begin()) {
      auto prev_it = std::prev(it);
      if(prev_it->second >= l) {
        l = std::min(l, prev_it->first);
        r = std::max(r, prev_it->second);
        it = s_.erase(prev_it);
      }
    }
    while(it != s_.end() && it->first <= r) {
      l = std::min(l, it->first);
      r = std::max(r, it->second);
      it = s_.erase(it);
    }
    auto res_it = s_.emplace(l, r).first;
    return {res_it, true};
  }
  size_type erase(T l, T r) {
    if(!(l < r)) return 0;
    size_type erased_cnt = 0;
    auto it = s_.lower_bound({l, l});
    if(it != s_.begin()) {
      auto prev_it = std::prev(it);
      if(prev_it->second > l) {
        auto a = prev_it->first;
        auto b = prev_it->second;
        s_.erase(prev_it);
        ++erased_cnt;
        if(a < l) { s_.emplace(a, l); }
        if(r < b) {
          s_.emplace(r, b);
          return erased_cnt;
        }
      }
    }
    it = s_.lower_bound({l, l});
    while(it != s_.end() && it->first < r) {
      auto a = it->first;
      auto b = it->second;
      it = s_.erase(it);
      ++erased_cnt;
      if(r < b) {
        s_.emplace(r, b);
        break;
      }
    }
    return erased_cnt;
  }
  bool covered(T l, T r) const {
    if(!(l <= r)) return false;
    if(l == r) return true;
    auto it = find_point(l);
    if(it == s_.end()) return false;
    return r <= it->second;
  }
  T mex(T p = T()) const {
    auto it = s_.upper_bound({p, std::numeric_limits<T>::max()});
    if(it == s_.begin()) return p;
    --it;
    if(it->first <= p && p < it->second) return it->second;
    return p;
  }
};
} // namespace gsh
