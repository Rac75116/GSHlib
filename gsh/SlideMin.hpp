#pragma once
#include "Functional.hpp"
#include <deque>
#include <utility>
namespace gsh {
template<class T, class Operator = Less> class SlideMin {
  std::deque<std::pair<T, u32>> deq;
  u32 cnt1 = 0, cnt2 = 0;
  [[no_unique_address]] Operator op;
public:
  constexpr SlideMin() : op(Operator()) {}
  constexpr SlideMin(const Operator& op_) : op(op_) {}
  constexpr SlideMin(Operator&& op_) : op(std::move(op_)) {}
  constexpr void push(const T& x) {
    while(!deq.empty() && std::invoke(op, x, deq.back().first)) deq.pop_back();
    deq.emplace_back(x, ++cnt1);
  }
  constexpr void pop() {
    if(deq.front().second == (++cnt2)) deq.pop_front();
  }
  constexpr const T& get() const { return deq.front().first; }
  constexpr bool empty() const { return deq.empty(); }
  constexpr u32 size() const { return cnt1 - cnt2; }
  constexpr void clear() {
    deq.clear();
    cnt1 = cnt2 = 0;
  }
};
}
