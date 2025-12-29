#pragma once
#include "Arr.hpp"
#include "Range.hpp"
#include "TypeDef.hpp"
#include <charconv>
#include <string>
#include <string_view>
namespace gsh {
namespace internal {
template<class CharT> class StrViewImpl;
template<class CharT, class Alloc = std::allocator<CharT>> class StrImpl : public std::basic_string<CharT, std::char_traits<CharT>, Alloc>, public ViewInterface<StrImpl<CharT, Alloc>, CharT> {
  using base = std::basic_string<CharT, std::char_traits<CharT>, Alloc>;
  using view = ViewInterface<StrImpl<CharT, Alloc>, CharT>;
public:
  constexpr StrImpl() noexcept(noexcept(Alloc())) : StrImpl(Alloc()) {}
  constexpr explicit StrImpl(const Alloc& a) noexcept : base(a) {}
  constexpr StrImpl(const StrImpl&) = default;
  constexpr StrImpl(StrImpl&&) noexcept = default;
  constexpr StrImpl(const StrImpl& str, std::size_t pos, std::size_t n, const Alloc& a = Alloc()) : base(str, pos, n, a) {}
  constexpr StrImpl(const StrImpl& str, std::size_t pos, const Alloc& a = Alloc()) : base(str, pos, a) {}
  constexpr StrImpl(StrImpl&& str, std::size_t pos, std::size_t n, const Alloc& a = Alloc()) : base(std::move(str), pos, n, a) {}
  constexpr StrImpl(const CharT* s, std::size_t n, const Alloc& a = Alloc()) : base(s, n, a) {}
  constexpr StrImpl(const CharT* s, const Alloc& a = Alloc()) : base(s, a) {}
  StrImpl(std::nullptr_t) = delete;
  constexpr StrImpl(std::size_t n, CharT c, const Alloc& a = Alloc()) : base(n, c, a) {}
  template<class InputIterator> constexpr StrImpl(InputIterator begin, InputIterator end, const Alloc& a = Alloc()) : base(begin, end, a) {}
  constexpr StrImpl(std::initializer_list<CharT> init, const Alloc& a = Alloc()) : base(std::move(init), a) {}
  constexpr StrImpl(const StrImpl& str, const Alloc& a) : base(str, a) {}
  constexpr StrImpl(StrImpl&& str, const Alloc& a) : base(str, a) {}
  template<class T> constexpr explicit StrImpl(const T& t, const Alloc& a = Alloc()) : base(t, a) {}
  template<class T> constexpr StrImpl(const T& t, std::size_t pos, std::size_t n, const Alloc& a = Alloc()) : base(t, pos, n, a) {}
  constexpr ~StrImpl() = default;
  constexpr StrImpl& operator=(const StrImpl& str) { return base::operator=(str), *this; }
  constexpr StrImpl& operator=(StrImpl&& str) noexcept(std::allocator_traits<Alloc>::propagate_on_container_move_assignment::value || std::allocator_traits<Alloc>::is_always_equal::value) { return base::operator=(static_cast<base&&>(str)), *this; }
  constexpr StrImpl& operator=(const CharT* s) { return base::operator=(s), *this; }
  constexpr StrImpl& operator=(CharT c) { return base::operator=(c), *this; }
  constexpr StrImpl& operator=(std::initializer_list<CharT> il) { return base::operator=(il), *this; }
  template<class T> constexpr StrImpl& operator=(const T& t) { return base::operator=(static_cast<std::basic_string_view<CharT>>(t)), *this; }
  StrImpl& operator=(std::nullptr_t) = delete;
  constexpr base::const_reference operator[](std::size_t pos) const {
#ifndef NDEBUG
    if(pos >= base::size() + 1) [[unlikely]]
      throw gsh::Exception("gsh::internal::StrImpl::operator[] / The index is out of range. ( n=", pos, ", size=", base::size(), " )");
#endif
    return base::operator[](pos);
  }
  constexpr base::reference operator[](std::size_t pos) {
#ifndef NDEBUG
    if(pos >= base::size() + 1) [[unlikely]]
      throw gsh::Exception("gsh::internal::StrImpl::operator[] / The index is out of range. ( n=", pos, ", size=", base::size(), " )");
#endif
    return base::operator[](pos);
  }
  constexpr base::const_reference at(std::size_t pos) const {
    if(pos >= base::size()) [[unlikely]]
      throw gsh::Exception("gsh::internal::StrImpl::at / The index is out of range. ( n=", pos, ", size=", base::size(), " )");
    return base::operator[](pos);
  }
  constexpr base::reference at(std::size_t pos) {
    if(pos >= base::size()) [[unlikely]]
      throw gsh::Exception("gsh::internal::StrImpl::at / The index is out of range. ( n=", pos, ", size=", base::size(), " )");
    return base::operator[](pos);
  }
  constexpr StrImpl& operator+=(const StrImpl& str) { return base::operator+=(str), *this; }
  constexpr StrImpl& operator+=(const CharT* s) { return base::operator+=(s), *this; }
  constexpr StrImpl& operator+=(CharT c) { return base::operator+=(c), *this; }
  constexpr StrImpl& operator+=(std::initializer_list<CharT> il) { return base::operator+=(il), *this; }
  template<class T> constexpr StrImpl& operator+=(const T& t) { return base::operator+=(static_cast<const std::basic_string_view<CharT>&>(t)), *this; }
  constexpr StrImpl& append(const StrImpl& str) { return base::append(str), *this; }
  constexpr StrImpl& append(const StrImpl& str, std::size_t pos, std::size_t n = base::npos) { return base::append(str, pos, n), *this; }
  constexpr StrImpl& append(const CharT* s, std::size_t n) { return base::append(s, n), *this; }
  constexpr StrImpl& append(const CharT* s) { return base::append(s), *this; }
  constexpr StrImpl& append(std::size_t n, CharT c) { return base::append(n, c), *this; }
  template<class InputIterator> constexpr StrImpl& append(InputIterator first, InputIterator last) { return base::append(first, last), *this; }
  constexpr StrImpl& append(std::initializer_list<CharT> il) { return base::append(il), *this; }
  template<class T> constexpr StrImpl& append(const T& t) { return base::append(static_cast<const std::basic_string_view<CharT>&>(t)), *this; }
  template<class T> constexpr StrImpl& append(const T& t, std::size_t pos, std::size_t n = base::npos) { return base::append(static_cast<const std::basic_string_view<CharT>&>(t), pos, n), *this; }
  constexpr StrImpl& assign(const StrImpl& str) { return base::assign(str), *this; }
  constexpr StrImpl& assign(const StrImpl& str, std::size_t pos, std::size_t n = base::npos) { return base::assign(str, pos, n), *this; }
  constexpr StrImpl& assign(const CharT* s, std::size_t n) { return base::assign(s, n), *this; }
  constexpr StrImpl& assign(const CharT* s) { return base::assign(s), *this; }
  constexpr StrImpl& assign(std::size_t n, CharT c) { return base::assign(n, c), *this; }
  template<class InputIterator> constexpr StrImpl& assign(InputIterator first, InputIterator last) { return base::assign(first, last), *this; }
  constexpr StrImpl& assign(std::initializer_list<CharT> il) { return base::assign(il), *this; }
  template<class T> constexpr StrImpl& assign(const T& t) { return base::assign(static_cast<const std::basic_string_view<CharT>&>(t)), *this; }
  template<class T> constexpr StrImpl& assign(const T& t, std::size_t pos, std::size_t n = base::npos) { return base::assign(static_cast<const std::basic_string_view<CharT>&>(t), pos, n), *this; }
  template<class R> constexpr StrImpl& assign_range(R&& rg) { return base::assign_range(std::forward<R>(rg)), *this; }
  constexpr StrImpl& insert(std::size_t pos1, const StrImpl& str) { return base::insert(pos1, str), *this; }
  constexpr StrImpl& insert(std::size_t pos1, const StrImpl& str, std::size_t pos2, std::size_t n = base::npos) { return base::insert(pos1, str, pos2, n), *this; }
  constexpr StrImpl& insert(std::size_t pos, const CharT* s, std::size_t n) { return base::insert(pos, s, n), *this; }
  constexpr StrImpl& insert(std::size_t pos, const CharT* s) { return base::insert(pos, s), *this; }
  constexpr StrImpl& insert(std::size_t pos, std::size_t n, CharT c) { return base::insert(pos, n, c), *this; }
  constexpr typename base::iterator insert(typename base::const_iterator p, CharT c) { return base::insert(p, c); }
  constexpr typename base::iterator insert(typename base::const_iterator p, std::size_t n, CharT c) { return base::insert(p, n, c); }
  template<class InputIterator> constexpr typename base::iterator insert(typename base::const_iterator p, InputIterator first, InputIterator last) { return base::insert(p, first, last); }
  constexpr typename base::iterator insert(typename base::const_iterator p, std::initializer_list<CharT> il) { return base::insert(p, il); }
  template<class T> constexpr StrImpl& insert(std::size_t pos1, const T& t) { return base::insert(pos1, static_cast<const std::basic_string_view<CharT>&>(t)), *this; }
  template<class T> constexpr StrImpl& insert(std::size_t pos1, const T& t, std::size_t pos2, std::size_t n = base::npos) { return base::insert(pos1, static_cast<const std::basic_string_view<CharT>&>(t), pos2, n), *this; }
  constexpr StrImpl& erase(std::size_t pos = 0, std::size_t n = base::npos) { return base::erase(pos, n), *this; }
  constexpr StrImpl& replace(std::size_t pos1, std::size_t n1, const StrImpl& str) { return base::replace(pos1, n1, str), *this; }
  constexpr StrImpl& replace(std::size_t pos1, std::size_t n1, const StrImpl& str, std::size_t pos2, std::size_t n2 = base::npos) { return base::replace(pos1, n1, str, pos2, n2), *this; }
  constexpr StrImpl& replace(std::size_t pos, std::size_t n1, const CharT* s, std::size_t n2) { return base::replace(pos, n1, s, n2), *this; }
  constexpr StrImpl& replace(std::size_t pos, std::size_t n1, const CharT* s) { return base::replace(pos, n1, s), *this; }
  constexpr StrImpl& replace(std::size_t pos, std::size_t n1, std::size_t n2, CharT c) { return base::replace(pos, n1, n2, c), *this; }
  constexpr StrImpl& replace(typename base::const_iterator i1, typename base::const_iterator i2, const StrImpl& str) { return base::replace(i1, i2, str), *this; }
  constexpr StrImpl& replace(typename base::const_iterator i1, typename base::const_iterator i2, const CharT* s, std::size_t n) { return base::replace(i1, i2, s, n), *this; }
  constexpr StrImpl& replace(typename base::const_iterator i1, typename base::const_iterator i2, const CharT* s) { return base::replace(i1, i2, s), *this; }
  constexpr StrImpl& replace(typename base::const_iterator i1, typename base::const_iterator i2, std::size_t n, CharT c) { return base::replace(i1, i2, n, c), *this; }
  template<class InputIterator> constexpr StrImpl& replace(typename base::const_iterator i1, typename base::const_iterator i2, InputIterator j1, InputIterator j2) { return base::replace(i1, i2, j1, j2), *this; }
  constexpr StrImpl& replace(typename base::const_iterator i1, typename base::const_iterator i2, std::initializer_list<CharT> il) { return base::replace(i1, i2, il), *this; }
  template<class T> constexpr StrImpl& replace(std::size_t pos1, std::size_t n1, const T& t) { return base::replace(pos1, n1, static_cast<const std::basic_string_view<CharT>&>(t)), *this; }
  template<class T> constexpr StrImpl& replace(std::size_t pos1, std::size_t n1, const T& t, std::size_t pos2, std::size_t n2 = base::npos) { return base::replace(pos1, n1, static_cast<const std::basic_string_view<CharT>&>(t), pos2, n2), *this; }
  template<class T> constexpr StrImpl& replace(typename base::const_iterator i1, typename base::const_iterator i2, const T& t) { return base::replace(i1, i2, static_cast<const std::basic_string_view<CharT>&>(t)), *this; }
  constexpr void swap(StrImpl& str) noexcept(std::allocator_traits<Alloc>::propagate_on_container_swap::value || std::allocator_traits<Alloc>::is_always_equal::value) { base::swap(str); }
  constexpr operator StrViewImpl<CharT>() const noexcept { return StrViewImpl<CharT>(base::data(), base::size()); }
  constexpr std::size_t find_first_of(const StrImpl& str, std::size_t pos = 0) const noexcept { return base::find_first_of(str, pos); }
  constexpr std::size_t find_last_of(const StrImpl& str, std::size_t pos = base::npos) const noexcept { return base::find_last_of(str, pos); }
  constexpr std::size_t find_first_not_of(const StrImpl& str, std::size_t pos = 0) const noexcept { return base::find_first_not_of(str, pos); }
  constexpr std::size_t find_last_not_of(const StrImpl& str, std::size_t pos = base::npos) const noexcept { return base::find_last_not_of(str, pos); }
  constexpr StrImpl substr(std::size_t pos = 0, std::size_t n = base::npos) const { return StrImpl(base::substr(pos, n)); }
  constexpr StrViewImpl<CharT> subview(std::size_t pos = 0, std::size_t n = base::npos) const { return StrViewImpl<CharT>(base::data() + pos, n == base::npos ? base::size() - pos : n); }
  constexpr int compare(const StrImpl& str) const noexcept { return base::compare(str); }
  constexpr int compare(std::size_t pos1, std::size_t n1, const StrImpl& str) const { return base::compare(pos1, n1, str); }
  constexpr int compare(std::size_t pos1, std::size_t n1, const StrImpl& str, std::size_t pos2, std::size_t n2 = base::npos) const { return base::compare(pos1, n1, str, pos2, n2); }
  template<class T> constexpr int compare(const T& t) const noexcept { return base::compare(static_cast<const std::basic_string_view<CharT>&>(t)); }
  template<class T> constexpr int compare(std::size_t pos1, std::size_t n1, const T& t) const { return base::compare(pos1, n1, static_cast<const std::basic_string_view<CharT>&>(t)); }
  template<class T> constexpr int compare(std::size_t pos1, std::size_t n1, const T& t, std::size_t pos2, std::size_t n2 = base::npos) const { return base::compare(pos1, n1, static_cast<const std::basic_string_view<CharT>&>(t), pos2, n2); }
  constexpr bool starts_with(StrViewImpl<CharT> x) const noexcept { return base::starts_with(static_cast<std::basic_string_view<CharT>>(x)); }
  constexpr bool ends_with(StrViewImpl<CharT> x) const noexcept { return base::ends_with(static_cast<std::basic_string_view<CharT>>(x)); }
  constexpr bool contains(StrViewImpl<CharT> x) const noexcept { return base::contains(static_cast<std::basic_string_view<CharT>>(x)); }
  constexpr bool contains(CharT x) const noexcept { return base::contains(x); }
  constexpr bool contains(const CharT* x) const { return base::contains(x); }
  template<class... Args> constexpr auto find(Args&&... args) const { return view::find(std::forward<Args>(args)...); }
  template<class... Args> constexpr auto rfind(Args&&... args) const { return view::rfind(std::forward<Args>(args)...); }
  template<class... Args> constexpr auto copy(Args&&... args) const { return view::copy(std::forward<Args>(args)...); }
};
template<class CharT> class StrViewImpl : public std::basic_string_view<CharT>, public ViewInterface<StrViewImpl<CharT>, CharT> {
  using base = std::basic_string_view<CharT>;
  using view = ViewInterface<StrViewImpl<CharT>, CharT>;
public:
  constexpr StrViewImpl() noexcept : base() {}
  constexpr StrViewImpl(const StrViewImpl&) noexcept = default;
  constexpr StrViewImpl(const CharT* str) : base(str) {}
  StrViewImpl(std::nullptr_t) = delete;
  constexpr StrViewImpl(const CharT* str, std::size_t len) : base(str, len) {}
  template<class It, class End> constexpr StrViewImpl(It begin, End end) : base(begin, end) {}
  template<class R> constexpr explicit StrViewImpl(R&& r) : base(std::forward<R>(r)) {}
  template<class... Args> constexpr auto find(Args&&... args) const { return view::find(std::forward<Args>(args)...); }
  template<class... Args> constexpr auto rfind(Args&&... args) const { return view::rfind(std::forward<Args>(args)...); }
  template<class... Args> constexpr auto copy(Args&&... args) const { return view::copy(std::forward<Args>(args)...); }
  template<class... Args> constexpr auto contains(Args&&... args) const { return base::contains(std::forward<Args>(args)...); }
};
template<class CharT, class Alloc> constexpr StrImpl<CharT, Alloc> operator+(const StrImpl<CharT, Alloc>& lhs, const StrImpl<CharT, Alloc>& rhs) {
  StrImpl<CharT, Alloc> res = lhs;
  res.append(rhs);
  return res;
}
template<class CharT, class Alloc> constexpr StrImpl<CharT, Alloc> operator+(StrImpl<CharT, Alloc>&& lhs, const StrImpl<CharT, Alloc>& rhs) {
  lhs.append(rhs);
  return std::move(lhs);
}
template<class CharT, class Alloc> constexpr StrImpl<CharT, Alloc> operator+(const StrImpl<CharT, Alloc>& lhs, StrImpl<CharT, Alloc>&& rhs) {
  rhs.insert(0, lhs);
  return std::move(rhs);
}
template<class CharT, class Alloc> constexpr StrImpl<CharT, Alloc> operator+(StrImpl<CharT, Alloc>&& lhs, StrImpl<CharT, Alloc>&& rhs) {
  lhs.append(rhs);
  return std::move(lhs);
}
template<class CharT, class Alloc> constexpr StrImpl<CharT, Alloc> operator+(const CharT* lhs, const StrImpl<CharT, Alloc>& rhs) {
  StrImpl<CharT, Alloc> res = lhs;
  res.append(rhs);
  return res;
}
template<class CharT, class Alloc> constexpr StrImpl<CharT, Alloc> operator+(const CharT* lhs, StrImpl<CharT, Alloc>&& rhs) {
  rhs.insert(0, lhs);
  return std::move(rhs);
}
template<class CharT, class Alloc> constexpr StrImpl<CharT, Alloc> operator+(CharT lhs, const StrImpl<CharT, Alloc>& rhs) {
  StrImpl<CharT, Alloc> res(1, lhs);
  res.append(rhs);
  return res;
}
template<class CharT, class Alloc> constexpr StrImpl<CharT, Alloc> operator+(CharT lhs, StrImpl<CharT, Alloc>&& rhs) {
  rhs.insert(0, 1, lhs);
  return std::move(rhs);
}
template<class CharT, class Alloc> constexpr StrImpl<CharT, Alloc> operator+(const StrImpl<CharT, Alloc>& lhs, const CharT* rhs) {
  StrImpl<CharT, Alloc> res = lhs;
  res.append(rhs);
  return res;
}
template<class CharT, class Alloc> constexpr StrImpl<CharT, Alloc> operator+(StrImpl<CharT, Alloc>&& lhs, const CharT* rhs) {
  lhs.append(rhs);
  return std::move(lhs);
}
template<class CharT, class Alloc> constexpr StrImpl<CharT, Alloc> operator+(const StrImpl<CharT, Alloc>& lhs, CharT rhs) {
  StrImpl<CharT, Alloc> res = lhs;
  res.push_back(rhs);
  return res;
}
template<class CharT, class Alloc> constexpr StrImpl<CharT, Alloc> operator+(StrImpl<CharT, Alloc>&& lhs, CharT rhs) {
  lhs.push_back(rhs);
  return std::move(lhs);
}
template<class CharT, class Alloc> constexpr bool operator==(const StrImpl<CharT, Alloc>& a, const StrImpl<CharT, Alloc>& b) noexcept {
  return static_cast<const std::basic_string<CharT>&>(a) == static_cast<const std::basic_string<CharT>&>(b);
}
template<class CharT, class Alloc> constexpr bool operator==(const StrImpl<CharT, Alloc>& a, const CharT* b) {
  return static_cast<const std::basic_string<CharT>&>(a) == b;
}
template<class CharT, class Alloc> constexpr bool operator==(const CharT* a, const StrImpl<CharT, Alloc>& b) {
  return a == static_cast<const std::basic_string<CharT>&>(b);
}
template<class CharT, class Alloc> constexpr auto operator<=>(const StrImpl<CharT, Alloc>& lhs, const StrImpl<CharT, Alloc>& rhs) noexcept {
  return static_cast<const std::basic_string<CharT>&>(lhs) <=> static_cast<const std::basic_string<CharT>&>(rhs);
}
template<class CharT, class Alloc> constexpr auto operator<=>(const StrImpl<CharT, Alloc>& lhs, const CharT* rhs) {
  return static_cast<const std::basic_string<CharT>&>(lhs) <=> std::basic_string_view<CharT>(rhs);
}
template<class CharT, class Alloc> constexpr void swap(StrImpl<CharT, Alloc>& x, StrImpl<CharT, Alloc>& y) noexcept(noexcept(x.swap(y))) {
  x.swap(y);
}
template<class CharT, class Alloc, class Predicate> constexpr typename StrImpl<CharT, Alloc>::size_type erase_if(StrImpl<CharT, Alloc>& c, Predicate pred) {
  auto it = std::remove_if(c.begin(), c.end(), pred);
  auto res = std::distance(it, c.end());
  c.erase(it, c.end());
  return res;
}
template<class CharT> constexpr void swap(StrViewImpl<CharT>& x, StrViewImpl<CharT>& y) noexcept(noexcept(x.swap(y))) {
  x.swap(y);
}
template<class InputIterator, class Allocator = std::allocator<typename std::iterator_traits<InputIterator>::value_type>> StrImpl(InputIterator, InputIterator, Allocator = Allocator()) -> StrImpl<typename std::iterator_traits<InputIterator>::value_type, Allocator>;
template<class CharT, class Allocator = std::allocator<CharT>> explicit StrImpl(StrViewImpl<CharT>, const Allocator& = Allocator()) -> StrImpl<CharT, Allocator>;
template<class CharT, class Allocator = std::allocator<CharT>> StrImpl(StrViewImpl<CharT>, typename StrViewImpl<CharT>::size_type, typename StrViewImpl<CharT>::size_type, const Allocator& = Allocator()) -> StrImpl<CharT, Allocator>;
} // namespace internal
using Str = internal::StrImpl<c8>;
using Str8 = internal::StrImpl<utf8>;
using Str16 = internal::StrImpl<utf16>;
using Str32 = internal::StrImpl<utf32>;
using Strw = internal::StrImpl<wc>;
using StrView = internal::StrViewImpl<c8>;
using StrView8 = internal::StrViewImpl<utf8>;
using StrView16 = internal::StrViewImpl<utf16>;
using StrView32 = internal::StrViewImpl<utf32>;
using StrVieww = internal::StrViewImpl<wc>;
template<class T> class Parser;
template<> class Parser<Str> {
public:
  template<class Stream> constexpr Str operator()(Stream&& stream) const {
    stream.reload(16);
    Str res;
    while(true) {
      const c8* e = stream.current();
      while(*e >= '!') ++e;
      const u32 len = e - stream.current();
      const u32 curlen = res.size();
      res.resize(curlen + len);
      MemoryCopy(res.data() + curlen, stream.current(), len);
      stream.skip(len);
      if(stream.avail() == 0) stream.reload();
      else break;
    }
    stream.skip(1);
    return res;
  }
  template<class Stream> constexpr Str operator()(Stream&& stream, u32 n) const {
    u32 rem = n;
    Str res;
    u32 avail = stream.avail();
    while(avail <= rem) {
      const u32 curlen = res.size();
      res.resize(curlen + avail);
      MemoryCopy(res.data() + curlen, stream.current(), avail);
      rem -= avail;
      stream.skip(avail);
      if(rem == 0) return res;
      stream.reload();
      avail = stream.avail();
    }
    const u32 curlen = res.size();
    res.resize(curlen + rem);
    MemoryCopy(res.data() + curlen, stream.current(), rem);
    stream.skip(rem + 1);
    return res;
  }
};
template<class T> class Formatter;
template<> class Formatter<StrView> {
public:
  template<class Stream> constexpr void operator()(Stream&& stream, const StrView& str) const {
    const c8* s = str.data();
    u32 len = str.size();
    u32 avail = stream.avail();
    if(avail >= len) [[likely]] {
      MemoryCopy(stream.current(), s, len);
      stream.skip(len);
    } else {
      MemoryCopy(stream.current(), s, avail);
      len -= avail;
      s += avail;
      stream.skip(avail);
      while(len != 0) {
        stream.reload();
        avail = stream.avail();
        const u32 tmp = len < avail ? len : avail;
        MemoryCopy(stream.current(), s, tmp);
        len -= tmp;
        s += tmp;
        stream.skip(tmp);
      }
    }
  }
};
template<> class Formatter<Str> {
public:
  template<class Stream> constexpr void operator()(Stream&& stream, const Str& str) const { Formatter<StrView>{}(stream, StrView(str.data(), str.size())); }
};
template<class... Args>
requires ((std::same_as<Args, c8> && ...) && sizeof...(Args) >= 1)
constexpr Str UniteChars(Args... c) {
  return Str{c...};
}
template<std::integral T> constexpr Str NumToStr(const T& val, const i32 base = 10) {
  char buf[sizeof(T) * 8];
  char* last = std::to_chars(buf, buf + sizeof(T) * 8, val, base).ptr;
  return Str(buf, last);
}
template<std::floating_point T> constexpr Str NumToStr(const T& val, const i32 precision = 10) {
  char buf[64];
  auto result = std::to_chars(buf, buf + 64, val, std::chars_format::fixed, precision);
  if(result.ec != std::errc()) {
    throw Exception("gsh::ToStr / Conversion error in floating point to string.");
  }
  return Str(buf, result.ptr);
}
template<class T> constexpr T StrToNum(const StrView& str, const i32 base = 10) {
  T res{};
  auto result = std::from_chars(str.data(), str.data() + str.size(), res, base);
  if(result.ec != std::errc() || result.ptr != str.data() + str.size()) {
    throw Exception("gsh::FromStr / Conversion error in string to number.");
  }
  return res;
}
template<class T> constexpr T StrToNum(const Str& str, const i32 base = 10) {
  return StrToNum<T>(StrView(str), base);
}
} // namespace gsh
