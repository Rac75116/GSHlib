#pragma once
#include "TypeDef.hpp"  // gsh::itype
#include "Arr.hpp"      // gsh::Arr

namespace gsh {

template<class T, class F, class Allocator = Allocator<T>> class SparseTable {
    using traits = AllocatorTraits<Allocator>;
    [[no_unique_address]] mutable F op;
    Arr<T, Allocator> buffer;
    StaticArr<T*, 24> table;
public:
    using reference = T&;
    using const_reference = const T&;
    using size_type = itype::u32;
    using difference_type = itype::i32;
    using value_type = T;
    using allocator_type = Allocator;
    using pointer = typename traits::pointer;
    using const_pointer = typename traits::const_pointer;
    constexpr SparseTable() : op(), buffer(), table(nullptr) {}
    constexpr explicit SparseTable(F func, const allocator_type& a = Allocator()) : op(std::move(func)), buffer(a), table(nullptr) {}
    template<std::input_iterator InputIter> constexpr SparseTable(InputIter first, InputIter last, F func = F(), const allocator_type& a = Allocator()) : op(std::move(func)), buffer(a), table(nullptr) {
        const itype::u32 len = std::distance(first, last);
        //
    }
    constexpr SparseTable(const SparseTable&) = default;
    constexpr SparseTable(SparseTable&&) = default;
    constexpr SparseTable(const SparseTable& x, const allocator_type& a = Allocator()) : op(x.op), buffer(x.buffer, a), table(x.table) {}
    constexpr SparseTable(SparseTable&& y, const allocator_type& a = Allocator()) : op(std::move(y.op)), buffer(std::move(y.buffer), a), table(y.table) {}
    constexpr SparseTable(std::initializer_list<value_type> il, F func = F(), const allocator_type& a = Allocator()) : SparseTable(il.begin(), il.end(), std::move(func), a) {}
    template<ForwardRange R>
        requires(!std::same_as<SparseTable, std::remove_cvref_t<R>>)
    constexpr SparseTable(R&& r, F func = F(), const allocator_type& a = Allocator()) : SparseTable(RangeTraits<R>::begin(r), RangeTraits<R>::end(r), std::move(func), a) {}
};

}  // namespace gsh
