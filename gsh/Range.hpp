#pragma once
#include <type_traits>      // std::is_class_v, std::remove_cv_t
#include <concepts>         // std::same_as, std::predicate
#include <utility>          // std::move
#include <gsh/TypeDef.hpp>  // gsh::itype

namespace gsh {

template<class D, class V>
    requires std::is_class_v<D> && std::same_as<D, std::remove_cv_t<D>>
class ViewInterface {
    constexpr D& get_ref() { return *reinterpret_cast<D*>(this); }
    constexpr const D& get_ref() const { return *reinterpret_cast<const D*>(this); }
    constexpr auto get_begin() { return get_ref().begin(); }
    constexpr auto get_begin() const { return get_ref().begin(); }
    constexpr auto get_end() { return get_ref().end(); }
    constexpr auto get_end() const { return get_ref().end(); }
public:
    using derived_type = D;
    using value_type = V;
    constexpr derived_type copy() const& { return get_ref(); }
    constexpr derived_type copy() && { return std::move(get_ref()); }
    template<std::predicate<value_type> Pred> constexpr bool all_of(Pred f) const {
        for (const auto& el : get_ref())
            if (!f(el)) return false;
        return true;
    }
    constexpr bool all_of(const value_type& x) const {
        for (const auto& el : get_ref())
            if (!(el == x)) return false;
        return true;
    }
    template<std::predicate<value_type> Pred> constexpr bool any_of(Pred f) const {
        for (const auto& el : get_ref())
            if (f(el)) return true;
        return false;
    }
    constexpr bool any_of(const value_type& x) const {
        for (const auto& el : get_ref())
            if (el == x) return true;
        return false;
    }
    template<std::predicate<value_type> Pred> constexpr bool none_of(Pred f) const {
        for (const auto& el : get_ref())
            if (f(el)) return false;
        return true;
    }
    constexpr bool none_of(const value_type& x) const {
        for (const auto& el : get_ref())
            if (el == x) return false;
        return true;
    }
    constexpr bool contains(const value_type& x) const {
        for (const auto& el : get_ref())
            if (el == x) return true;
        return false;
    }
    constexpr auto find(const value_type& x) const {
        const auto end = get_end();
        for (auto itr = get_begin(); itr != end; ++itr)
            if (*itr == x) return itr;
        return end;
    }
    constexpr itype::u32 count(const value_type& x) const {
        itype::u32 res = 0;
        for (const auto& el : get_ref()) res += (el == x);
        return res;
    }
};

}  // namespace gsh