#pragma once
#include "BitVector.hpp"
#include <bit>
#if defined(__BMI2__)
#include <immintrin.h>
#endif
namespace gsh {
class IndexableDict {
  static constexpr u32 word_bits = 64;
  static constexpr u64 one = 1ULL;
  BitVector bv_;
  u32 bit_len = 0;
  u32 word_len = 0;
  Vec<u32> rank_; // rank_[i] = #ones in [0, i*64)
  static constexpr u32 words_for_bits(u32 bits) noexcept { return (bits + word_bits - 1) / word_bits; }
  static constexpr u64 low_mask(u32 bits) noexcept {
    if(bits == 0) return 0;
    if(bits >= word_bits) return ~0ULL;
    return (one << bits) - 1;
  }
  constexpr u32 valid_bits_in_word(u32 wi) const noexcept {
    const u32 base = wi << 6;
    if(base >= bit_len) return 0;
    const u32 rem = bit_len - base;
    return rem >= word_bits ? word_bits : rem;
  }
  constexpr u64 word_mask(u32 wi) const noexcept { return low_mask(valid_bits_in_word(wi)); }
  constexpr void check_pos_on_debug(u32 pos) const {
#ifndef NDEBUG
    if(pos > bit_len) [[unlikely]]
      throw Exception("gsh::IndexableDict / The index is out of range. ( pos=", pos, ", size=", bit_len, " )");
#else
    Assume(pos <= bit_len);
#endif
  }
  static constexpr i32 nth_one_pos(u64 x, u32 n) noexcept {
    const u32 pc = std::popcount(x);
    if(pc <= n) return -1;
#if defined(__BMI2__)
    const u64 bit = _pdep_u64(one << n, x);
    return bit ? static_cast<i32>(std::countr_zero(bit)) : -1;
#else
    u32 lo = 0;
    u32 hi = 64;
    while(lo + 1 < hi) {
      const u32 mid = (lo + hi) >> 1;
      const u64 m = (mid == 64) ? ~0ULL : low_mask(mid);
      const u32 cnt = std::popcount(x & m);
      if(cnt <= n) lo = mid;
      else hi = mid;
    }
    return static_cast<i32>(hi - 1);
#endif
  }
  constexpr u32 ones_prefix_words(u32 wi) const noexcept { return rank_[wi]; }
  constexpr u32 bits_prefix_words(u32 wi) const noexcept {
    const u64 bits = static_cast<u64>(wi) * word_bits;
    return static_cast<u32>(bits < bit_len ? bits : bit_len);
  }
  constexpr u32 zeros_prefix_words(u32 wi) const noexcept { return bits_prefix_words(wi) - ones_prefix_words(wi); }
  constexpr u32 rank1_unchecked(u32 pos) const noexcept {
    const u32 wi = pos >> 6;
    const u32 bi = pos & 63;
    u32 res = rank_[wi];
    const u64* bits = bv_.data();
    const u64 x = bits[wi] & ((1ull << bi) - 1);
    res += static_cast<u32>(std::popcount(x));
    return res;
  }
public:
  constexpr IndexableDict() noexcept = default;
  explicit IndexableDict(const BitVector& bv) : bv_(bv) { build(); }
  explicit IndexableDict(BitVector&& bv) : bv_(std::move(bv)) { build(); }
  void build(const BitVector& bv) {
    bv_ = bv;
    build();
  }
  void build(BitVector&& bv) {
    bv_ = std::move(bv);
    build();
  }
  void build() {
    const u64* bits = bv_.data();
    bit_len = bv_.size();
    word_len = words_for_bits(bit_len);
    rank_.resize(word_len + 1);
    rank_[0] = 0;
    for(u32 i = 0; i != word_len; ++i) rank_[i + 1] = rank_[i] + static_cast<u32>(std::popcount(bits[i]));
  }
  constexpr u32 size() const noexcept { return bit_len; }
  constexpr bool empty() const noexcept { return bit_len == 0; }
  constexpr const u64* data() const noexcept { return bv_.data(); }
  constexpr bool operator[](u32 pos) const { return bv_.test(pos); }
  constexpr bool any() const noexcept { return word_len != 0 && rank_[word_len] != 0; }
  constexpr bool none() const noexcept { return !any(); }
  constexpr bool all() const noexcept { return rank_.empty() ? (bit_len == 0) : (rank_[word_len] == bit_len); }
  constexpr u32 count() const noexcept { return rank_.empty() ? 0 : rank_[word_len]; }
  template<class CharT = char, class Traits = std::char_traits<CharT>, class Alloc = std::allocator<CharT>> constexpr std::basic_string<CharT, Traits, Alloc> to_string(CharT zero = CharT('0'), CharT one_c = CharT('1')) const { return bv_.template to_string<CharT, Traits, Alloc>(zero, one_c); }
  constexpr u32 rank1(u32 pos) const {
    check_pos_on_debug(pos);
    return rank1_unchecked(pos);
  }
  constexpr u32 rank0(u32 pos) const {
    check_pos_on_debug(pos);
    return pos - rank1_unchecked(pos);
  }
  constexpr u32 count1() const noexcept { return rank_.empty() ? 0 : rank_[word_len]; }
  constexpr u32 count0() const noexcept { return bit_len - count1(); }
  constexpr i32 select1(u32 k) const noexcept {
    if(word_len == 0) return -1;
    const u32 total = rank_[word_len];
    if(k >= total) return -1;
    const u64* bits = bv_.data();
    u32 lo = 0, hi = word_len;
    while(lo + 1 < hi) {
      const u32 mid = (lo + hi) >> 1;
      if(rank_[mid] <= k) lo = mid;
      else hi = mid;
    }
    const u32 wi = lo;
    const u32 n = k - rank_[wi];
    const u64 x = bits[wi] & word_mask(wi);
    const i32 pos_in = nth_one_pos(x, n);
    if(pos_in < 0) return -1;
    const u32 pos = (wi << 6) + static_cast<u32>(pos_in);
    return (pos < bit_len) ? static_cast<i32>(pos) : -1;
  }
  constexpr i32 select0(u32 k) const noexcept {
    if(word_len == 0) return -1;
    const u32 total0 = bit_len - rank_[word_len];
    if(k >= total0) return -1;
    const u64* bits = bv_.data();
    u32 lo = 0, hi = word_len;
    while(lo + 1 < hi) {
      const u32 mid = (lo + hi) >> 1;
      if(zeros_prefix_words(mid) <= k) lo = mid;
      else hi = mid;
    }
    const u32 wi = lo;
    const u32 n = k - zeros_prefix_words(wi);
    const u64 m = word_mask(wi);
    const u64 x = (~bits[wi]) & m;
    const i32 pos_in = nth_one_pos(x, n);
    if(pos_in < 0) return -1;
    const u32 pos = (wi << 6) + static_cast<u32>(pos_in);
    return (pos < bit_len) ? static_cast<i32>(pos) : -1;
  }
  constexpr const BitVector& vector() const { return bv_; }
  constexpr operator const BitVector&() const { return bv_; }
};
} // namespace gsh
