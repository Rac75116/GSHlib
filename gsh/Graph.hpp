#pragma once
#include "Arr.hpp"
#include "Exception.hpp"
#include "TypeDef.hpp"
#include "Vec.hpp"
#include <tuple>
#include <type_traits>
#include <utility>


namespace gsh {

template<class W = void> class Edge {
    u32 t = 0;
    W w{};
public:
    constexpr Edge(u32 _t) : t(_t) {}
    constexpr Edge(u32 _t, W _w) : t(_t), w(_w) {}
    constexpr u32 to() const noexcept { return t; }
    constexpr W& weight() noexcept { return w; }
    constexpr const W& weight() const noexcept { return w; }
    constexpr operator u32() const noexcept { return t; }
};
template<> class Edge<void> {
    u32 t = 0;
public:
    constexpr Edge(u32 _t) noexcept : t(_t) {}
    constexpr Edge(u32 _t, u32 _w) : t(_t) {
        if (_w != 1) throw gsh::Exception("gsh::Edge<void>::Edge / The weight is not 1.");
    }
    constexpr u32 to() const noexcept { return t; }
    constexpr u32 weight() const noexcept { return 1; }
    constexpr operator u32() const noexcept { return t; }
};

}  // namespace gsh

namespace std {
template<class W> class tuple_size<gsh::Edge<W>> : integral_constant<size_t, 2> {};
template<class W> class tuple_element<0, gsh::Edge<W>> {
    using type = gsh::u32;
};
}  // namespace std

namespace gsh {

template<std::size_t M, class W> auto get(const Edge<W>& e) {
    static_assert(M <= 1, "gsh::get(gsh::Edge) / The index is out of range.");
    if constexpr (M == 0) return e.to();
    else return e.weight();
}
template<std::size_t M, class W> auto get(Edge<W>& e) {
    static_assert(M <= 1, "gsh::get(gsh::Edge) / The index is out of range.");
    if constexpr (M == 0) return e.to();
    else return e.weight();
}

// gsh::graph_format::(DOK, LIL, COO, CRS, Matrix, Grid, Generative, Functional)
namespace graph_format {
    template<class W> class CRS {
        Vec<std::pair<Edge<W>, u32>> storage;
        Arr<u32> tail;
        template<bool IsConst> class adjacency_list {
            using storage_ptr_type = std::conditional_t<IsConst, typename decltype(storage)::const_iterator, typename decltype(storage)::iterator>;
            storage_ptr_type storage_ptr;
            u32 idx;
        public:
            class iterator {
                storage_ptr_type storage_ptr;
                u32 current_idx;
                constexpr iterator(storage_ptr_type p, u32 i) : storage_ptr(p), current_idx(i) {}
            public:
                using difference_type = i32;
                using value_type = Edge<W>;
                using pointer = decltype(&storage_ptr->first);
                using reference = std::add_lvalue_reference<decltype(storage_ptr->first)>;
                using iterator_category = std::forward_iterator_tag;
                constexpr reference operator*() const noexcept { return storage_ptr[current_idx].first; }
                constexpr pointer operator->() const noexcept { return &(storage_ptr[current_idx].first); }
                constexpr iterator& operator++() {
                    current_idx = storage_ptr[current_idx].second;
                    return *this;
                }
                constexpr iterator operator++(int) {
                    auto copy = *this;
                    operator++();
                    return copy;
                }
            };
            class const_iterator {
                using storage_ptr_type = decltype(storage)::const_iterator;
                storage_ptr_type storage_ptr;
                u32 current_idx;
                constexpr const_iterator(storage_ptr_type p, u32 i) : storage_ptr(p), current_idx(i) {}
            public:
                using difference_type = i32;
                using value_type = Edge<W>;
                using pointer = const value_type*;
                using reference = const value_type&;
                using iterator_category = std::forward_iterator_tag;
                constexpr reference operator*() const noexcept { return storage_ptr[current_idx].first; }
                constexpr pointer operator->() const noexcept { return &(storage_ptr[current_idx].first); }
                constexpr iterator& operator++() {
                    current_idx = storage_ptr[current_idx].second;
                    return *this;
                }
                constexpr iterator operator++(int) {
                    auto copy = *this;
                    operator++();
                    return copy;
                }
            };
            constexpr iterator begin() { return iterator(storage_ptr, idx); }
        };
    protected:
        constexpr CRS() {}
        constexpr CRS(u32 n) : tail(n, 0xffffffffu) {}
    public:
        using graph_format = void;
        constexpr void reserve(u32 m) { storage.reserve(m); }
        constexpr u32 vertex_count() const noexcept { return tail.size(); }
        constexpr u32 edge_count() const noexcept { return storage.size(); }
        constexpr void connect(u32 from, u32 to) {
            storage.emplace_back(std::piecewise_construct, std::tuple{ to }, std::tuple{ tail[from] });
            tail[from] = storage.size() - 1;
        }
        constexpr void connect(u32 from, u32 to, const W& w) {
            storage.emplace_back(std::piecewise_construct, std::tuple{ to, w }, std::tuple{ tail[from] });
            tail[from] = storage.size() - 1;
        }
        constexpr void connect(u32 from, u32 to, W&& w) {
            storage.emplace_back(std::piecewise_construct, std::tuple{ to, std::move(w) }, std::tuple{ tail[from] });
            tail[from] = storage.size() - 1;
        }
    };
}  // namespace graph_format

}  // namespace gsh
