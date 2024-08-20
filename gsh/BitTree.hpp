#pragma once
#include <bit>              // std::popcount
#include <cstring>          // std::memset
#include <type_traits>      // std::is_constant_evaluated
#include <gsh/TypeDef.hpp>  // gsh::itype
#include <gsh/Util.hpp>     // gsh::Assume
#include <immintrin.h>


namespace gsh {

template<itype::u32 Size>
    requires(Size <= (1u << 24))
class BitTree24 {
    constexpr static itype::u32 s1 = ((Size + 262143) / 262144 + 7) / 8 * 8, s2 = ((Size + 4095) / 4096 + 7) / 8 * 8, s3 = ((Size + 63) / 64 + 7) / 8 * 8;
    alignas(32) itype::u64 v0, v1[s1], v2[s2], v3[s3];
    constexpr void build() noexcept {
        if (true || std::is_constant_evaluated()) {
            v0 = 0;
            for (itype::u32 i = 0; i != s1; ++i) v1[i] = 0;
            for (itype::u32 i = 0; i != s2; ++i) v2[i] = 0;
            for (itype::u32 i = 0; i != s3; ++i) v2[i / 64] |= (static_cast<itype::u64>(v3[i] != 0) << (i % 64));
            for (itype::u32 i = 0; i != s2; ++i) v1[i / 64] |= (static_cast<itype::u64>(v2[i] != 0) << (i % 64));
            for (itype::u32 i = 0; i != s1; ++i) v0 |= (static_cast<itype::u64>(v1[i] != 0) << (i % 64));
            return;
        }
        v0 = 0;
        for (itype::u32 i = 0; i != s1; ++i) v1[i] = 0;
        for (itype::u32 i = 0; i != s2; ++i) v2[i] = 0;
        for (itype::u32 i = s3 / 64 * 64; i != s3; ++i) v2[i / 64] |= (static_cast<itype::u64>(v3[i] != 0) << (i % 64));
        for (itype::u32 i = 0; i != s2; ++i) v1[i / 64] |= (static_cast<itype::u64>(v2[i] != 0) << (i % 64));
        for (itype::u32 i = 0; i != s1; ++i) v0 |= (static_cast<itype::u64>(v1[i] != 0) << (i % 64));
        for (itype::u32 x = 0; x < s3; x += 64) {
            auto get = [&](itype::u32 n) -> itype::u32 {
                return _mm256_movemask_pd(_mm256_castsi256_pd(_mm256_cmpeq_epi64(_mm256_load_si256(reinterpret_cast<const __m256i*>(&v3[x + n])), _mm256_setzero_si256())));
            };
            const itype::u64 a = get(0), b = get(4), c = get(8), d = get(12), e = get(16), f = get(20), g = get(24), h = get(28);
            const itype::u64 i = get(32), j = get(36), k = get(40), l = get(44), m = get(48), n = get(52), o = get(56), p = get(60);
            v2[x / 64] = ~(a | b << 4 | c << 8 | d << 12 | e << 16 | f << 20 | g << 24 | h << 28 | i << 32 | j << 36 | k << 40 | l << 44 | m << 48 | n << 52 | o << 56 | p << 60);
        }
        /*
        GSH_INTERNAL_UNROLL(16)
        for (itype::u32 i = 0; i < s3; i += 4) {
            const itype::u32 t = _mm256_movemask_pd(_mm256_castsi256_pd(_mm256_cmpeq_epi64(_mm256_load_si256(reinterpret_cast<const __m256i*>(&v3[i])), _mm256_setzero_si256())));
            v2[i / 64] |= static_cast<itype::u64>(t ^ 0xf) << (i % 64);
        }
        for (itype::u32 i = s3 / 4 * 4; i < s3; ++i) v2[i / 64] |= (static_cast<itype::u64>(v3[i] != 0) << (i % 64));
        GSH_INTERNAL_UNROLL(16)
        for (itype::u32 i = 0; i < s2; i += 4) {
            const itype::u32 t = _mm256_movemask_pd(_mm256_castsi256_pd(_mm256_cmpeq_epi64(_mm256_load_si256(reinterpret_cast<const __m256i*>(&v2[i])), _mm256_setzero_si256())));
            v1[i / 64] |= static_cast<itype::u64>(t ^ 0xf) << (i % 64);
        }
        for (itype::u32 i = s2 / 4 * 4; i < s2; ++i) v1[i / 64] |= (static_cast<itype::u64>(v2[i] != 0) << (i % 64));
        GSH_INTERNAL_UNROLL(16)
        for (itype::u32 i = 0; i < s1; i += 4) {
            const itype::u32 t = _mm256_movemask_pd(_mm256_castsi256_pd(_mm256_cmpeq_epi64(_mm256_load_si256(reinterpret_cast<const __m256i*>(&v1[i])), _mm256_setzero_si256())));
            v0 |= static_cast<itype::u64>(t ^ 0xf) << (i % 64);
        }
        v0 = 0;
        for (itype::u32 i = s1 / 4 * 4; i < s1; ++i) v0 |= (static_cast<itype::u64>(v1[i] != 0) << (i % 64));
        */
    }
public:
    constexpr BitTree24() noexcept : v0{}, v1{}, v2{}, v3{} {}
    constexpr BitTree24(itype::u64 val) noexcept : v0{}, v1{}, v2{}, v3{} {
        if (val != 0) {
            v0 = v1[0] = v2[0] = 1ull;
            v3[0] = val;
        }
    }
    constexpr BitTree24(const ctype::c8* p) { assign(p); }
    constexpr BitTree24(const ctype::c8* p, itype::u32 sz) { assign(p, sz); }
    constexpr ~BitTree24() noexcept = default;
    constexpr BitTree24& operator=(const BitTree24&) noexcept = default;
    constexpr void assign(const ctype::c8* p) { assign(p, std::strlen(p)); }
    constexpr void assign(const ctype::c8* p, itype::u32 sz) {
        sz = sz < Size ? sz : Size;
        if (std::is_constant_evaluated()) {
            for (itype::u32 i = 0; i != s3; ++i) v3[i] = 0;
            for (itype::u32 i = 0; i < sz; ++i) v3[i / 64] |= static_cast<itype::u64>(p[i] - '0') << (i % 64);
            build();
            return;
        }
        for (itype::u32 i = 0; i < sz; i += 64) {
            auto get = [&](itype::u32 n) -> itype::u32 {
                return _mm256_movemask_epi8(_mm256_cmpeq_epi8(_mm256_loadu_si256(reinterpret_cast<const __m256i_u*>(p + i + n)), _mm256_set1_epi8('1')));
            };
            const itype::u64 a = get(0), b = get(8), c = get(16), d = get(24), e = get(32), f = get(40), g = get(48), h = get(56);
            v3[i / 64] = a | b << 8 | c << 16 | d << 24 | e << 32 | f << 40 | g << 48 | h << 56;
        }
        for (itype::u32 i = sz / 64; i != s3; ++i) v3[i] = 0;
        for (itype::u32 i = sz / 64 * 64; i < sz; i += 4) {
            const itype::u32 n = _mm256_movemask_epi8(_mm256_cmpeq_epi8(_mm256_loadu_si256(reinterpret_cast<const __m256i_u*>(p + i)), _mm256_set1_epi8('1')));
            v3[i / 64] |= static_cast<itype::u64>(n) << (i % 64);
        }
        const itype::u32 b = sz / 4 * 4;
        switch (sz % 4) {
        case 3 : v3[(b + 2) / 64] |= static_cast<itype::u64>(p[b + 2] - '0') << ((b + 2) % 64); [[fallthrough]];
        case 2 : v3[(b + 1) / 64] |= static_cast<itype::u64>(p[b + 1] - '0') << ((b + 1) % 64); [[fallthrough]];
        case 1 : v3[(b + 0) / 64] |= static_cast<itype::u64>(p[b + 0] - '0') << ((b + 0) % 64); [[fallthrough]];
        default : break;
        }
        build();
    }
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
        const itype::u64 m1 = (1ull << ((pos / 4096) % 64)), m2 = (1ull << ((pos / 64) % 64)), m3 = (1ull << (pos % 64));
        const bool f1 = v1[pos / 262144] == m1, f2 = v2[pos / 4096] == m2, f3 = v3[pos / 64] == m3;
        v3[pos / 64] &= ~m3;
        v2[pos / 4096] &= (f3 ? ~m2 : 0xffffffffffffffff);
        v1[pos / 262144] &= (f2 && f3 ? ~m1 : 0xffffffffffffffff);
        v0 &= (f1 && f2 && f3 ? ~(1ull << (pos / 262144)) : 0xffffffffffffffff);
        return *this;
    }
    constexpr BitTree24& flip() noexcept {
        for (itype::u32 i = 0; i != s3; ++i) v3[i] = ~v3[i];
        build();
    }
    constexpr BitTree24& flip(itype::u32 pos) noexcept { return set(pos, !test(pos)); }
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
        constexpr reference& operator=(const reference& x) {
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
            const itype::u32 t = _mm256_movemask_epi8(_mm256_cmpeq_epi8(_mm256_load_si256((const __m256i*) &v1[i]), _mm256_load_si256((const __m256i*) &rhs.v1[i])));
            if (t != 0xffffffff) return false;
        }
        for (itype::u32 i = 0; i != s2; i += 4) {
            const itype::u32 t = _mm256_movemask_epi8(_mm256_cmpeq_epi8(_mm256_load_si256((const __m256i*) &v2[i]), _mm256_load_si256((const __m256i*) &rhs.v2[i])));
            if (t != 0xffffffff) return false;
        }
        for (itype::u32 i = 0; i != s3; i += 4) {
            const itype::u32 t = _mm256_movemask_epi8(_mm256_cmpeq_epi8(_mm256_load_si256((const __m256i*) &v3[i]), _mm256_load_si256((const __m256i*) &rhs.v3[i])));
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
    constexpr static itype::u32 npos = -1;
    constexpr itype::u32 find_next(itype::u32 pos) const {
        if (const itype::u64 tmp = v3[pos / 64] & -(1ull << (pos % 64)); tmp != 0) return pos / 64 * 64 + std::countr_zero(tmp);
        if (const itype::u64 tmp = v2[pos / 4096] & -(2ull << (pos / 64 % 64)); tmp != 0) {
            const itype::u64 a = pos / 4096 * 64 + std::countr_zero(tmp), b = v3[a];
            Assume(b != 0);
            return a * 64 + std::countr_zero(b);
        }
        if (const itype::u64 tmp = v1[pos / 262144] & -(2ull << (pos / 4096 % 64)); tmp != 0) {
            const itype::u64 a = pos / 262144 * 64 + std::countr_zero(tmp), b = v2[a];
            Assume(b != 0);
            const itype::u64 c = a * 64 + std::countr_zero(b), d = v3[c];
            Assume(d != 0);
            return c * 64 + std::countr_zero(d);
        }
        if (const itype::u64 tmp = v0 & -(2ull << (pos / 262144 % 64)); tmp != 0) {
            const itype::u64 a = std::countr_zero(tmp), b = v1[a];
            Assume(b != 0);
            const itype::u64 c = a * 64 + std::countr_zero(b), d = v2[c];
            Assume(d != 0);
            const itype::u64 e = c * 64 + std::countr_zero(d), f = v3[e];
            Assume(f != 0);
            return e * 64 + std::countr_zero(f);
        }
        return npos;
    }
    constexpr itype::u32 find_prev(itype::u32 pos) const {
        if (const itype::u64 tmp = v3[pos / 64] & ((2ull << (pos % 64)) - 1); tmp != 0) return pos / 64 * 64 + std::bit_width(tmp) - 1;
        if (const itype::u64 tmp = v2[pos / 4096] & ((1ull << (pos / 64 % 64)) - 1); tmp != 0) {
            const itype::u64 a = pos / 4096 * 64 + std::bit_width(tmp) - 1, b = v3[a];
            Assume(b != 0);
            return a * 64 + std::bit_width(b) - 1;
        }
        if (const itype::u64 tmp = v1[pos / 262144] & ((1ull << (pos / 4096 % 64)) - 1); tmp != 0) {
            const itype::u64 a = pos / 262144 * 64 + std::bit_width(tmp) - 1, b = v2[a];
            Assume(b != 0);
            const itype::u64 c = a * 64 + std::bit_width(b) - 1, d = v3[c];
            Assume(d != 0);
            return c * 64 + std::bit_width(d) - 1;
        }
        if (const itype::u64 tmp = v0 & ((1ull << (pos / 262144 % 64)) - 1); tmp != 0) {
            const itype::u64 a = std::bit_width(tmp) - 1, b = v1[a];
            Assume(b != 0);
            const itype::u64 c = a * 64 + std::bit_width(b) - 1, d = v2[c];
            Assume(d != 0);
            const itype::u64 e = c * 64 + std::bit_width(d) - 1, f = v3[e];
            Assume(f != 0);
            return e * 64 + std::bit_width(f) - 1;
        }
        return npos;
    }
};

}  // namespace gsh