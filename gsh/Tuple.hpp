#pragma once
#include <type_traits>
#include <tuple>
#include <utility>      // std::forward, std::make_integer_sequence
#include "TypeDef.hpp"  // gsh::itype

namespace gsh {

namespace internal {

    template<class T, itype::u32 N> struct TupleEl {
    private:
        T element;
    protected:
        template<class... Args> TupleEl(Args&&... args) : element{ std::forward<Args>(args)... } {}
        constexpr T& el() noexcept { return element; }
        constexpr const T& el() const noexcept { return element; }
    };
    // clang-format off
#define GSH_INTERNAL_DEF_TUPLEEL(n, name) \
    template<class T> struct TupleEl<T, n> { \
        T name; \
    protected: \
        template<class... Args> TupleEl(Args&&... args) : name{ std::forward<Args>(args)... } {} \
        constexpr T& el() noexcept { return name; } \
        constexpr const T& el() const noexcept { return name; } \
    };
GSH_INTERNAL_DEF_TUPLEEL(0, first);
GSH_INTERNAL_DEF_TUPLEEL(1, second);
GSH_INTERNAL_DEF_TUPLEEL(2, third);
GSH_INTERNAL_DEF_TUPLEEL(3, fourth);
GSH_INTERNAL_DEF_TUPLEEL(4, fifth);
GSH_INTERNAL_DEF_TUPLEEL(5, sixth);
GSH_INTERNAL_DEF_TUPLEEL(6, seventh);
GSH_INTERNAL_DEF_TUPLEEL(7, eighth);
GSH_INTERNAL_DEF_TUPLEEL(8, ninth);
GSH_INTERNAL_DEF_TUPLEEL(9, tenth);
GSH_INTERNAL_DEF_TUPLEEL(10, eleventh);
GSH_INTERNAL_DEF_TUPLEEL(11, twelfth);
GSH_INTERNAL_DEF_TUPLEEL(12, thirteenth);
GSH_INTERNAL_DEF_TUPLEEL(13, fourteenth);
GSH_INTERNAL_DEF_TUPLEEL(14, fifteenth);
GSH_INTERNAL_DEF_TUPLEEL(15, sixteenth);
GSH_INTERNAL_DEF_TUPLEEL(16, seventeenth);
GSH_INTERNAL_DEF_TUPLEEL(17, eighteenth);
GSH_INTERNAL_DEF_TUPLEEL(18, nineteenth);
GSH_INTERNAL_DEF_TUPLEEL(19, twentieth);
    // clang-format on
    template<itype::u32 N, class... Args> class TupleImpl {};
    template<itype::u32 N, class T, class... Args> class TupleImpl<N, T, Args...> : public TupleEl<T, N>, public TupleImpl<N + 1, Args...> {
    protected:
        template<itype::u32 M> GSH_INTERNAL_INLINE auto get_by_idx() {
            if constexpr (M == N) return TupleEl<T, N>::el();
            else TupleImpl<N + 1, Args...>::template get_by_idx<M>();
        }
        template<itype::u32 M> GSH_INTERNAL_INLINE auto get_by_idx() const {
            if constexpr (M == N) return TupleEl<T, N>::el();
            else TupleImpl<N + 1, Args...>::template get_by_idx<M>();
        }
    };
}  // namespace internal

template<class... Types> class Tuple : public internal::TupleImpl<0, Types...> {
    using base = internal::TupleImpl<0, Types...>;
    template<class Tup, itype::u32... I, class... UTypes> constexpr bool enable_construction(Tup&& u, std::integer_sequence<itype::u32, I...>, Tuple<UTypes...>) {
        if (sizeof...(Types) != sizeof(UTypes)) return false;
        else if constexpr (!std::is_constructible_v<Types, decltype(get<I>(static_cast<decltype(u)>(u)))> && ...) return false;
        else if constexpr (sizeof...(Types) != 1) return true;
        else return (!std::is_convertible_v<decltype(u), Types...> && !std::is_constructible_v<Types..., decltype(u)> && !std::is_same_v<Types..., UTypes...>);
    }
public:
    explicit(see below) constexpr Tuple()
        requires std::conjunction_v<std::is_default_constructible<Types>...>
    = default;
    explicit(see below) constexpr Tuple(const Types&...)
        requires std::conjunction_v<std::is_copy_constructible<Types>...>;
    template<class... UTypes>
        requires(sizeof...(Types) >= 1 && sizeof...(Types) == sizeof...(UTypes) && std::conjunction_v<std::conditional_t<sizeof...(Types) == 1, std::negation<std::is_same<std::remove_cvref_t<Utypes...>, std::true_type>>>, std::is_constructible<Types, UTypes>...>)
    explicit(see below) constexpr Tuple(UTypes&&...);
    constexpr Tuple(const Tuple&)
        requires std::conjunction_v<std::is_copy_constructible<Types>...>
    = default;
    constexpr Tuple(Tuple&&)
        requires std::conjunction_v<std::is_move_constructible<Types>...>
    = default;
    template<class... UTypes> explicit(see below) constexpr Tuple(Tuple<UTypes...>& u)
        requires(enable_construction(u, std::make_integer_sequence<itype::u32, sizeof...(Types)>{}, u));
    template<class... UTypes> explicit(see below) constexpr Tuple(const Tuple<UTypes...>&)
        requires(enable_construction(u, std::make_integer_sequence<itype::u32, sizeof...(Types)>{}, u));
    template<class... UTypes> explicit(see below) constexpr Tuple(Tuple<UTypes...>&&)
        requires(enable_construction(u, std::make_integer_sequence<itype::u32, sizeof...(Types)>{}, u));
    template<class... UTypes> explicit(see below) constexpr Tuple(const Tuple<UTypes...>&&)
        requires(enable_construction(u, std::make_integer_sequence<itype::u32, sizeof...(Types)>{}, u));
    //template<tuple - like UTuple> explicit(see below) constexpr Tuple(UTuple&&);
    /*
    template<class Alloc> explicit(see below) constexpr Tuple(allocator_arg_t, const Alloc& a);
    template<class Alloc> explicit(see below) constexpr Tuple(allocator_arg_t, const Alloc& a, const Types&...);
    template<class Alloc, class... UTypes> explicit(see below) constexpr Tuple(allocator_arg_t, const Alloc& a, UTypes&&...);
    template<class Alloc> constexpr Tuple(allocator_arg_t, const Alloc& a, Tuple&&);
    template<class Alloc, class... UTypes> explicit(see below) constexpr Tuple(allocator_arg_t, const Alloc& a, Tuple<UTypes...>&);
    template<class Alloc, class... UTypes> explicit(see below) constexpr Tuple(allocator_arg_t, const Alloc& a, const Tuple<UTypes...>&);
    template<class Alloc, class... UTypes> explicit(see below) constexpr Tuple(allocator_arg_t, const Alloc& a, Tuple<UTypes...>&&);
    template<class Alloc, class... UTypes> explicit(see below) constexpr Tuple(allocator_arg_t, const Alloc& a, const Tuple<UTypes...>&&);
    template<class Alloc, tuple - like UTuple> explicit(see below) constexpr Tuple(allocator_arg_t, const Alloc& a, UTuple&&);
    */
};
template<class T, class U> using Pair = Tuple<T, U>;

}  // namespace gsh
