#pragma once
#include <ranges>  // std::range
#include <algorithm>
#include <gsh/TypeDef.hpp>  // gsh::itype

namespace gsh {

namespace internal {
    template<class Base> class Container : public Base {
    public:
        using Base::Base;
        constexpr Container copy() const { return *this; }
        template<class Pred> constexpr bool all_of(Pred f) const {
            for (const auto& el : *this)
                if (!f(el)) return false;
            return true;
        }
        constexpr bool all_of(const Base::value_type& x) const {
            for (const auto& el : *this)
                if (!(el == x)) return false;
            return true;
        }
        template<class Pred> constexpr bool any_of(Pred f) const {
            for (const auto& el : *this)
                if (f(el)) return true;
            return false;
        }
        constexpr bool any_of(const Base::value_type& x) const {
            for (const auto& el : *this)
                if (el == x) return true;
            return false;
        }
        template<class Pred> constexpr bool none_of(Pred f) const {
            for (const auto& el : *this)
                if (f(el)) return false;
            return true;
        }
        constexpr bool none_of(const Base::value_type& x) const {
            for (const auto& el : *this)
                if (el == x) return false;
            return true;
        }
        constexpr bool contains(const Base::value_type& x) const {
            for (const auto& el : *this)
                if (el == x) return true;
            return false;
        }
        constexpr auto find(const Base::value_type& x) const {
            const auto end = Base::cend();
            for (auto itr = Base::cbegin(); itr != end; ++itr)
                if (*itr == x) return itr;
            return end;
        }
        constexpr itype::u32 count(const Base::value_type& x) const {
            if constexpr (requires(const Base& c, const Base::value_type& t) { c.count(t); }) {
                return Base::count(x);
            } else {
                itype::u32 res = 0;
                for (const auto& el : *this) res += el == x;
                return res;
            }
        }
    };
}  // namespace internal

}  // namespace gsh