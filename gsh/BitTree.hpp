#pragma once
#include <bit>          // std::popcount
#include <cstring>      // std::memset
#include <type_traits>  // std::is_constant_evaluated
#include <immintrin.h>
#include <gsh/TypeDef.hpp>

namespace gsh {

template<itype::u32 Size>
    requires(Size <= (1u << 24))
class BitTree24 {
    constexpr static itype::u32 s1 = ((Size + 262143) / 262144 + 7) / 8 * 8, s2 = ((Size + 4095) / 4096 + 7) / 8 * 8, s3 = ((Size + 63) / 64 + 7) / 8 * 8;
    alignas(32) itype::u64 v0, v1[s1], v2[s2], v3[s3];
    constexpr void build() noexcept {}
public:
    constexpr BitTree24() noexcept : v0{}, v1{}, v2{}, v3{} {}
    constexpr BitTree24(itype::u64 val) noexcept : v0{}, v1{}, v2{}, v3{} {
        if (val != 0) {
            v0 = v1[0] = v2[0] = 1ull;
            v3[0] = val;
        }
    }
    constexpr ~BitTree24() noexcept = default;
    constexpr BitTree24& operator=(const BitTree24&) noexcept = default;
    constexpr BitTree24& operator&=(const BitTree24& rhs) noexcept {
        for (itype::u32 i = 0; i != s3; ++i) v3[i] &= rhs.v3[i];
        build();
        return *this;
    }
    constexpr BitTree24& operator|=(const BitTree24& rhs) noexcept {
        v0 |= rhs.v0;
        for (itype::u32 i = 0; i != s1; ++i) v1[i] |= rhs.v1[i];
        for (itype::u32 i = 0; i != s2; ++i) v2[i] |= rhs.v2[i];
        for (itype::u32 i = 0; i != s3; ++i) v3[i] |= rhs.v3[i];
        return *this;
    }
    constexpr BitTree24& operator^=(const BitTree24& rhs) noexcept {
        for (itype::u32 i = 0; i != s3; ++i) v3[i] ^= rhs.v3[i];
        build();
        return *this;
    }
    constexpr BitTree24& operator<<=(itype::u32 pos) noexcept;
    constexpr BitTree24& operator>>=(itype::u32 pos) noexcept;
    constexpr BitTree24& set() noexcept {
        for (itype::u32 i = 0; i != (Size + 4095) / 262144; ++i) v1[i] = 0xffffffffffffffff;
        for (itype::u32 i = 0; i != (Size + 63) / 4096; ++i) v2[i] = 0xffffffffffffffff;
        for (itype::u32 i = 0; i != Size / 64; ++i) v3[i] = 0xffffffffffffffff;
        if constexpr (Size + 262143 >= (1ull << 24)) v0 = 0xffffffffffffffff;
        else v0 = (1ull << ((Size + 262143) / 262144)) - 1;
        if constexpr (constexpr itype::u32 x = (Size + 4095) / 4096; x % 64 != 0) v1[x / 64] = (1ull << (x % 64)) - 1;
        if constexpr (constexpr itype::u32 x = (Size + 63) / 64; x % 64 != 0) v2[x / 64] = (1ull << (x % 64)) - 1;
        if constexpr (Size % 64 != 0) v3[Size / 64] = (1ull << (Size % 64)) - 1;
        return *this;
    }
    constexpr BitTree24& set(itype::u32 pos) {
        v0 |= (1ull << (pos / 262144));
        v1[pos / 262144] |= (1ull << (pos / 4096) % 64);
        v2[pos / 4096] |= (1ull << ((pos / 64) % 64));
        v3[pos / 64] |= (1ull << (pos % 64));
        return *this;
    }
    constexpr BitTree24& set(itype::u32 pos, bool val) {
        if (val) return set(pos);
        else return reset(pos);
    }
    constexpr BitTree24& reset() noexcept {
        std::memset(this, 0, sizeof(BitTree24));
        return *this;
    }
    constexpr BitTree24& reset(itype::u32 pos) {
        const itype::u64 m1 = ~(1ull << ((pos / 4096) % 64)), m2 = ~(1ull << ((pos / 64) % 64)), m3 = ~(1ull << (pos % 64));
        const bool f1 = v1[pos / 262144] == m1, f2 = v2[pos / 4096] == m2, f3 = v3[pos / 64] == m3;
        v3[pos / 64] &= m3;
        v2[pos / 4096] &= (f3 ? m2 : 0xffffffffffffffff);
        v1[pos / 262144] &= (f2 && f3 ? m1 : 0xffffffffffffffff);
        v0 &= (f1 && f2 && f3 ? ~(1ull << (pos / 262144)) : 0xffffffffffffffff);
        return *this;
    }
    constexpr BitTree24& flip() noexcept {
        for (itype::u32 i = 0; i != s3; ++i) v3[i] = ~v3[i];
        build();
    }
    constexpr BitTree24& flip(itype::u32 pos) noexcept { return set(pos, ~test(pos)); }
    class reference {
        friend class BitTree24;
        BitTree24& ref;
        itype::u32 idx;
        constexpr reference(BitTree24& ref_, itype::u32 idx_) noexcept : ref(ref_), idx(idx_) {}
    public:
        constexpr ~reference() noexcept {}
        constexpr reference& operator=(bool x) noexcept {
            ref.set(idx, x);
            return *this;
        }
        constexpr refernece& operator=(const reference& x) {
            ref.set(idx, x);
            return *this;
        }
        constexpr bool operator~() const noexcept { return ~ref.test(idx); }
        constexpr operator bool() const noexcept { return ref.test(idx); }
        constexpr reference& flip() const noexcept {
            ref.flip(idx);
            return *this;
        }
    };
    constexpr bool operator[](itype::u32 pos) const { return test(pos); }
    constexpr reference operator[](itype::u32 pos) { return reference(*this, pos); }
    constexpr itype::u32 count() const noexcept {
        itype::u32 res = 0;
        for (itype::u32 i = 0; i != s3; ++i) res += std::popcount(v3[i]);
        return res;
    }
    constexpr itype::u32 size() const noexcept { return Size; }
    constexpr bool test(itype::u32 pos) const { return v3[pos / 64] >> (pos % 64) & 1; }
    constexpr bool all() const noexcept;
    constexpr bool any() const noexcept { return v0 != 0; }
    constexpr bool none() const noexcept { return v0 == 0; }
    constexpr itype::u64 to_u64() const {
        if (v0 > 1 || v1[0] > 1 || v2[0] > 1) throw Exception("gsh::BitTree::to_u64 / Result overflowed.");
        return v3[0];
    }
    constexpr unsigned long to_ulong() const { return to_u64(); }
    constexpr unsigned long long to_ullong() const { return to_u64(); }
    constexpr bool operator==(const BitTree24& rhs) const noexcept {
        if (v0 != rhs.v0) return false;
        if (std::is_constant_evaluated()) {
            for (itype::u32 i = 0; i != s1; ++i)
                if (v1[i] != rhs.v1[i]) return false;
            for (itype::u32 i = 0; i != s2; ++i)
                if (v2[i] != rhs.v2[i]) return false;
            for (itype::u32 i = 0; i != s3; ++i)
                if (v3[i] != rhs.v3[i]) return false;
            return true;
        }
        for (itype::u32 i = 0; i != s1; i += 4) {
            unsigned t = _mm256_movemask_epi8(_mm256_cmpeq_epi8(_mm256_load_si256((const __m256i*) &v1[i]), _mm256_load_si256((const __m256i*) &rhs.v1[i])));
            if (t != 0xffffffff) return false;
        }
        for (itype::u32 i = 0; i != s2; i += 4) {
            unsigned t = _mm256_movemask_epi8(_mm256_cmpeq_epi8(_mm256_load_si256((const __m256i*) &v2[i]), _mm256_load_si256((const __m256i*) &rhs.v2[i])));
            if (t != 0xffffffff) return false;
        }
        for (itype::u32 i = 0; i != s3; i += 4) {
            unsigned t = _mm256_movemask_epi8(_mm256_cmpeq_epi8(_mm256_load_si256((const __m256i*) &v3[i]), _mm256_load_si256((const __m256i*) &rhs.v3[i])));
            if (t != 0xffffffff) return false;
        }
        return true;
    }
    constexpr bool operator!=(const BitTree24& rhs) const noexcept { return !operator==(rhs); }
    constexpr BitTree24 operator<<(itype::u32 pos) const noexcept { return BitTree24(*this) <<= pos; }
    constexpr BitTree24 operator>>(itype::u32 pos) const noexcept { return BitTree24(*this) >>= pos; }
    friend constexpr BitTree24 operator&(const BitTree24& lhs, const BitTree24& rhs) noexcept { return BitTree24(lhs) &= rhs; }
    friend constexpr BitTree24 operator|(const BitTree24& lhs, const BitTree24& rhs) noexcept { return BitTree24(lhs) |= rhs; }
    friend constexpr BitTree24 operator^(const BitTree24& lhs, const BitTree24& rhs) noexcept { return BitTree24(lhs) ^= rhs; }
    constexpr itype::u32 find_next(itype::u32 pos) {
        if (itype::u64 tmp = v3[pos / 64] & -(1ull << (pos % 64)); tmp != 0) return pos / 64 * 64 + std::countr_zero(tmp);
        // TODO
    }
};

}  // namespace gsh