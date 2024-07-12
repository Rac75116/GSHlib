#pragma once
#include <concepts>         // std::totally_ordered_with, std::same_as, std::integral, std::floating_point
#include <type_traits>      // std::remove_cv_t, std::remove_cvref_t
#include <utility>          // std::forward
#include <cstddef>          // std::nullptr_t
#include <gsh/TypeDef.hpp>  // gsh::itype

namespace gsh {

namespace internal {
    template<typename T, typename U> concept less_builtin_ptr_cmp = requires(T&& t, U&& u) {
        { t < u } -> std::same_as<bool>;
    } && std::convertible_to<T, const volatile void*> && std::convertible_to<U, const volatile void*> && (!requires(T&& t, U&& u) { operator<(std::forward<T>(t), std::forward<U>(u)); } && !requires(T&& t, U&& u) { std::forward<T>(t).operator<(std::forward<U>(u)); });
}  // namespace internal
class Less {
public:
    template<class T, class U>
        requires std::totally_ordered_with<T, U>
    constexpr bool operator()(T&& t, U&& u) const noexcept(noexcept(std::declval<T>() < std::declval<U>())) {
        if constexpr (internal::less_builtin_ptr_cmp<T, U>) {
            if (std::is_constant_evaluated()) return t < u;
            auto x = reinterpret_cast<itype::u64>(static_cast<const volatile void*>(std::forward<T>(t)));
            auto y = reinterpret_cast<itype::u64>(static_cast<const volatile void*>(std::forward<U>(u)));
            return x < y;
        } else return std::forward<T>(t) < std::forward<U>(u);
    }
    using is_transparent = void;
};
class Greater {
public:
    template<class T, class U>
        requires std::totally_ordered_with<T, U>
    constexpr bool operator()(T&& t, U&& u) const noexcept(noexcept(std::declval<U>() < std::declval<T>())) {
        if constexpr (internal::less_builtin_ptr_cmp<U, T>) {
            if (std::is_constant_evaluated()) return u < t;
            auto x = reinterpret_cast<itype::u64>(static_cast<const volatile void*>(std::forward<T>(t)));
            auto y = reinterpret_cast<itype::u64>(static_cast<const volatile void*>(std::forward<U>(u)));
            return y < x;
        } else return std::forward<U>(u) < std::forward<T>(t);
    }
    using is_transparent = void;
};

class Identity {
public:
    template<class T> [[nodiscard]]
    constexpr T&& operator()(T&& t) const noexcept {
        return std::forward<T>(t);
    }
    using is_transparent = void;
};

}  // namespace gsh

namespace std {
template<class T> class hash;
}

namespace gsh {

namespace internal {
    template<class T> concept Nocvref = std::same_as<T, std::remove_cv_t<T>> && !std::is_reference_v<T>;
    constexpr itype::u64 MixIntegers(itype::u64 a, itype::u64 b) {
        itype::u128 tmp = static_cast<itype::u128>(a) * b;
        return static_cast<itype::u64>(tmp) ^ static_cast<itype::u64>(tmp >> 64);
    }
}  // namespace internal
template<class T> class Hash : public std::hash<T> {};
// https://raw.githubusercontent.com/martinus/unordered_dense/v1.3.0/include/ankerl/unordered_dense.h
template<> class Hash<itype::u64> {
public:
    constexpr itype::u64 operator()(itype::u64 x) const noexcept { return internal::MixIntegers(x, 0x9e3779b97f4a7c15); }
};
template<internal::Nocvref T>
    requires std::integral<T>
class Hash<T> {
public:
    constexpr itype::u64 operator()(const T& x) const noexcept { return Hash<itype::u64>{}(static_cast<itype::u64>(x)); }
};
template<internal::Nocvref T>
    requires std::floating_point<T>
class Hash<T> {
public:
    constexpr itype::u64 operator()(const T& x) const noexcept {
        if constexpr (sizeof(T) <= 8) {
            union {
                itype::u64 a = 0;
                T b;
            };
            b = x;
            return Hash<itype::u64>{}(a);
        } else {
            union {
                itype::u128 a = 0;
                T b;
            };
            b = x;
            return Hash<itype::u128>{}(a);
        }
    }
};
template<class T> class Hash<T*> {
public:
    constexpr itype::u64 operator()(T* x) const noexcept {
        static_assert(sizeof(x) == 4 || sizeof(x) == 8);
        if constexpr (sizeof(x) == 8) return Hash<itype::u64>{}(reinterpret_cast<itype::u64>(x));
        else return Hash<itype::u32>{}(reinterpret_cast<itype::u32>(x));
    }
};
template<> class Hash<std::nullptr_t> {
public:
    constexpr itype::u64 operator()(const std::nullptr_t& x) const noexcept { return Hash<void*>{}(static_cast<void*>(x)); }
};
template<class T> class Hash<T&> {
public:
    constexpr itype::u64 operator()(const T& x) const {
        using val = std::remove_cvref_t<decltype(&x)>;
        return Hash<val>{}(static_cast<val>(&x));
    }
};
template<class T> class Hash<T&&> {
public:
    constexpr itype::u64 operator()(const T&& x) const {
        using val = std::remove_cvref_t<decltype(&x)>;
        return Hash<val>{}(static_cast<val>(&x));
    }
};

namespace internal {
    constexpr itype::u64 HashBytes(const ctype::c8* ptr, itype::u32 len) noexcept {
        constexpr itype::u64 m = 0xc6a4a7935bd1e995;
        constexpr itype::u64 seed = 0xe17a1465;
        constexpr itype::u32 r = 47;
        itype::u64 h = seed ^ (len * m);
        const itype::u32 n_blocks = len / 8;
        for (itype::u64 i = 0; i < n_blocks; ++i) {
            itype::u64 k;
            const auto p = ptr + i * 8;
            if (std::is_constant_evaluated()) {
                k = 0;
                for (itype::u32 j = 0; j != 8; ++j) k |= static_cast<itype::u64>(p[j]) << (8 * j);
            } else {
                for (int j = 0; j != 8; ++j) *(reinterpret_cast<ctype::c8*>(&k) + j) = *(p + j);
            }
            k *= m;
            k ^= k >> r;
            k *= m;
            h ^= k;
            h *= m;
        }
        const auto data8 = ptr + n_blocks * 8;
        switch (len & 7u) {
        case 7 : h ^= static_cast<itype::u64>(data8[6]) << 48U; [[fallthrough]];
        case 6 : h ^= static_cast<itype::u64>(data8[5]) << 40U; [[fallthrough]];
        case 5 : h ^= static_cast<itype::u64>(data8[4]) << 32U; [[fallthrough]];
        case 4 : h ^= static_cast<itype::u64>(data8[3]) << 24U; [[fallthrough]];
        case 3 : h ^= static_cast<itype::u64>(data8[2]) << 16U; [[fallthrough]];
        case 2 : h ^= static_cast<itype::u64>(data8[1]) << 8U; [[fallthrough]];
        case 1 :
            h ^= static_cast<itype::u64>(data8[0]);
            h *= m;
            [[fallthrough]];
        default : break;
        }
        h ^= h >> r;
        return h;
    }
    constexpr itype::u64 HashBytes(const ctype::c8* ptr) noexcept {
        auto last = ptr;
        while (*last != '\0') ++last;
        return HashBytes(ptr, last - ptr);
    }
}  // namespace internal

}  // namespace gsh
