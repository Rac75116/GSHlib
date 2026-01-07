#pragma once
#include "BitVector.hpp"
#include <bit>
namespace gsh {
class IndexableDict {
  static constexpr u32 word_bits = 64;
  static constexpr u64 one = 1ULL;
  BitVector bv_;
  u32 bit_len = 0;
  u32 word_len = 0;
  Vec<u32> rank_; // rank_[i] = #ones in [0, i*64)
  constexpr void check_pos_on_debug(u32 pos) const {
#ifndef NDEBUG
    if(pos > bit_len) [[unlikely]]
      throw Exception("gsh::IndexableDict / The index is out of range. ( pos=", pos, ", size=", bit_len, " )");
#else
    Assume(pos <= bit_len);
#endif
  }
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
    word_len = (bit_len + word_bits - 1) / word_bits;
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
  constexpr u32 count1() const noexcept { return count(); }
  constexpr u32 count0() const noexcept { return bit_len - count(); }
  constexpr const BitVector& vector() const { return bv_; }
  constexpr operator const BitVector&() const { return bv_; }
};
} // namespace gsh
