#pragma once
#include <type_traits>        // std::integral_constant
#include <tuple>              // std::tuple_size, std::tuple_element
#include <utility>            // std::pair
#include <gsh/TypeDef.hpp>    // gsh::itype
#include <gsh/Exception.hpp>  // gsh::Exception
#include <gsh/Vec.hpp>        // gsh::Vec
#include <gsh/Arr.hpp>        // gsh::Arr

namespace gsh {

template<class W = void> class Edge {
    itype::u32 t = 0;
    W w{};
public:
    constexpr Edge(itype::u32 _t) : t(_t) {}
    constexpr Edge(itype::u32 _t, W _w) : t(_t), w(_w) {}
    constexpr itype::u32 to() const noexcept { return t; }
    constexpr const W& weight() const noexcept { return w; }
    constexpr operator itype::u32() const noexcept { return t; }
};
template<> class Edge<void> {
    itype::u32 t = 0;
public:
    constexpr Edge(itype::u32 _t) noexcept : t(_t) {}
    constexpr Edge(itype::u32 _t, itype::u32 _w) : t(_t) {
        if (_w != 1) throw gsh::Exception("gsh::Edge<void>::Edge / The weight is not 1.");
    }
    constexpr itype::u32 to() const noexcept { return t; }
    constexpr itype::u32 weight() const noexcept { return 1; }
    constexpr operator itype::u32() const noexcept { return t; }
};

}  // namespace gsh

namespace std {
template<class W> class tuple_size<gsh::Edge<W>> : integral_constant<size_t, 2> {};
template<class W> class tuple_element<0, gsh::Edge<W>> {
    using type = gsh::itype::u32;
};
}  // namespace std

namespace gsh {

template<std::size_t M, class W> auto get(const Edge<W>& e) {
    static_assert(M <= 1, "gsh::get(gsh::Edge) / The index is out of range.");
    if constexpr (M == 0) return e.to();
    else return e.weight();
}

/*
// gsh::graph_format::(DOK, LIL, COO, CRS, Matrix, Grid, Generative, Functional)
namespace graph_format {
    template<class W> class CRS {
        Vec<std::pair<Edge<W>, itype::u32>> storage;
        Arr<itype::u32> tail;
    protected:
        constexpr CRS() {}
        constexpr CRS(itype::u32 n) : tail(n, 0xffffffffu) {}
    public:
        using graph_format = void;
        constexpr void reserve(itype::u32 m) { storage.reserve(m); }
        constexpr itype::u32 vertex_count() const noexcept { return tail.size(); }
        constexpr itype::u32 edge_count() const noexcept { return storage.size(); }
    };
}  // namespace graph_format
*/

}  // namespace gsh
