#pragma once
#include <type_traits>      // std::is_class_v, std::remove_cv_t
#include <concepts>         // std::same_as, std::predicate
#include <utility>          // std::move
#include <iterator>         // std::next, std::iter_value_t
#include <ranges>           // std::ranges::iterator_t, std::sentinel_for, std::ranges::reverse
#include <gsh/TypeDef.hpp>  // gsh::itype

namespace gsh {

template<class D, class V>
    requires std::is_class_v<D> && std::same_as<D, std::remove_cv_t<D>>
class ViewInterface;

template<class Iter> class SlicedRange : public ViewInterface<SlicedRange<Iter>, std::iter_value_t<Iter>> {
public:
    using iterator = Iter;
    using value_type = std::iter_value_t<Iter>;
    static_assert(std::sentinel_for<iterator, iterator>, "gsh::SlicedRange / The iterator cannot behave as sentinel.");
private:
    iterator first, last;
public:
    constexpr SlicedRange(iterator beg, iterator end) : first(beg), last(end) {}
    constexpr iterator begin() const { return first; }
    constexpr iterator end() const { return last; }
    constexpr auto rbegin() const { return std::reverse_iterator{ last }; }
    constexpr auto rend() const { return std::reverse_iterator{ first }; }
};

template<class D, class V>
    requires std::is_class_v<D> && std::same_as<D, std::remove_cv_t<D>>
class ViewInterface {
    constexpr D& get_ref() { return *reinterpret_cast<D*>(this); }
    constexpr const D& get_ref() const { return *reinterpret_cast<const D*>(this); }
    constexpr auto get_begin() { return get_ref().begin(); }
    constexpr auto get_begin() const { return get_ref().cbegin(); }
    constexpr auto get_end() { return get_ref().end(); }
    constexpr auto get_end() const { return get_ref().cend(); }
    constexpr auto get_rbegin() { return get_ref().rbegin(); }
    constexpr auto get_rbegin() const { return get_ref().crbegin(); }
    constexpr auto get_rend() { return get_ref().rend(); }
    constexpr auto get_rend() const { return get_ref().crend(); }
public:
    using derived_type = D;
    using value_type = V;
    constexpr derived_type copy() const& { return get_ref(); }
    constexpr derived_type copy() & { return get_ref(); }
    constexpr derived_type copy() && { return std::move(get_ref()); }
    constexpr auto slice(itype::u32 a, itype::u32 b) {
        auto beg = std::next(get_begin(), a);
        auto end = std::next(beg, b - a);
        return SlicedRange{ beg, end };
    }
    constexpr auto slice(itype::u32 a, itype::u32 b) const {
        auto beg = std::next(get_begin(), a);
        auto end = std::next(beg, b - a);
        return SlicedRange{ beg, end };
    }
    constexpr auto slice(itype::u32 a) { return SlicedRange{ std::next(get_begin(), a), get_end() }; }
    constexpr auto slice(itype::u32 a) const { return SlicedRange{ std::next(get_begin(), a), get_end() }; }
    constexpr derived_type& reverse() {
        std::ranges::reverse(get_ref());
        return get_ref();
    }
    constexpr auto reversed() { return SlicedRange{ get_rbegin(), get_rend() }; }
    constexpr auto reversed() const { return SlicedRange{ get_rbegin(), get_rend() }; }
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