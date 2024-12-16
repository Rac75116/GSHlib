#pragma once
#include <type_traits>             // std::is_class_v, std::remove_cv_t, std::common_reference_t, std::is_lvalue_reference
#include <concepts>                // std::same_as, std::predicate, std::convertible_to
#include <utility>                 // std::move, std::declval
#include <iterator>                // std::next, std::iter_value_t, std::iter_reference_t, std::input_iterator
#include <ranges>                  // std::ranges::iterator_t, std::sentinel_for, std::ranges::reverse, std::ranges::range, std::ranges::range_value_t, std::ranges::subrange_kind, std::sized_sentinel_for
#include <tuple>                   // std::tuple_element
#include "TypeDef.hpp"             // gsh::itype
#include "internal/Operation.hpp"  //gsh::internal::IteratorInterface

namespace gsh {

enum class RangeKind { Sized, Unsized };

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
    constexpr D& derived() { return *static_cast<D*>(this); }
    constexpr const D& derived() const { return *static_cast<const D*>(this); }
    constexpr auto get_begin() { return derived().begin(); }
    constexpr auto get_begin() const { return derived().cbegin(); }
    constexpr auto get_end() { return derived().end(); }
    constexpr auto get_end() const { return derived().cend(); }
    constexpr auto get_rbegin() { return derived().rbegin(); }
    constexpr auto get_rbegin() const { return derived().crbegin(); }
    constexpr auto get_rend() { return derived().rend(); }
    constexpr auto get_rend() const { return derived().crend(); }
public:
    using derived_type = D;
    using value_type = V;
    constexpr derived_type copy() const& { return derived(); }
    constexpr derived_type copy() & { return derived(); }
    constexpr derived_type copy() && { return std::move(derived()); }
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
    template<std::predicate<value_type> Pred> constexpr bool all_of(Pred f) const {
        for (const auto& el : derived())
            if (!f(el)) return false;
        return true;
    }
    constexpr bool all_of(const value_type& x) const {
        for (const auto& el : derived())
            if (!(el == x)) return false;
        return true;
    }
    template<std::predicate<value_type> Pred> constexpr bool any_of(Pred f) const {
        for (const auto& el : derived())
            if (f(el)) return true;
        return false;
    }
    constexpr bool any_of(const value_type& x) const {
        for (const auto& el : derived())
            if (el == x) return true;
        return false;
    }
    template<std::predicate<value_type> Pred> constexpr bool none_of(Pred f) const {
        for (const auto& el : derived())
            if (f(el)) return false;
        return true;
    }
    constexpr bool none_of(const value_type& x) const {
        for (const auto& el : derived())
            if (el == x) return false;
        return true;
    }
    constexpr bool contains(const value_type& x) const {
        for (const auto& el : derived())
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
        for (const auto& el : derived()) res += (el == x);
        return res;
    }
};

namespace internal {
    template<class T, class U> concept difference_from = std::same_as<std::remove_cvref_t<T>, std::remove_cvref_t<U>>;
    template<class From, class To> concept convertible_to_non_slicing = std::convertible_to<From, To> && !(std::is_pointer_v<std::decay_t<From>> && std::is_pointer_v<std::decay_t<To>> && !std::convertible_to<std::remove_pointer_t<std::decay_t<From>> (*)[], std::remove_pointer_t<std::decay_t<To>> (*)[]>);
    template<class T> concept pair_like = /* tuple-like<T> && */ std::tuple_size_v<std::remove_cvref_t<T>> == 2;
    template<class T, class U, class V> concept pair_like_convertible_from = !std::ranges::range<T> && !std::is_reference_v<T> && pair_like<T> && std::constructible_from<T, U, V> && convertible_to_non_slicing<U, std::tuple_element_t<0, T>> && std::convertible_to<V, std::tuple_element_t<1, T>>;
}  // namespace internal
template<std::input_or_output_iterator I, std::sentinel_for<I> S = I, RangeKind K = std::sized_sentinel_for<S, I> ? RangeKind::Sized : RangeKind::Unsized>
    requires(K == RangeKind::Sized || !std::sized_sentinel_for<S, I>)
class Subrange : public ViewInterface<Subrange<I, S, K>, std::iter_value_t<I>> {
    I itr;
    S sent;
    static constexpr bool StoreSize = (K == RangeKind::Sized && !std::sized_sentinel_for<S, I>);
    struct empty_sz {};
    [[no_unique_address]] std::conditional_t<StoreSize, std::make_unsigned_t<std::iter_difference_t<I>>, empty_sz> sz;
public:
    constexpr Subrange() = default;
    constexpr Subrange(internal::convertible_to_non_slicing<I> auto i, S s)
        requires(!StoreSize)
      : itr(i),
        sent(s) {}
    constexpr Subrange(internal::convertible_to_non_slicing<I> auto i, S s, std::make_unsigned_t<std::iter_difference_t<I>> n)
        requires(K == RangeKind::Sized)
      : itr(i),
        sent(s) {
        if constexpr (StoreSize) sz = n;
    }
    template<internal::difference_from<Subrange> R>
        requires std::ranges::borrowed_range<R> && internal::convertible_to_non_slicing<std::ranges::iterator_t<R>, I>
                 && std::convertible_to<std::ranges::sentinel_t<R>, S>
                 constexpr Subrange(R&& r)
                     requires(!StoreSize || std::ranges::sized_range<R>)
      : itr(std::ranges::begin(r)),
        sent(std::ranges::end(r)) {}
    template<std::ranges::borrowed_range R>
        requires internal::convertible_to_non_slicing<std::ranges::iterator_t<R>, I>
                 && std::convertible_to<std::ranges::sentinel_t<R>, S>
                 constexpr Subrange(R&& r, std::make_unsigned_t<std::iter_difference_t<I>> n)
                     requires(K == RangeKind::Sized)
      : Subrange{ std::ranges::begin(r), std::ranges::end(r), n } {}
    template<internal::difference_from<Subrange> PairLike>
        requires internal::pair_like_convertible_from<PairLike, const I&, const S&>
    constexpr operator PairLike() const {
        return PairLike(itr, sent);
    }
    constexpr I begin() const
        requires std::copyable<I>
    {
        return itr;
    }
    [[nodiscard]] constexpr I begin()
        requires(!std::copyable<I>)
    {
        return std::move(itr);
    }
    constexpr S end() const { return sent; }
    constexpr bool empty() const { return itr == sent; }
    constexpr I data() const
        requires(std::is_pointer_v<I> && std::copyable<I>)
    {
        return itr;
    }
    constexpr I data() const
        requires(std::is_pointer_v<I> && !std::copyable<I>)
    {
        return std::move(itr);
    }
    [[nodiscard]] constexpr Subrange next(std::iter_difference_t<I> n = 1) const&
        requires std::forward_iterator<I>
    {
        auto tmp = *this;
        tmp.advance(n);
        return tmp;
    }
    [[nodiscard]] constexpr Subrange next(std::iter_difference_t<I> n = 1) && {
        advance(n);
        return std::move(*this);
    }
    [[nodiscard]] constexpr Subrange prev(std::iter_difference_t<I> n = 1) const
        requires std::bidirectional_iterator<I>
    {
        auto tmp = *this;
        tmp.advance(-n);
        return tmp;
    }
    constexpr Subrange& advance(std::iter_difference_t<I> n) {
        if constexpr (StoreSize) {
            auto d = n - std::ranges::advance(itr, n, sent);
            if (d >= 0) sz -= static_cast<std::make_unsigned_t<std::remove_cvref_t<decltype(d)>>>(d);
            else sz += static_cast<std::make_unsigned_t<std::remove_cvref_t<decltype(d)>>>(d);
            return *this;
        } else {
            std::ranges::advance(itr, n, sent);
            return *this;
        }
    }
};
template<std::input_or_output_iterator I, std::sentinel_for<I> S> Subrange(I, S) -> Subrange<I, S>;
template<std::input_or_output_iterator I, std::sentinel_for<I> S> Subrange(I, S, std::make_unsigned_t<std::iter_difference_t<I>>) -> Subrange<I, S, RangeKind::Sized>;
template<std::ranges::borrowed_range R> Subrange(R&&) -> Subrange<std::ranges::iterator_t<R>, std::ranges::sentinel_t<R>, (std::ranges::sized_range<R> || std::sized_sentinel_for<std::ranges::sentinel_t<R>, std::ranges::iterator_t<R>>) ? RangeKind::Sized : RangeKind::Unsized>;
template<std::ranges::borrowed_range R> Subrange(R&&, std::make_unsigned_t<std::ranges::range_difference_t<R>>) -> Subrange<std::ranges::iterator_t<R>, std::ranges::sentinel_t<R>, RangeKind::Sized>;

}  // namespace gsh

namespace std::ranges {
template<class I, class S, gsh::RangeKind K> constexpr bool enable_borrowed_range<gsh::Subrange<I, S, K>> = true;
}

/*
namespace gsh {

namespace internal {
    template<class T> class StepIterator : public IteratorInterface<StepIterator<T>> {
        T current;
        constexpr StepIterator() noexcept(std::is_nothrow_default_constructible_v<T>) : current() {}
        constexpr StepIterator(const T& x) noexcept(std::is_nothrow_copy_constructible_v<T>) : current(x) {}
        constexpr StepIterator(T&& x) noexcept(std::is_nothrow_move_constructible_v<T>) : current(std::move(x)) {}
    public:
        using difference_type = T;
        using size_type = T;
        using value_type = T;
        using reference = T&;
        using pointer = decltype(&std::declval<const T&>());
        using iterator_category = std::random_access_iterator_tag;
        constexpr const T& operator*() const noexcept { return current; }
        constexpr iterator& operator++() noexcept(noexcept(++current)) {
            ++current;
            return *this;
        }
        constexpr iterator& operator--() noexcept(noexcept(--current)) {
            --current;
            return *this;
        }
        constexpr iterator& operator+=(const T& n) noexcept(noexcept(current += n)) {
            current += n;
            return *this;
        }
        constexpr iterator& operator-=(const T& n) noexcept(noexcept(current -= n)) {
            current -= n;
            return *this;
        }
        friend constexpr auto operator-(const StepIterator& a, const StepIterator& b) noexcept(noexcept(a.current - b.current)) { return a.current - b.current; }
        friend constexpr bool operator==(const StepIterator& a, const StepIterator& b) noexcept(noexcept(a.current == b.current)) { return a.current == b.current; }
        friend constexpr auto operator<=>(const StepIterator& a, const StepIterator& b) noexcept(noexcept(a.current <=> b.current)) { return a.current <=> b.current; }
    };
    template<class T> class StepSentinel : public SentinelInterface<StepSentinel<T>> {
        const T& current;
        constexpr StepSentinel(const T& x) noexcept : current(x) {}
    public:
        template<class U> friend constexpr auto operator-(const StepSentinel& a, const StepIterator<U>& b) noexcept(noexcept(a.current - b.current)) { return a.current - b.current; }
        template<class U> friend constexpr auto operator-(const StepIterator<U>& a, const StepSentinel& b) noexcept(noexcept(a.current - b.current)) { return a.current - b.current; }
        template<class U> friend constexpr bool operator==(const StepSentinel& a, const StepIterator<U>& b) noexcept(noexcept(a.current == b.current)) { return a.current == b.current; }
        template<class U> friend constexpr bool operator<=>(const StepSentinel& a, const StepIterator<U>& b) noexcept(noexcept(a.current <=> b.current)) { return a.current <=> b.current; }
    };
    template<class T> class Step1 : public ViewInterface<Step1<T>> {
        T last;
        constexpr Step1(const T& x) : last(x) {}
        constexpr Step1(T&& x) : last(std::move(x)) {}
    public:
        using iterator = StepIterator<T>;
        using sentinel = StepSentinel<T>;
        constexpr iterator begin() const noexcept { return iterator(); }
        constexpr sentinel end() const noexcept { return sentinel(last); }
        template<class U> friend constexpr StepImpl(U&& x) noexcept(noexcept(internal::Step1(std::forward<U>(x)))) { return internal::Step1(std::forward<U>(x)); }
    };
}  // namespace internal

template<class U> constexpr auto Step(U&& x) noexcept(noexcept(internal::Step1(std::forward<U>(x)))) {
    internal::StepImpl(x);
}

}  // namespace gsh
*/
