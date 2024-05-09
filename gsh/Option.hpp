#pragma once
#include <type_traits>       // std::is_trivially_*
#include <utility>           // std::move, std::forward
#include <initializer_list>  // std::initializer_list
#include <gsh/Util.hpp>      // gsh::InPlace

namespace gsh {

class NullOpt {};
constexpr NullOpt Null;
template<class T> class Option {
public:
    using value_type = T;
private:
    bool has = false;
    alignas(value_type) Byte buffer[sizeof(value_type)];
    template<class... Args> constexpr void construct(Args&&... args) {
        if constexpr (!std::is_trivially_constructible_v<T, Args...>) new (buffer) value_type(std::forward(args)...);
    }
    constexpr void destroy() {
        if constexpr (!std::is_trivially_destructible_v<value_type>) delete reinterpret_cast<value_type*>(buffer);
    }
    constexpr value_type& ref() { return *reinterpret_cast<value_type*>(buffer); }
    constexpr bool is_explicit = requires(U&& rhs) { construct(std::move(rhs)); };
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
        construct(std::forward(args)...);
    }
    template<class U, class... Args> constexpr explicit Option(InPlaceTag, std::initializer_list<U> il, Args&&... args) {
        has = true;
        construct(il, std::forward(args)...);
    }
    template<class U = T> explicit(is_explicit) constexpr Option(U&& rhs) {
        has = true;
        construct(static_cast<value_type>(std::move(rhs)));
    }
    template<class U> explicit(is_explicit) constexpr Option(const Option<U>& rhs) {
        if (rhs.has) {
            has = true;
            construct(static_cast<value_type>(rhs.ref()));
        } else {
            has = false;
        }
    }
    template<class U> explicit(is_explicit) constexpr Option(Option<U>&& rhs) {
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
        construct(std::forward(args)...);
        return ref();
    }
    template<class U, class... Args> constexpr T& emplace(std::initializer_list<U> il, Args&&... args) {
        if (has) destroy();
        has = true;
        construct(il, std::forward(args)...);
        return ref();
    }
    constexpr bool has_value() const noexcept { return has; }
};
template<class T> Option(T) -> Option<T>;

template<class T> Option<T> Some(const T& x) {
    return Option<T>{ x };
}
template<class T> Option<T> Some(T&& x) {
    return Option<T>{ std::move(x) };
}

}  // namespace gsh
