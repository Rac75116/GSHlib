#pragma once
#include "Exception.hpp"
#include "TypeDef.hpp"
#include "Util.hpp"
#include "Vec.hpp"
#include <bit>
#include <limits>
#include <string>
#include <string_view>
namespace gsh {
class BitVector {
  static constexpr u32 word_bits = 64;
  static constexpr u64 one = 1ULL;
  Vec<u64> words;
  u32 bit_len = 0;
  GSH_INTERNAL_INLINE static constexpr u32 words_for_bits(u32 bits) noexcept { return (bits + word_bits - 1) / word_bits; }
  GSH_INTERNAL_INLINE constexpr u32 word_len() const noexcept { return static_cast<u32>(words.size()); }
  static constexpr u64 low_mask(u32 bits) noexcept {
    if(bits == 0) return 0;
    if(bits >= word_bits) return ~0ULL;
    return (one << bits) - 1;
  }
  GSH_INTERNAL_INLINE constexpr u64 last_word_mask() const noexcept {
    const u32 r = bit_len & (word_bits - 1);
    return r == 0 ? ~0ULL : low_mask(r);
  }
  GSH_INTERNAL_INLINE constexpr void mask_tail() noexcept {
    if(bit_len == 0) return;
    const u32 wn = words_for_bits(bit_len);
    if(wn == 0) return;
    words[wn - 1] &= last_word_mask();
  }
  GSH_INTERNAL_INLINE constexpr void check_pos_on_debug(u32 pos) const {
#ifndef NDEBUG
    if(pos >= bit_len) [[unlikely]]
      throw Exception("gsh::BitVector / The index is out of range. ( pos=", pos, ", size=", bit_len, " )");
#else
    Assume(pos < bit_len);
#endif
  }
  GSH_INTERNAL_INLINE constexpr void check_same_size(const BitVector& x) const {
    if(bit_len != x.bit_len) [[unlikely]]
      throw Exception("gsh::BitVector / Size mismatch. ( lhs=", bit_len, ", rhs=", x.bit_len, " )");
  }
  template<class CharT, class Traits> constexpr void assign_from_view(std::basic_string_view<CharT, Traits> sv, CharT zero, CharT one_c) {
    if(sv.size() > static_cast<std::size_t>(std::numeric_limits<u32>::max())) [[unlikely]]
      throw std::length_error("gsh::BitVector / The string is too long.");
    assign(static_cast<u32>(sv.size()));
    for(u32 i = 0; i != bit_len; ++i) {
      const CharT c = sv[static_cast<std::size_t>(bit_len - 1 - i)];
      if(c == zero) continue;
      if(c == one_c) {
        words[i >> 6] |= (one << (i & 63));
        continue;
      }
      throw std::invalid_argument("gsh::BitVector / The string contains invalid characters.");
    }
    mask_tail();
  }
public:
  class reference {
    friend class BitVector;
    u64* w = nullptr;
    u64 m = 0;
    constexpr reference(u64* word, u64 mask) noexcept : w(word), m(mask) {}
  public:
    constexpr reference() noexcept = default;
    constexpr reference& operator=(bool v) noexcept {
      if(v) *w |= m;
      else *w &= ~m;
      return *this;
    }
    constexpr reference& operator=(const reference& x) noexcept { return (*this = static_cast<bool>(x)); }
    constexpr bool operator~() const noexcept { return !static_cast<bool>(*this); }
    constexpr operator bool() const noexcept { return (*w & m) != 0; }
    constexpr reference& flip() noexcept {
      *w ^= m;
      return *this;
    }
  };
  constexpr BitVector() noexcept = default;
  template<class CharT, class Traits, class Alloc> explicit BitVector(const std::basic_string<CharT, Traits, Alloc>& str, typename std::basic_string<CharT, Traits, Alloc>::size_type pos = 0, typename std::basic_string<CharT, Traits, Alloc>::size_type n = std::basic_string<CharT, Traits, Alloc>::npos) : BitVector(std::basic_string_view<CharT, Traits>(str), pos, n, CharT('0'), CharT('1')) {}
  template<class CharT, class Traits, class Alloc> explicit BitVector(const std::basic_string<CharT, Traits, Alloc>& str, typename std::basic_string<CharT, Traits, Alloc>::size_type pos, typename std::basic_string<CharT, Traits, Alloc>::size_type n, CharT zero, CharT one_c) : BitVector(std::basic_string_view<CharT, Traits>(str), pos, n, zero, one_c) {}
  template<class CharT, class Traits> constexpr explicit BitVector(std::basic_string_view<CharT, Traits> str, typename std::basic_string_view<CharT, Traits>::size_type pos = 0, typename std::basic_string_view<CharT, Traits>::size_type n = std::basic_string_view<CharT, Traits>::npos, CharT zero = CharT('0'), CharT one_c = CharT('1')) {
    if(pos > str.size()) [[unlikely]]
      throw std::out_of_range("gsh::BitVector / The position is out of range.");
    const auto sub = str.substr(pos, n);
    assign_from_view<CharT, Traits>(sub, zero, one_c);
  }
  template<class CharT> explicit BitVector(const CharT* str, typename std::basic_string<CharT>::size_type n = std::basic_string<CharT>::npos, CharT zero = CharT('0'), CharT one_c = CharT('1')) {
    if(str == nullptr) [[unlikely]]
      throw std::invalid_argument("gsh::BitVector / Null string pointer.");
    const std::size_t len = (n == std::basic_string<CharT>::npos) ? std::char_traits<CharT>::length(str) : static_cast<std::size_t>(n);
    assign_from_view<CharT, std::char_traits<CharT>>(std::basic_string_view<CharT>(str, len), zero, one_c);
  }
  constexpr explicit BitVector(u32 n) : words(words_for_bits(n)), bit_len(n) {
    if(n != 0) MemorySet(words.data(), 0, static_cast<u32>(word_len() * sizeof(u64)));
  }
  constexpr BitVector(u32 n, bool value) : BitVector(n) {
    if(value) set();
  }
  constexpr BitVector(const BitVector&) = default;
  constexpr BitVector(BitVector&&) noexcept = default;
  constexpr BitVector& operator=(const BitVector&) = default;
  constexpr BitVector& operator=(BitVector&&) noexcept = default;
  constexpr void assign(u32 n) {
    bit_len = n;
    words.resize(words_for_bits(n));
    if(n != 0) MemorySet(words.data(), 0, static_cast<u32>(word_len() * sizeof(u64)));
  }
  constexpr void resize(u32 n) {
    const u32 old_bits = bit_len;
    const u32 old_words = words_for_bits(old_bits);
    const u32 new_words = words_for_bits(n);
    bit_len = n;
    words.resize(new_words);
    if(new_words > old_words) {
      for(u32 i = old_words; i != new_words; ++i) words[i] = 0;
    }
    if(n > old_bits && old_words != 0) { words[old_words - 1] &= low_mask(old_bits & (word_bits - 1)); }
    mask_tail();
  }
  constexpr const u64* data() const noexcept { return words.data(); }
  constexpr u32 size() const noexcept { return bit_len; }
  constexpr bool empty() const noexcept { return bit_len == 0; }
  constexpr bool operator[](u32 pos) const {
    check_pos_on_debug(pos);
    return (words[pos >> 6] >> (pos & 63)) & 1ULL;
  }
  constexpr reference operator[](u32 pos) {
    check_pos_on_debug(pos);
    return reference(&words[pos >> 6], one << (pos & 63));
  }
  constexpr bool test(u32 pos) const { return (*this)[pos]; }
  constexpr BitVector& set() noexcept {
    if(bit_len == 0) return *this;
    for(u32 i = 0; i != word_len(); ++i) words[i] = ~0ULL;
    mask_tail();
    return *this;
  }
  constexpr BitVector& set(u32 pos) {
    check_pos_on_debug(pos);
    words[pos >> 6] |= one << (pos & 63);
    return *this;
  }
  constexpr BitVector& set(u32 pos, bool value) {
    check_pos_on_debug(pos);
    const u64 mask = one << (pos & 63);
    if(value) words[pos >> 6] |= mask;
    else words[pos >> 6] &= ~mask;
    return *this;
  }
  constexpr BitVector& reset() noexcept {
    if(bit_len == 0) return *this;
    for(u32 i = 0; i != word_len(); ++i) words[i] = 0;
    return *this;
  }
  constexpr BitVector& reset(u32 pos) {
    check_pos_on_debug(pos);
    words[pos >> 6] &= ~(one << (pos & 63));
    return *this;
  }
  constexpr BitVector& flip() noexcept {
    if(bit_len == 0) return *this;
    for(u32 i = 0; i != word_len(); ++i) words[i] = ~words[i];
    mask_tail();
    return *this;
  }
  constexpr BitVector& flip(u32 pos) {
    check_pos_on_debug(pos);
    words[pos >> 6] ^= (one << (pos & 63));
    return *this;
  }
  constexpr bool any() const noexcept {
    for(u32 i = 0; i != word_len(); ++i) {
      if(words[i] != 0) return true;
    }
    return false;
  }
  constexpr bool none() const noexcept { return !any(); }
  constexpr bool all() const noexcept {
    if(bit_len == 0) return true;
    const u32 wn = word_len();
    for(u32 i = 0; i + 1 < wn; ++i) {
      if(words[i] != ~0ULL) return false;
    }
    return (words[wn - 1] & last_word_mask()) == last_word_mask();
  }
  constexpr u32 count() const noexcept {
    u32 res = 0;
    for(u32 i = 0; i != word_len(); ++i) res += std::popcount(words[i]);
    return res;
  }
  template<class CharT = char, class Traits = std::char_traits<CharT>, class Alloc = std::allocator<CharT>> constexpr std::basic_string<CharT, Traits, Alloc> to_string(CharT zero = CharT('0'), CharT one_c = CharT('1')) const {
    std::basic_string<CharT, Traits, Alloc> s;
    s.resize(bit_len);
    for(u32 i = 0; i != bit_len; ++i) { s[bit_len - 1 - i] = test(i) ? one_c : zero; }
    return s;
  }
  constexpr BitVector& operator&=(const BitVector& x) {
    check_same_size(x);
    for(u32 i = 0; i != word_len(); ++i) words[i] &= x.words[i];
    return *this;
  }
  constexpr BitVector& operator|=(const BitVector& x) {
    check_same_size(x);
    for(u32 i = 0; i != word_len(); ++i) words[i] |= x.words[i];
    mask_tail();
    return *this;
  }
  constexpr BitVector& operator^=(const BitVector& x) {
    check_same_size(x);
    for(u32 i = 0; i != word_len(); ++i) words[i] ^= x.words[i];
    mask_tail();
    return *this;
  }
  constexpr BitVector operator~() const {
    BitVector r(*this);
    r.flip();
    return r;
  }
  constexpr BitVector& operator<<=(u32 pos) noexcept {
    if(bit_len == 0) return *this;
    if(pos >= bit_len) return reset();
    if(pos == 0) return *this;
    const u32 shift = static_cast<u32>(pos);
    const u32 wshift = shift >> 6;
    const u32 bshift = shift & 63;
    const u32 wn = word_len();
    if(bshift == 0) {
      for(u32 i = wn; i-- > wshift;) words[i] = words[i - wshift];
      for(u32 i = 0; i != wshift; ++i) words[i] = 0;
    } else {
      for(u32 i = wn; i-- > wshift + 0;) {
        if(i == wshift) break;
        const u32 src = i - wshift;
        words[i] = (words[src] << bshift) | (words[src - 1] >> (word_bits - bshift));
      }
      words[wshift] = words[0] << bshift;
      for(u32 i = 0; i != wshift; ++i) words[i] = 0;
    }
    mask_tail();
    return *this;
  }
  constexpr BitVector& operator>>=(u32 pos) noexcept {
    if(bit_len == 0) return *this;
    if(pos >= bit_len) return reset();
    if(pos == 0) return *this;
    const u32 shift = static_cast<u32>(pos);
    const u32 wshift = shift >> 6;
    const u32 bshift = shift & 63;
    const u32 wn = word_len();
    if(bshift == 0) {
      const u32 lim = wn > wshift ? (wn - wshift) : 0;
      for(u32 i = 0; i != lim; ++i) words[i] = words[i + wshift];
      for(u32 i = lim; i != wn; ++i) words[i] = 0;
    } else {
      const u32 lim = wn > wshift ? (wn - wshift) : 0;
      if(lim != 0) {
        for(u32 i = 0; i + 1 < lim; ++i) {
          const u32 src = i + wshift;
          words[i] = (words[src] >> bshift) | (words[src + 1] << (word_bits - bshift));
        }
        const u32 last = lim - 1;
        words[last] = words[last + wshift] >> bshift;
      }
      for(u32 i = lim; i != wn; ++i) words[i] = 0;
    }
    mask_tail();
    return *this;
  }
  constexpr BitVector operator<<(u32 pos) const {
    BitVector r(*this);
    r <<= pos;
    return r;
  }
  constexpr BitVector operator>>(u32 pos) const {
    BitVector r(*this);
    r >>= pos;
    return r;
  }
  constexpr bool operator==(const BitVector& x) const noexcept {
    if(bit_len != x.bit_len) return false;
    const u32 wn = word_len();
    for(u32 i = 0; i + 1 < wn; ++i) {
      if(words[i] != x.words[i]) return false;
    }
    if(wn == 0) return true;
    const u64 m = last_word_mask();
    return (words[wn - 1] & m) == (x.words[wn - 1] & m);
  }
  friend constexpr BitVector operator&(BitVector a, const BitVector& b) { return a &= b; }
  friend constexpr BitVector operator|(BitVector a, const BitVector& b) { return a |= b; }
  friend constexpr BitVector operator^(BitVector a, const BitVector& b) { return a ^= b; }
};
}
