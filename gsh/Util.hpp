#pragma once
#include <type_traits>  // std::decay_t
#include <utility>      // std::forward, std::move
#include <iterator>     // std::advance, std::random_access_iterator_tag
#include <compare>      // std::three_way_comparable_with, std::compare_three_way_result_t

namespace gsh {

class InPlaceTag {};
constexpr InPlaceTag InPlace;

namespace internal {

    template<class T> struct DiffType {
        using type = void;
    };
    template<class T>
        requires requires(T x) { x - x; }
    struct DiffType<T> {
        T x;
        using type = decltype(x - x);
    };
    template<class T> class StepIterator {
        T data;
    public:
        using value_type = T;
        using difference_type = DiffType<T>;
        using pointer = const T*;
        using reference = const T&;
        using iterator_category = std::random_access_iterator_tag;
        using iterator_concept = std::random_access_iterator_tag;
        constexpr StepIterator() : data() {}
        constexpr StepIterator(const value_type& x) : data(x) {}
        constexpr StepIterator(value_type&& x) : data(std::move(x)) {}
        template<class U> constexpr StepIterator(const StepIterator<U>& x) : data(static_cast<value_type>(*x)) {}
        template<class U> constexpr StepIterator(StepIterator<U>&& x) : data(static_cast<value_type>(std::move(*x))) {}
        constexpr StepIterator& operator=(const StepIterator& u) = default;
        template<class U> constexpr StepIterator& operator=(const StepIterator<U>& u) {
            data = static_cast<value_type>(u.data);
            return *this;
        }
        template<class U> constexpr StepIterator& operator=(StepIterator<U>&& u) {
            data = static_cast<value_type>(std::move(u.data));
            return *this;
        }
        constexpr reference operator*() { return data; }
        constexpr reference operator*() const { return data; }
        constexpr pointer operator->() const { return &data; }
        constexpr StepIterator& operator++() {
            ++data;
            return *this;
        }
        constexpr StepIterator operator++(int) {
            StepIterator copy = *this;
            ++data;
            return copy;
        }
        constexpr StepIterator& operator--() {
            --data;
            return *this;
        }
        constexpr StepIterator operator--(int) {
            StepIterator copy = *this;
            --data;
            return copy;
        }
        constexpr StepIterator& operator+=(const difference_type& n) {
            data += n;
            return *this;
        }
        constexpr StepIterator& operator-=(const difference_type& n) {
            data -= n;
            return *this;
        }
        constexpr StepIterator operator+(const difference_type& n) const { return StepIterator(*this) += n; }
        constexpr StepIterator operator-(const difference_type& n) const { return StepIterator(*this) -= n; }
        constexpr reference operator[](const difference_type& n) const { return **this + n; }
        friend constexpr StepIterator operator+(const difference_type& n, const StepIterator& x) { return StepIterator(x) += n; }
        friend constexpr difference_type operator-(const StepIterator& x, const StepIterator& y) { return x.data - y.data; }
    };
    template<class T, class U> constexpr bool operator==(const StepIterator<T>& x, const StepIterator<U>& y) {
        return *x == *y;
    }
    template<class T, std::three_way_comparable_with<T> U> constexpr std::compare_three_way_result_t<T, U> operator<=>(const StepIterator<T>& x, const StepIterator<U>& y) {
        return *x <=> *y;
    }

    template<class T> class StepSentinel {
        T data;
    public:
        using value_type = T;
        using difference_type = DiffType<T>;
        using pointer = const T*;
        using reference = const T&;
        constexpr StepSentinel() : data() {}
        constexpr StepSentinel(const value_type& x) : data(x) {}
        constexpr StepSentinel(value_type&& x) : data(std::move(x)) {}
        template<class U> constexpr StepSentinel(const StepSentinel<U>& x) : data(static_cast<value_type>(*x)) {}
        template<class U> constexpr StepSentinel(StepSentinel<U>&& x) : data(static_cast<value_type>(std::move(*x))) {}
        constexpr StepSentinel& operator=(const StepSentinel& u) = default;
        template<class U> constexpr StepSentinel& operator=(const StepSentinel<U>& u) {
            data = static_cast<value_type>(u.data);
            return *this;
        }
        template<class U> constexpr StepSentinel& operator=(StepSentinel<U>&& u) {
            data = static_cast<value_type>(std::move(u.data));
            return *this;
        }
        constexpr reference operator*() { return data; }
        constexpr reference operator*() const { return data; }
        constexpr pointer operator->() const { return &data; }
    };
    template<class T, class U> constexpr bool operator==(const StepSentinel<T>& x, const StepSentinel<U>& y) {
        return *x == *y;
    }
    template<class T, class U> constexpr bool operator==(const StepIterator<T>& x, const StepSentinel<U>& y) {
        if constexpr (requires(T x, U y) { x < y; }) return !(*x < *y);
        else return *x == *y;
    }
    template<class T, class U> constexpr bool operator==(const StepSentinel<T>& x, const StepIterator<U>& y) {
        return operator==(y, x);
    }

    template<class T> class StepResult1 {
        T ed;
    public:
        using iterator = StepIterator<T>;
        using const_iterator = iterator;
        using sentinel = StepSentinel<T>;
        template<class U> constexpr StepResult1(U&& e) : ed(std::forward<U>(e)) {}
        constexpr iterator begin() const { return iterator(); }
        constexpr const_iterator cbegin() const { return const_iterator(); }
        constexpr sentinel end() const { return sentinel(ed); }
        constexpr sentinel cend() const { return sentinel(ed); }
    };
    template<class T> class StepResult2 {
        T bg, ed;
    public:
        using iterator = StepIterator<T>;
        using const_iterator = iterator;
        using sentinel = StepSentinel<T>;
        template<class U, class V> constexpr StepResult2(U&& b, V&& e) : bg(std::forward<U>(b)), ed(std::forward<V>(e)) {}
        constexpr iterator begin() const { return iterator(bg); }
        constexpr const_iterator cbegin() const { return const_iterator(ed); }
        constexpr sentinel end() const { return sentinel(ed); }
        constexpr sentinel cend() const { return sentinel(ed); }
    };

}  // namespace internal

template<class T> constexpr auto Step(T&& ed) {
    return internal::StepResult1<std::decay_t<T>>(std::forward<T>(ed));
}
template<class T, class U> constexpr auto Step(T&& bg, U&& ed) {
    using loop_var_type = std::common_type_t<T, U>;
    return internal::StepResult2<loop_var_type>(std::forward<loop_var_type>(bg), std::forward<loop_var_type>(ed));
}

}  // namespace gsh