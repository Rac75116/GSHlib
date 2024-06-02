#pragma once
#include <type_traits>        // std::is_trivially_*
#include <utility>            // std::move, std::forward
#include <initializer_list>   // std::initializer_list
#include <compare>            // std::three_way_comparable, std::compare_three_way_result
#include <tuple>              // std::tuple_size, std::tuple_element
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
    alignas(value_type) Byte buffer[sizeof(value_type)];
    bool has = false;
    template<class... Args> constexpr void construct(Args&&... args) { new (buffer) value_type(std::forward<Args>(args)...); }
    constexpr void destroy() {
        if constexpr (!std::is_trivially_destructible_v<value_type>) delete reinterpret_cast<value_type*>(buffer);
    }
    constexpr value_type& ref() { return *reinterpret_cast<value_type*>(buffer); }
    constexpr const value_type& ref() const { return *reinterpret_cast<const value_type*>(buffer); }
    template<class U> constexpr static bool is_explicit = requires(U&& rhs) { construct(std::forward<U>(rhs)); };
    constexpr static bool noexcept_swapable = []() {
        using std::swap;
        return noexcept(swap(ref(), ref()));
    }();
public:
    constexpr Option() noexcept {}
    constexpr ~Option() noexcept {
        if (has) destroy();
    }
    constexpr Option(NullOpt) noexcept {}
    constexpr Option(const Option& rhs) {
        if (rhs.has) {
            has = true;
            construct(rhs.ref());
        } else {
            has = false;
        }
    }
    constexpr Option(Option&& rhs) noexcept {
        if (rhs.has) {
            has = true;
            construct(std::move(rhs.ref()));
        } else {
            has = false;
        }
    }
    template<class... Args> constexpr explicit Option(InPlaceTag, Args&&... args) {
        has = true;
        construct(std::forward<Args>(args)...);
    }
    template<class U, class... Args> constexpr explicit Option(InPlaceTag, std::initializer_list<U> il, Args&&... args) {
        has = true;
        construct(il, std::forward<Args>(args)...);
    }
    template<class U = T>
        requires requires(U&& rhs) { static_cast<value_type>(std::forward<U>(rhs)); }
    explicit(is_explicit<U>) constexpr Option(U&& rhs) {
        has = true;
        construct(static_cast<value_type>(std::forward<U>(rhs)));
    }
    template<class U> explicit(is_explicit<U>) constexpr Option(const Option<U>& rhs) {
        if (rhs.has) {
            has = true;
            construct(static_cast<value_type>(rhs.ref()));
        } else {
            has = false;
        }
    }
    template<class U> explicit(is_explicit<U>) constexpr Option(Option<U>&& rhs) {
        if (rhs.has) {
            has = true;
            construct(static_cast<value_type>(std::move(rhs.ref())));
        } else {
            has = false;
        }
    }
    constexpr Option& operator=(NullOpt) noexcept {
        if (has) {
            destroy();
            has = false;
        }
    }
    constexpr Option& operator=(const Option& rhs) {
        if (has) destroy();
        if (rhs.has) {
            has = true;
            construct(rhs.ref());
        } else {
            has = false;
        }
    }
    template<class... Args> constexpr T& emplace(Args&&... args) {
        if (has) destroy();
        has = true;
        construct(std::forward<Args>(args)...);
        return ref();
    }
    template<class U, class... Args> constexpr T& emplace(std::initializer_list<U> il, Args&&... args) {
        if (has) destroy();
        has = true;
        construct(il, std::forward<Args>(args)...);
        return ref();
    }
    constexpr void swap(Option& rhs) noexcept(noexcept_swapable) {
        using std::swap;
        swap(has, rhs.has);
        swap(ref(), rhs.ref());
    }
    constexpr void reset() noexcept {
        if (has) {
            destroy();
            has = false;
        }
    }
    constexpr T& operator*() & { return ref(); }
    constexpr T&& operator*() && { return std::move(ref()); }
    constexpr const T& operator*() const& { return ref(); }
    constexpr const T&& operator*() const&& { return std::move(ref()); }
    constexpr const T* operator->() const { return reinterpret_cast<const T*>(buffer); }
    constexpr T* operator->() { return reinterpret_cast<T*>(buffer); }
    constexpr explicit operator bool() const noexcept { return has; }
    constexpr bool has_value() const noexcept { return has; }
    constexpr const T& value() const& {
        if (!has) throw gsh::Exception("gsh::Option::value / The object doesn't have value.");
        return ref();
    }
    constexpr T& value() & {
        if (!has) throw gsh::Exception("gsh::Option::value / The object doesn't have value.");
        return ref();
    }
    constexpr T&& value() && {
        if (!has) throw gsh::Exception("gsh::Option::value / The object doesn't have value.");
        return std::move(ref());
    }
    constexpr const T&& value() const&& {
        if (!has) throw gsh::Exception("gsh::Option::value / The object doesn't have value.");
        return std::move(ref());
    }
    template<class U> constexpr T value_or(U&& v) const& { return has ? ref() : static_cast<T>(std::forward<U>(v)); }
    template<class U> constexpr T value_or(U&& v) && { return has ? static_cast<T>(std::move(ref())) : static_cast<T>(std::forward<U>(v)); }
};
template<class T> Option(T) -> Option<T>;

template<class T, class U> constexpr bool operator==(const Option<T>& x, const Option<U>& y) {
    if (x.has_value() && y.has_value()) return *x == *y;
    else return x.has_value() ^ y.has_value();
}
template<class T> constexpr bool operator==(const Option<T>& x, NullOpt) noexcept {
    return !x.has_value();
}
template<class T, class U> constexpr bool operator==(const Option<T>& x, const U& y) {
    return x.has_value() && (*x == y);
}
template<class T, class U> constexpr bool operator==(const T& x, const Option<U>& y) {
    return y.has_value() && (x == *y);
}
template<class T, std::three_way_comparable_with<T> U> constexpr std::compare_three_way_result_t<T, U> operator<=>(const Option<T>& x, const Option<U>& y) {
    return x.has_value() && y.has_value() ? *x <=> *y : x.has_value() <=> y.has_value();
}
template<class T> constexpr std::strong_ordering operator<=>(const Option<T>& x, NullOpt) noexcept {
    return x.has_value <=> false;
}
template<class T, std::three_way_comparable_with<T> U> constexpr std::compare_three_way_result_t<T, U> operator<=>(const Option<T>& x, const U& y) {
    return x.has_value() ? *x <=> y : std::strong_ordering::less;
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
    if constexpr (N == 0) return *x;
    else return x.has_value();
}
template<std::size_t N, class T> auto get(Option<T>&& x) {
    if constexpr (N == 0) return *std::move(x);
    else return x.has_value();
}
}  // namespace gsh