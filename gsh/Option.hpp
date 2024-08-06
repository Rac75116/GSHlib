#pragma once
#include <type_traits>        // std::is_trivially_(***), std::integral_constant, std::is_convertible_v
#include <utility>            // std::move, std::forward, std::declval
#include <initializer_list>   // std::initializer_list
#include <compare>            // std::three_way_comparable, std::compare_three_way_result
#include <tuple>              // std::tuple_size, std::tuple_element
#include <memory>             // std::construct_at, std::destroy_at
#include <gsh/Util.hpp>       // gsh::InPlace
#include <gsh/Exception.hpp>  // gsh::Exception

namespace gsh {

class NullOpt {};
constexpr NullOpt Null;
template<class T> class Option {
    template<class U> friend class Option;
public:
    using value_type = T;
private:
    union {
        value_type val_;
    };
    bool has;
    template<class... Args> constexpr void construct(Args&&... args) { std::construct_at(std::addressof(val_), std::forward<Args>(args)...); }
    constexpr void destroy() { std::destroy_at(std::addressof(val_)); }
    template<class U> constexpr static bool is_explicit = !std::is_convertible_v<U, T>;
    constexpr static bool noexcept_swapable = []() {
        using std::swap;
        return noexcept(swap(std::declval<value_type&>(), std::declval<value_type&>()));
    }();
public:
    constexpr Option() noexcept : has(false) {}
    constexpr Option(NullOpt) noexcept : has(false) {}
    constexpr Option(const Option& rhs) {
        if (rhs.has) has = true, construct(rhs.val_);
        else has = false;
    }
    constexpr Option(Option&& rhs) noexcept {
        if (rhs.has) has = true, construct(std::move(rhs.val_));
        else has = false;
    }
    template<class... Args> constexpr explicit Option(InPlaceTag, Args&&... args) : has(true) { construct(std::forward<Args>(args)...); }
    template<class U, class... Args> constexpr explicit Option(InPlaceTag, std::initializer_list<U> il, Args&&... args) : has(true) { construct(il, std::forward<Args>(args)...); }
    template<class U = T>
        requires requires(U&& rhs) { static_cast<value_type>(std::forward<U>(rhs)); }
    explicit(is_explicit<U>) constexpr Option(U&& rhs) : has(true) {
        construct(std::forward<U>(rhs));
    }
    template<class U> explicit(is_explicit<U>) constexpr Option(const Option<U>& rhs) {
        if (rhs.has) has = true, construct(static_cast<value_type>(rhs.val_));
        else has = false;
    }
    template<class U> explicit(is_explicit<U>) constexpr Option(Option<U>&& rhs) {
        if (rhs.has) has = true, construct(static_cast<value_type>(std::move(rhs.val_)));
        else has = false;
    }
    constexpr ~Option() noexcept {
        if (has) destroy();
    }
    constexpr Option& operator=(NullOpt) noexcept {
        if (has) destroy(), has = false;
    }
    constexpr Option& operator=(const Option& rhs) {
        if (has) destroy();
        if (rhs.has) has = true, construct(rhs.ref());
        else has = false;
    }
    template<class... Args> constexpr T& emplace(Args&&... args) {
        if (has) destroy();
        has = true;
        construct(std::forward<Args>(args)...);
        return val_;
    }
    template<class U, class... Args> constexpr T& emplace(std::initializer_list<U> il, Args&&... args) {
        if (has) destroy();
        has = true;
        construct(il, std::forward<Args>(args)...);
        return val_;
    }
    constexpr void swap(Option& rhs) noexcept(noexcept_swapable) {
        using std::swap;
        swap(has, rhs.has);
        swap(val_, rhs.val_);
    }
    constexpr void reset() noexcept {
        if (has) destroy(), has = false;
    }
    constexpr T& operator*() & { return val_; }
    constexpr T&& operator*() && { return std::move(val_); }
    constexpr const T& operator*() const& { return val_; }
    constexpr const T&& operator*() const&& { return std::move(val_); }
    constexpr const T* operator->() const { return std::addressof(val_); }
    constexpr T* operator->() { return std::addressof(val_); }
    constexpr explicit operator bool() const noexcept { return has; }
    constexpr bool has_val() const noexcept { return has; }
    constexpr const T& val() const& {
        if (!has) throw gsh::Exception("gsh::Option::val / The object doesn't have value.");
        return val_;
    }
    constexpr T& val() & {
        if (!has) throw gsh::Exception("gsh::Option::val / The object doesn't have value.");
        return val_;
    }
    constexpr T&& val() && {
        if (!has) throw gsh::Exception("gsh::Option::val / The object doesn't have value.");
        return std::move(val_);
    }
    constexpr const T&& val() const&& {
        if (!has) throw gsh::Exception("gsh::Option::val / The object doesn't have value.");
        return std::move(val_);
    }
    template<class U> constexpr T val_or(U&& v) const& {
        if (has) return val_;
        else return std::forward<U>(v);
    }
    template<class U> constexpr T val_or(U&& v) && {
        if (has) return std::move(val_);
        else return std::forward<U>(v);
    }
};
template<class T> Option(T) -> Option<T>;

template<class T, class U> constexpr bool operator==(const Option<T>& x, const Option<U>& y) {
    if (x.has_val() && y.has_val()) return *x == *y;
    else return x.has_val() ^ y.has_val();
}
template<class T> constexpr bool operator==(const Option<T>& x, NullOpt) noexcept {
    return !x.has_val();
}
template<class T, class U> constexpr bool operator==(const Option<T>& x, const U& y) {
    return x.has_val() && (*x == y);
}
template<class T, class U> constexpr bool operator==(const T& x, const Option<U>& y) {
    return y.has_val() && (x == *y);
}
template<class T, std::three_way_comparable_with<T> U> constexpr std::compare_three_way_result_t<T, U> operator<=>(const Option<T>& x, const Option<U>& y) {
    return x.has_val() && y.has_val() ? *x <=> *y : x.has_val() <=> y.has_val();
}
template<class T> constexpr std::strong_ordering operator<=>(const Option<T>& x, NullOpt) noexcept {
    return x.has_val() <=> false;
}
template<class T, std::three_way_comparable_with<T> U> constexpr std::compare_three_way_result_t<T, U> operator<=>(const Option<T>& x, const U& y) {
    return x.has_val() ? *x <=> y : std::strong_ordering::less;
}

}  // namespace gsh

namespace std {
template<class T> struct tuple_size<gsh::Option<T>> : integral_constant<size_t, 2> {};
template<class T> struct tuple_element<0, gsh::Option<T>> {
    using type = T;
};
template<class T> struct tuple_element<1, gsh::Option<T>> {
    using type = bool;
};
}  // namespace std

namespace gsh {
template<std::size_t N, class T> auto get(const Option<T>& x) {
    if constexpr (N == 0) {
        if (x.has_val()) return *x;
        else return T();
    } else return x.has_val();
}
template<std::size_t N, class T> auto get(Option<T>& x) {
    if constexpr (N == 0) {
        if (x.has_val()) return *x;
        else return T();
    } else return x.has_val();
}
template<std::size_t N, class T> auto get(Option<T>&& x) {
    if constexpr (N == 0) {
        if (x.has_val()) return std::move(*x);
        else return T();
    } else return x.has_val();
}
}  // namespace gsh