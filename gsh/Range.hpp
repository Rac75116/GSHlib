#pragma once
#include <type_traits>             // std::is_class_v, std::remove_cv_t, std::common_reference_t, std::is_lvalue_reference
#include <concepts>                // std::same_as, std::predicate, std::convertible_to
#include <utility>                 // std::move, std::declval
#include <iterator>                // std::next, std::iter_value_t, std::iter_reference_t, std::input_iterator
#include <ranges>                  // std::ranges::iterator_t, std::sentinel_for, std::ranges::reverse, std::ranges::range, std::ranges::range_value_t, std::ranges::subrange_kind, std::sized_sentinel_for
#include <tuple>                   // std::tuple_element
#include "TypeDef.hpp"             // gsh::itype
#include "internal/Operation.hpp"  //gsh::internal::IteratorInterface
#include "Functional.hpp"          // gsh::Less, gsh::Identity

namespace gsh {

enum class RangeKind { Sized, Unsized };

namespace internal {
    template<class F, class T, class I, class U> concept IndirectlyBinaryLeftFoldableImpl = std::movable<T> && std::movable<U> && std::convertible_to<T, U> && std::invocable<F&, U, std::iter_reference_t<I>> && std::assignable_from<U&, std::invoke_result_t<F&, U, std::iter_reference_t<I>>>;
    template<class F, class T, class I> concept IndirectlyBinaryLeftFoldable = std::copy_constructible<F> && std::indirectly_readable<I> && std::invocable<F&, T, std::iter_reference_t<I>> && std::convertible_to<std::invoke_result_t<F&, T, std::iter_reference_t<I>>, std::decay_t<std::invoke_result_t<F&, T, std::iter_reference_t<I>>>> && IndirectlyBinaryLeftFoldableImpl<F, T, I, std::decay_t<std::invoke_result_t<F&, T, std::iter_reference_t<I>>>>;

    // These functions are defined in gsh/Algorithm.hpp
    template<class R, class Comp, class Proj> constexpr auto MinImpl(R&& r, Comp&& comp, Proj&& proj);
    template<class R, class Comp, class Proj> constexpr auto MaxImpl(R&& r, Comp&& comp, Proj&& proj);
    template<class R, class T, class F> constexpr auto FoldImpl(R&& r, T init, F&& f);
    template<class R, class F> constexpr auto SumImpl(R&& r, F&& f);
    template<class R> constexpr void ReverseImpl(R&& r);
    template<class R, class Comp, class Proj> constexpr void SortImpl(R&& r, Comp&& comp, Proj&& proj);
    template<class R, class Comp, class Proj> constexpr auto SortIndexImpl(R&& r, Comp&& comp, Proj&& proj);
    template<class R, class Comp, class Proj> constexpr auto OrderImpl(R&& r, Comp&& comp, Proj&& proj);
    template<class R, class Comp, class Proj> constexpr auto IsSortedImpl(R&& r, Comp&& comp, Proj&& proj);
    template<class R, class Comp, class Proj> constexpr auto IsSortedUntilImpl(R&& r, Comp&& comp, Proj&& proj);
    template<class Iter, class Sent, class T, class Proj, class Comp> constexpr auto LowerBoundImpl(Iter first, Sent last, const T& value, Comp&& comp, Proj&& proj);
    template<class Iter, class Sent, class T, class Proj, class Comp> constexpr auto UpperBoundImpl(Iter first, Sent last, const T& value, Comp&& comp, Proj&& proj);
    template<class R, class Equal> constexpr auto IsPalindromeImpl(R&& r, Equal&& equal);
    // These functions are defined in gsh/Numeric.hpp
    template<class R, class Proj> constexpr auto GCDImpl(R&& r, Proj&& proj);
    template<class R, class Proj> constexpr auto LCMImpl(R&& r, Proj&& proj);
}  // namespace internal

template<class D, class V>
    requires std::is_class_v<D> && std::same_as<D, std::remove_cv_t<D>>
class ViewInterface;

template<std::input_or_output_iterator I, std::sentinel_for<I> S, RangeKind K>
    requires(K == RangeKind::Sized || !std::sized_sentinel_for<S, I>)
class Subrange;

template<class D, class V>
    requires std::is_class_v<D> && std::same_as<D, std::remove_cv_t<D>>
class ViewInterface {
    constexpr D& derived() { return *static_cast<D*>(this); }
    constexpr const D& derived() const { return *static_cast<const D*>(this); }
    constexpr auto get_begin() { return std::ranges::begin(derived()); }
    constexpr auto get_begin() const { return std::ranges::begin(derived()); }
    constexpr auto get_end() { return std::ranges::end(derived()); }
    constexpr auto get_end() const { return std::ranges::end(derived()); }
    constexpr auto get_rbegin() { return std::ranges::rbegin(derived()); }
    constexpr auto get_rbegin() const { return std::ranges::rbegin(derived()); }
    constexpr auto get_rend() { return std::ranges::rend(derived()); }
    constexpr auto get_rend() const { return std::ranges::rend(derived()); }
    using derived_type = D;
public:
    using value_type = V;
    template<class Iter, std::sentinel_for<Iter> Sent> constexpr void assign(Iter first, Sent last) { derived() = derived_type(std::move(first), std::move(last)); }
    template<std::ranges::input_range R> static constexpr derived_type from_range(R&& r) { return derived_type(std::ranges::begin(r), std::ranges::end(r)); }
    static constexpr derived_type from_range(std::initializer_list<value_type> init) { return derived_type(std::ranges::begin(init), std::ranges::end(init)); }
    template<std::ranges::input_range R> constexpr void assign_range(R&& r) { derived().assign(std::ranges::begin(r), std::ranges::end(r)); }
    constexpr void assign_range(std::initializer_list<value_type> init) { derived().assign(init); }
    constexpr derived_type copy() const& { return derived(); }
    constexpr derived_type copy() & { return derived(); }
    constexpr derived_type copy() && { return std::move(derived()); }
    constexpr auto slice(itype::u32 start) { return Subrange(std::ranges::next(get_begin(), start), get_end()); }
    constexpr auto slice(itype::u32 start, itype::u32 end) { return Subrange(std::ranges::next(get_begin(), start), std::ranges::next(get_begin(), end)); }
    constexpr auto slice(itype::u32 start) const { return Subrange(std::ranges::next(get_begin(), start), get_end()); }
    constexpr auto slice(itype::u32 start, itype::u32 end) const { return Subrange(std::ranges::next(get_begin(), start), std::ranges::next(get_begin(), end)); }
    template<std::indirect_unary_predicate<std::ranges::iterator_t<derived_type>> Pred>
        requires std::ranges::input_range<derived_type>
    [[nodiscard]] constexpr auto filter(Pred&& pred = {}) const {
        return derived() | std::views::filter(std::forward<Pred>(pred));
    }
    template<std::invocable<value_type> Proj>
        requires std::ranges::input_range<derived_type>
    [[nodiscard]] constexpr auto transform(Proj&& proj = {}) const {
        return derived() | std::views::transform(std::forward<Proj>(proj));
    }
    template<std::indirect_unary_predicate<std::ranges::iterator_t<derived_type>> Pred>
        requires std::ranges::input_range<derived_type>
    [[nodiscard]] constexpr auto erase_if(Pred&& pred = {}) {
        auto view = derived().filter([&](auto&& x) { return !Invoke(pred, x); });
        derived() = derived_type(std::ranges::begin(view), std::ranges::end(view));
    }
    template<std::invocable<value_type> Proj>
        requires std::ranges::input_range<derived_type> && std::ranges::output_range<derived_type, std::invoke_result_t<Proj, value_type&&>>
    constexpr void apply(Proj&& proj = {}) {
        for (auto&& x : derived()) x = Invoke(proj, std::move(x));
    }
    template<class T>
        requires std::ranges::output_range<derived_type, const T&>
    constexpr void fill(const T& x) {
        for (auto& el : derived()) el = x;
    }
    /*
    template<class Proj = Identity, std::indirect_equivalence_relation<std::projected<std::ranges::iterator_t<derived_type>, Proj>> Comp = EqualTo>
        requires std::ranges::forward_range<derived_type> && std::permutable<std::ranges::iterator_t<derived_type>>
    constexpr void unique(Comp&& comp = {}, Proj&& proj = {}) {
        const auto sr = std::ranges::unique(get_begin(), get_end(), std::forward<Comp>(comp), std::forward<Proj>(proj));
        if constexpr (requires { derived().erase(sr.begin(), sr.end()); }) derived().erase(sr.begin(), sr.end());
        else derived() = derived_type(get_begin(), sr.begin());
    }
    */
    template<std::predicate<value_type> Pred>
        requires std::ranges::input_range<derived_type>
    constexpr bool all_of(Pred&& f = {}) const {
        for (auto&& el : derived())
            if (!Invoke(f, std::forward<decltype(el)>(el))) return false;
        return true;
    }
    template<std::predicate<value_type> Pred>
        requires std::ranges::input_range<derived_type>
    constexpr bool any_of(Pred&& f = {}) const {
        for (auto&& el : derived())
            if (Invoke(f, std::forward<decltype(el)>(el))) return true;
        return false;
    }
    template<std::predicate<value_type> Pred>
        requires std::ranges::input_range<derived_type>
    constexpr bool none_of(Pred f) const {
        for (auto&& el : derived())
            if (Invoke(f, std::forward<decltype(el)>(el))) return false;
        return true;
    }
    template<class T, class Proj = Identity, std::indirect_equivalence_relation<std::projected<std::ranges::iterator_t<derived_type>, Proj>> Comp = EqualTo>
        requires std::ranges::input_range<derived_type>
    constexpr bool contains(const T& x, Comp&& comp = {}, Proj&& proj = {}) const {
        for (auto&& el : derived())
            if (Invoke(comp, x, Invoke(proj, std::forward<decltype(el)>(el)))) return true;
        return false;
    }
    template<class T, class Proj = Identity, std::indirect_equivalence_relation<std::projected<std::ranges::iterator_t<derived_type>, Proj>> Comp = EqualTo>
        requires std::ranges::input_range<derived_type>
    constexpr auto find(const T& x, Comp&& comp = {}, Proj&& proj = {}) const {
        const auto end = get_end();
        auto itr = get_begin();
        for (; itr != end; ++itr)
            if (Invoke(comp, x, Invoke(proj, *itr))) return itr;
        return itr;
    }
    template<class T, class Proj = Identity, std::indirect_equivalence_relation<std::projected<std::ranges::iterator_t<derived_type>, Proj>> Comp = EqualTo>
        requires std::ranges::input_range<derived_type>
    constexpr itype::u32 index_of(const T& x, Comp&& comp = {}, Proj&& proj = {}) const {
        const auto end = get_end();
        itype::u32 cnt = 0;
        for (auto itr = get_begin(); itr != end; ++itr) {
            if (Invoke(comp, x, Invoke(proj, *itr))) return cnt;
            ++cnt;
        }
        return cnt;
    }
    template<class T, class Proj = Identity, std::indirect_equivalence_relation<std::projected<std::ranges::iterator_t<derived_type>, Proj>> Comp = EqualTo>
        requires std::ranges::input_range<derived_type>
    constexpr itype::u32 count(const T& x, Comp&& comp = {}, Proj&& proj = {}) const {
        itype::u32 res = 0;
        for (auto&& el : derived()) res += static_cast<bool>(Invoke(comp, x, Invoke(proj, std::forward<decltype(el)>(el))));
        return res;
    }
    template<class Proj = Identity, std::indirect_strict_weak_order<std::projected<std::ranges::iterator_t<derived_type>, Proj>> Comp = Less>
        requires std::ranges::forward_range<derived_type> && std::indirectly_copyable_storable<std::ranges::iterator_t<derived_type>, value_type*>
    constexpr auto min(Comp&& comp = {}, Proj&& proj = {}) const {
        return internal::MinImpl(derived(), std::forward<Comp>(comp), std::forward<Proj>(proj));
    }
    template<class Proj = Identity, std::indirect_strict_weak_order<std::projected<std::ranges::iterator_t<derived_type>, Proj>> Comp = Less>
        requires std::ranges::forward_range<derived_type> && std::indirectly_copyable_storable<std::ranges::iterator_t<derived_type>, value_type*>
    constexpr auto max(Comp&& comp = {}, Proj&& proj = {}) const {
        return internal::MaxImpl(derived(), std::forward<Comp>(comp), std::forward<Proj>(proj));
    }
    template<class T = value_type, internal::IndirectlyBinaryLeftFoldable<T, std::ranges::iterator_t<derived_type>> F = Plus>
        requires std::ranges::forward_range<derived_type>
    constexpr auto fold(T&& init = {}, F&& f = {}) const {
        return internal::FoldImpl(derived(), std::forward<T>(init), std::forward<F>(f));
    }
    template<internal::IndirectlyBinaryLeftFoldable<value_type, std::ranges::iterator_t<derived_type>> F = Plus>
        requires std::ranges::forward_range<derived_type>
    constexpr auto sum(F&& f = {}) const {
        return internal::SumImpl(derived(), std::forward<F>(f));
    }
    void reverse()
        requires std::ranges::bidirectional_range<derived_type> && std::permutable<std::ranges::iterator_t<derived_type>>
    {
        internal::ReverseImpl(derived());
    }
    /*
    [[nodiscard]] auto reversed() const
        requires std::ranges::bidirectional_range<derived_type> && std::permutable<std::ranges::iterator_t<derived_type>>
    {
        auto res = copy();
        internal::ReverseImpl(res);
        return res;
    }
    */
    template<class Proj = Identity, class Comp = Less>
        requires std::ranges::forward_range<derived_type> && std::sortable<std::ranges::iterator_t<derived_type>, Comp, Proj>
    constexpr void sort(Comp&& comp = {}, Proj&& proj = {}) {
        internal::SortImpl(derived(), std::forward<Comp>(comp), std::forward<Proj>(proj));
    }
    /*
    template<class Proj = Identity, class Comp = Less>
        requires std::ranges::forward_range<derived_type> && std::sortable<std::ranges::iterator_t<derived_type>, Comp, Proj>
    [[nodiscard]] constexpr auto sorted(Comp&& comp = {}, Proj&& proj = {}) const {
        auto res = copy();
        internal::SortImpl(res, std::forward<Comp>(comp), std::forward<Proj>(proj));
        return res;
    }
    */
    template<class Proj = Identity, class Comp = Less>
        requires std::ranges::random_access_range<derived_type> && std::sortable<std::ranges::iterator_t<derived_type>, Comp, Proj>
    constexpr auto sort_index(Comp&& comp = {}, Proj&& proj = {}) const {
        return internal::SortIndexImpl(derived(), std::forward<Comp>(comp), std::forward<Proj>(proj));
    }
    template<class Proj = Identity, class Comp = Less>
        requires std::ranges::random_access_range<derived_type> && std::sortable<std::ranges::iterator_t<derived_type>, Comp, Proj>
    constexpr auto order(Comp&& comp = {}, Proj&& proj = {}) const {
        return internal::OrderImpl(derived(), std::forward<Comp>(comp), std::forward<Proj>(proj));
    }
    template<class Proj = Identity, std::indirect_strict_weak_order<std::projected<std::ranges::iterator_t<derived_type>, Proj>> Comp = Less>
        requires std::ranges::forward_range<derived_type>
    constexpr auto is_sorted(Comp&& comp = {}, Proj&& proj = {}) const {
        return internal::IsSortedImpl(derived(), std::forward<Comp>(comp), std::forward<Proj>(proj));
    }
    template<class Proj = Identity, std::indirect_strict_weak_order<std::projected<std::ranges::iterator_t<derived_type>, Proj>> Comp = Less>
        requires std::ranges::forward_range<derived_type>
    constexpr auto is_sorted_until(Comp&& comp = {}, Proj&& proj = {}) const {
        return internal::IsSortedUntilImpl(derived(), std::forward<Comp>(comp), std::forward<Proj>(proj));
    }
    template<class T, class Proj = Identity, std::indirect_strict_weak_order<const T*, std::projected<std::ranges::iterator_t<derived_type>, Proj>> Comp = Less>
        requires std::ranges::forward_range<derived_type>
    constexpr auto lower_bound(const T& value, Comp&& comp = {}, Proj&& proj = {}) const {
        return internal::LowerBoundImpl(get_begin(), get_end(), value, std::forward<Comp>(comp), std::forward<Proj>(proj));
    }
    template<class T, class Proj = Identity, std::indirect_strict_weak_order<const T*, std::projected<std::ranges::iterator_t<derived_type>, Proj>> Comp = Less>
        requires std::ranges::forward_range<derived_type>
    constexpr auto upper_bound(const T& value, Comp&& comp = {}, Proj&& proj = {}) const {
        return internal::UpperBoundImpl(get_begin(), get_end(), value, std::forward<Comp>(comp), std::forward<Proj>(proj));
    }
    template<std::indirect_binary_predicate<std::ranges::iterator_t<derived_type>, std::ranges::iterator_t<derived_type>> Equal = EqualTo>
        requires std::ranges::bidirectional_range<derived_type>
    constexpr auto is_palindrome(Equal&& equal = {}) const {
        return internal::IsPalindromeImpl(derived(), std::forward<Equal>(equal));
    }
    template<class Proj = Identity>
        requires std::ranges::input_range<derived_type> && std::invocable<Proj, value_type>
    constexpr auto gcd(Proj&& proj = {}) const {
        return internal::GCDImpl(derived(), std::forward<Proj>(proj));
    }
    template<class Proj = Identity>
        requires std::ranges::input_range<derived_type> && std::invocable<Proj, value_type>
    constexpr auto lcm(Proj&& proj = {}) const {
        return internal::LCMImpl(derived(), std::forward<Proj>(proj));
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
