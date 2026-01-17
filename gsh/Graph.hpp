#pragma once
#include "Arr.hpp"
#include "Exception.hpp"
#include "Range.hpp"
#include "TypeDef.hpp"
#include "Vec.hpp"
#include <iterator>
#include <limits>
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
    if(_w != 1) throw gsh::Exception("gsh::Edge<void>::Edge / The weight is not 1.");
  }
  constexpr u32 to() const noexcept { return t; }
  constexpr u32 weight() const noexcept { return 1; }
  constexpr operator u32() const noexcept { return t; }
};
} // namespace gsh
namespace std {
template<class W> struct tuple_size<gsh::Edge<W>> : integral_constant<size_t, 2> {};
template<std::size_t M, class W> struct tuple_element<M, gsh::Edge<W>> {
  static_assert(M < 2, "std::tuple_element<gsh::Edge> / The index is out of range.");
  using type = std::conditional_t<M == 0, gsh::u32, W>;
};
template<std::size_t M> struct tuple_element<M, gsh::Edge<void>> {
  static_assert(M < 2, "std::tuple_element<gsh::Edge<void>> / The index is out of range.");
  using type = gsh::u32;
};
} // namespace std
namespace gsh {
template<std::size_t M, class W> auto get(const Edge<W>& e) {
  static_assert(M <= 1, "gsh::get(gsh::Edge) / The index is out of range.");
  if constexpr(M == 0) return e.to();
  else return e.weight();
}
template<std::size_t M, class W> auto get(Edge<W>& e) {
  static_assert(M <= 1, "gsh::get(gsh::Edge) / The index is out of range.");
  if constexpr(M == 0) return e.to();
  else return e.weight();
}
template<std::size_t M, class W> auto get(Edge<W>&& e) {
  static_assert(M <= 1, "gsh::get(gsh::Edge) / The index is out of range.");
  if constexpr(M == 0) return e.to();
  else return std::move(e.weight());
}
namespace graph_format {
template<class W> class CRS {
  constexpr static u32 npos = 0xffffffffu;
  Vec<std::pair<Edge<W>, u32>> storage;
  Vec<u32> tail;
  template<bool IsConst> class adjacency_list : public ViewInterface<adjacency_list<IsConst>, Edge<W>> {
    using storage_ptr_type = std::conditional_t<IsConst, typename decltype(storage)::const_iterator, typename decltype(storage)::iterator>;
    storage_ptr_type storage_ptr;
    u32 idx;
    friend class CRS;
    constexpr adjacency_list(storage_ptr_type p, u32 i) : storage_ptr(p), idx(i) {}
  public:
    class iterator {
      storage_ptr_type storage_ptr;
      u32 current_idx;
      constexpr iterator(storage_ptr_type p, u32 i) : storage_ptr(p), current_idx(i) {}
      friend class adjacency_list;
    public:
      using difference_type = i32;
      using value_type = Edge<W>;
      using pointer = std::conditional_t<IsConst, const value_type*, value_type*>;
      using reference = std::conditional_t<IsConst, const value_type&, value_type&>;
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
      friend constexpr bool operator==(const iterator& a, const iterator& b) noexcept { return a.current_idx == b.current_idx; }
    };
    class const_iterator {
      using storage_ptr_type = decltype(storage)::const_iterator;
      storage_ptr_type storage_ptr;
      u32 current_idx;
      constexpr const_iterator(storage_ptr_type p, u32 i) : storage_ptr(p), current_idx(i) {}
      friend class adjacency_list;
    public:
      using difference_type = i32;
      using value_type = Edge<W>;
      using pointer = const value_type*;
      using reference = const value_type&;
      using iterator_category = std::forward_iterator_tag;
      constexpr reference operator*() const noexcept { return storage_ptr[current_idx].first; }
      constexpr pointer operator->() const noexcept { return &(storage_ptr[current_idx].first); }
      constexpr const_iterator& operator++() {
        current_idx = storage_ptr[current_idx].second;
        return *this;
      }
      constexpr const_iterator operator++(int) {
        auto copy = *this;
        operator++();
        return copy;
      }
      friend constexpr bool operator==(const const_iterator& a, const const_iterator& b) noexcept { return a.current_idx == b.current_idx; }
    };
    constexpr bool empty() const noexcept { return idx == CRS::npos; }
    constexpr iterator begin() noexcept { return iterator(storage_ptr, idx); }
    constexpr iterator end() noexcept { return iterator(storage_ptr, CRS::npos); }
    constexpr iterator begin() const noexcept { return iterator(storage_ptr, idx); }
    constexpr iterator end() const noexcept { return iterator(storage_ptr, CRS::npos); }
  };
protected:
  constexpr CRS() {}
  constexpr explicit CRS(u32 n) : tail(n, npos) {}
  constexpr u32 vertex_count() const noexcept { return tail.size(); }
  constexpr u32 edge_count() const noexcept { return storage.size(); }
  constexpr void connect(u32 from, u32 to) {
    storage.emplace_back(std::piecewise_construct, std::tuple{to}, std::tuple{tail[from]});
    tail[from] = storage.size() - 1;
  }
  constexpr void connect(u32 from, u32 to, const W& w) {
    storage.emplace_back(std::piecewise_construct, std::tuple{to, w}, std::tuple{tail[from]});
    tail[from] = storage.size() - 1;
  }
  constexpr void connect(u32 from, u32 to, W&& w) {
    storage.emplace_back(std::piecewise_construct, std::tuple{to, std::move(w)}, std::tuple{tail[from]});
    tail[from] = storage.size() - 1;
  }
  constexpr auto operator[](u32 v) {
#ifndef NDEBUG
    if(v >= vertex_count()) [[unlikely]]
      throw Exception("gsh::graph_format::CRS::operator[] / The index is out of range. ( v=", v, ", size=", vertex_count(), " )");
#endif
    return adjacency_list<false>(storage.begin(), tail[v]);
  }
  constexpr auto operator[](u32 v) const {
#ifndef NDEBUG
    if(v >= vertex_count()) [[unlikely]]
      throw Exception("gsh::graph_format::CRS::operator[] const / The index is out of range. ( v=", v, ", size=", vertex_count(), " )");
#endif
    return adjacency_list<true>(storage.begin(), tail[v]);
  }
  constexpr void reserve(u32 m) { storage.reserve(m); }
};
} // namespace graph_format
template<class W, template<class> class Format = graph_format::CRS> class DirectedGraph : public Format<W> {
  using base = Format<W>;
  using weight_type_impl = std::conditional_t<std::is_void_v<W>, u32, W>;
public:
  using edge_type = Edge<W>;
  using weight_type = weight_type_impl;
  constexpr static bool is_weighted = !std::is_void_v<W>;
  constexpr DirectedGraph() : base() {}
  constexpr explicit DirectedGraph(u32 n) : base(n) {}
  constexpr u32 vertex_count() const noexcept { return base::vertex_count(); }
  constexpr u32 edge_count() const noexcept { return base::edge_count(); }
  constexpr auto operator[](u32 v) { return base::operator[](v); }
  constexpr auto operator[](u32 v) const { return base::operator[](v); }
  constexpr void reserve(u32 m) { base::reserve(m); }
  constexpr void connect(u32 from, u32 to) { base::connect(from, to); }
  constexpr void connect(u32 from, u32 to, const weight_type& w) requires is_weighted { base::connect(from, to, w); }
  constexpr void connect(u32 from, u32 to, weight_type&& w) requires is_weighted { base::connect(from, to, std::move(w)); }
};
template<class W, template<class> class Format = graph_format::CRS> class UndirectedGraph : public Format<W> {
  using base = Format<W>;
  using weight_type_impl = std::conditional_t<std::is_void_v<W>, u32, W>;
public:
  using edge_type = Edge<W>;
  using weight_type = weight_type_impl;
  constexpr static bool is_weighted = !std::is_void_v<W>;
  constexpr UndirectedGraph() : base() {}
  constexpr explicit UndirectedGraph(u32 n) : base(n) {}
  constexpr u32 vertex_count() const noexcept { return base::vertex_count(); }
  constexpr u32 edge_count() const noexcept { return base::edge_count() / 2; }
  constexpr auto operator[](u32 v) { return base::operator[](v); }
  constexpr auto operator[](u32 v) const { return base::operator[](v); }
  constexpr void reserve(u32 m) { base::reserve(m * 2); }
  constexpr void connect(u32 a, u32 b) {
    base::connect(a, b);
    base::connect(b, a);
  }
  constexpr void connect(u32 a, u32 b, const weight_type& w) requires is_weighted {
    base::connect(a, b, w);
    base::connect(b, a, w);
  }
  constexpr void connect(u32 a, u32 b, weight_type&& w) requires is_weighted {
    weight_type tmp = std::move(w);
    base::connect(a, b, tmp);
    base::connect(b, a, std::move(tmp));
  }
  constexpr auto to_directed() const {
    u32 n = vertex_count();
    DirectedGraph<W, Format> res(n);
    res.reserve(base::count_edges_raw());
    for(u32 i = 0; i != n; ++i) {
      for(const auto& e : (*this)[i]) {
        if constexpr(!is_weighted) res.connect(i, e.to());
        else res.connect(i, e.to(), e.weight());
      }
    }
    return res;
  }
};
namespace internal {
template<class T> constexpr static bool IsGraphType = false;
template<class W, template<class> class Format> constexpr static bool IsGraphType<DirectedGraph<W, Format>> = true;
template<class W, template<class> class Format> constexpr static bool IsGraphType<UndirectedGraph<W, Format>> = true;
template<class T> concept GraphType = IsGraphType<T>;
}
} // namespace gsh
