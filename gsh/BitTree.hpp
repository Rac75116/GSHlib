#pragma once
#include "Exception.hpp"
#include "FenwickTree.hpp"
#include "TypeDef.hpp"
#include "Util.hpp"
#include <bit>
#include <cstring>
#include <immintrin.h>
#include <type_traits>


namespace gsh {

template<u32 Size>
    requires(Size <= (1u << 24))
class BitTree {
protected:
    constexpr static u32 s1 = ((Size + 262143) / 262144 + 63) / 64 * 64, s2 = ((Size + 4095) / 4096 + 63) / 64 * 64, s3 = ((Size + 63) / 64 + 63) / 64 * 64;
    u64 v0;
    alignas(32) u64 v1[s1], v2[s2], v3[s3];
    constexpr void build() noexcept {
        if (std::is_constant_evaluated()) {
            v0 = 0;
            for (u32 i = 0; i != s1; ++i) v1[i] = 0;
            for (u32 i = 0; i != s2; ++i) v2[i] = 0;
            for (u32 i = 0; i != s3; ++i) v2[i / 64] |= (static_cast<u64>(v3[i] != 0) << (i % 64));
            for (u32 i = 0; i != s2; ++i) v1[i / 64] |= (static_cast<u64>(v2[i] != 0) << (i % 64));
            for (u32 i = 0; i != s1; ++i) v0 |= (static_cast<u64>(v1[i] != 0) << (i % 64));
            return;
        }
        for (u32 x = 0; x < s3; x += 64) {
            auto get = [&](u32 n) GSH_INTERNAL_INLINE_LAMBDA -> u32 {
                return _mm256_movemask_pd(_mm256_castsi256_pd(_mm256_cmpeq_epi64(_mm256_load_si256(reinterpret_cast<const __m256i*>(&v3[x + n])), _mm256_setzero_si256())));
            };
            const u64 a = get(0), b = get(4), c = get(8), d = get(12), e = get(16), f = get(20), g = get(24), h = get(28);
            const u64 i = get(32), j = get(36), k = get(40), l = get(44), m = get(48), n = get(52), o = get(56), p = get(60);
            v2[x / 64] = ~(a | b << 4 | c << 8 | d << 12 | e << 16 | f << 20 | g << 24 | h << 28 | i << 32 | j << 36 | k << 40 | l << 44 | m << 48 | n << 52 | o << 56 | p << 60);
        }
        for (u32 i = s3 / 64; i != s2; ++i) v2[i] = 0;
        for (u32 i = s3 / 64 * 64; i != s3; ++i) v2[i / 64] |= (static_cast<u64>(v3[i] != 0) << (i % 64));
        for (u32 x = 0; x < s2; x += 64) {
            auto get = [&](u32 n) GSH_INTERNAL_INLINE_LAMBDA -> u32 {
                return _mm256_movemask_pd(_mm256_castsi256_pd(_mm256_cmpeq_epi64(_mm256_load_si256(reinterpret_cast<const __m256i*>(&v2[x + n])), _mm256_setzero_si256())));
            };
            const u64 a = get(0), b = get(4), c = get(8), d = get(12), e = get(16), f = get(20), g = get(24), h = get(28);
            const u64 i = get(32), j = get(36), k = get(40), l = get(44), m = get(48), n = get(52), o = get(56), p = get(60);
            v1[x / 64] = ~(a | b << 4 | c << 8 | d << 12 | e << 16 | f << 20 | g << 24 | h << 28 | i << 32 | j << 36 | k << 40 | l << 44 | m << 48 | n << 52 | o << 56 | p << 60);
        }
        for (u32 i = s2 / 64; i != s1; ++i) v1[i] = 0;
        for (u32 i = s2 / 64 * 64; i != s2; ++i) v1[i / 64] |= (static_cast<u64>(v2[i] != 0) << (i % 64));
        v0 = 0;
        for (u32 i = 0; i != s1; ++i) v0 |= (static_cast<u64>(v1[i] != 0) << (i % 64));
    }
public:
    constexpr BitTree() noexcept : v0{}, v1{}, v2{}, v3{} {}
    constexpr BitTree(u64 val) noexcept : v0{}, v1{}, v2{}, v3{} {
        if (val != 0) {
            v0 = v1[0] = v2[0] = 1ull;
            v3[0] = val;
        }
    }
    constexpr BitTree(const c8* p) { assign(p); }
    constexpr BitTree(const c8* p, u32 sz, const c8 one = '1') { assign(p, sz, one); }
    constexpr BitTree(const u64* p, u32 sz) { assign(p, sz); }
    constexpr BitTree& operator=(const BitTree&) noexcept = default;
    constexpr void assign(const c8* p) { assign(p, StrLen(p)); }
    constexpr void assign(const c8* p, u32 sz, const c8 one = '1') {
        sz = sz < Size ? sz : Size;
        if (std::is_constant_evaluated()) {
            for (u32 i = 0; i != s3; ++i) v3[i] = 0;
            for (u32 i = 0; i < sz; ++i) v3[i / 64] |= static_cast<u64>(p[i] == one) << (i % 64);
            build();
            return;
        }
        const __m256i ones = _mm256_set1_epi8(one);
        for (u32 i = 0; i + 64 <= sz; i += 64) {
            auto get = [&](u32 n) -> u32 {
                return _mm256_movemask_epi8(_mm256_cmpeq_epi8(_mm256_loadu_si256(reinterpret_cast<const __m256i_u*>(p + i + n)), ones));
            };
            const u64 a = get(0), b = get(8), c = get(16), d = get(24), e = get(32), f = get(40), g = get(48), h = get(56);
            v3[i / 64] = a | b << 8 | c << 16 | d << 24 | e << 32 | f << 40 | g << 48 | h << 56;
        }
        for (u32 i = sz / 64; i != s3; ++i) v3[i] = 0;
        for (u32 i = sz / 64 * 64; i + 4 <= sz; i += 4) {
            const u32 n = _mm256_movemask_epi8(_mm256_cmpeq_epi8(_mm256_loadu_si256(reinterpret_cast<const __m256i_u*>(p + i)), ones));
            v3[i / 64] |= static_cast<u64>(n) << (i % 64);
        }
        const u32 b = sz / 4 * 4;
        switch (sz % 4) {
        case 3 : v3[(b + 2) / 64] |= static_cast<u64>(p[b + 2] == one) << ((b + 2) % 64); [[fallthrough]];
        case 2 : v3[(b + 1) / 64] |= static_cast<u64>(p[b + 1] == one) << ((b + 1) % 64); [[fallthrough]];
        case 1 : v3[(b + 0) / 64] |= static_cast<u64>(p[b + 0] == one) << ((b + 0) % 64); [[fallthrough]];
        default : break;
        }
        build();
    }
    constexpr void assign(const u64* p, u32 sz) {
        sz = sz < s3 ? sz : s3;
        for (u32 i = 0; i != sz; ++i) v3[i] = p[i];
        build();
    }
    constexpr BitTree& operator&=(const BitTree& rhs) noexcept {
        for (u32 i = 0; i != s3; ++i) v3[i] &= rhs.v3[i];
        build();
        return *this;
    }
    constexpr BitTree& operator|=(const BitTree& rhs) noexcept {
        v0 |= rhs.v0;
        for (u32 i = 0; i != s1; ++i) v1[i] |= rhs.v1[i];
        for (u32 i = 0; i != s2; ++i) v2[i] |= rhs.v2[i];
        for (u32 i = 0; i != s3; ++i) v3[i] |= rhs.v3[i];
        return *this;
    }
    constexpr BitTree& operator^=(const BitTree& rhs) noexcept {
        for (u32 i = 0; i != s3; ++i) v3[i] ^= rhs.v3[i];
        build();
        return *this;
    }
    constexpr BitTree& operator<<=(u32 pos) noexcept;
    constexpr BitTree& operator>>=(u32 pos) noexcept;
    constexpr BitTree& set() noexcept {
        for (u32 i = 0; i != (Size + 4095) / 262144; ++i) v1[i] = 0xffffffffffffffff;
        for (u32 i = 0; i != (Size + 63) / 4096; ++i) v2[i] = 0xffffffffffffffff;
        for (u32 i = 0; i != Size / 64; ++i) v3[i] = 0xffffffffffffffff;
        if constexpr (Size + 262143 >= (1ull << 24)) v0 = 0xffffffffffffffff;
        else v0 = (1ull << ((Size + 262143) / 262144)) - 1;
        if constexpr (constexpr u32 x = (Size + 4095) / 4096; x % 64 != 0) v1[x / 64] = (1ull << (x % 64)) - 1;
        if constexpr (constexpr u32 x = (Size + 63) / 64; x % 64 != 0) v2[x / 64] = (1ull << (x % 64)) - 1;
        if constexpr (Size % 64 != 0) v3[Size / 64] = (1ull << (Size % 64)) - 1;
        return *this;
    }
    constexpr BitTree& set(u32 pos) {
        v0 |= (1ull << (pos / 262144));
        v1[pos / 262144] |= (1ull << (pos / 4096) % 64);
        v2[pos / 4096] |= (1ull << ((pos / 64) % 64));
        v3[pos / 64] |= (1ull << (pos % 64));
        return *this;
    }
    constexpr BitTree& set(u32 pos, bool val) {
        if (val) return set(pos);
        else return reset(pos);
    }
    constexpr BitTree& reset() noexcept {
        MemorySet(this, 0, sizeof(BitTree));
        return *this;
    }
    constexpr BitTree& reset(u32 pos) {
        const u64 m1 = (1ull << ((pos / 4096) % 64)), m2 = (1ull << ((pos / 64) % 64)), m3 = (1ull << (pos % 64));
        const bool f1 = v1[pos / 262144] == m1, f2 = v2[pos / 4096] == m2, f3 = v3[pos / 64] == m3;
        v3[pos / 64] &= ~m3;
        v2[pos / 4096] &= (f3 ? ~m2 : 0xffffffffffffffff);
        v1[pos / 262144] &= (f2 && f3 ? ~m1 : 0xffffffffffffffff);
        v0 &= (f1 && f2 && f3 ? ~(1ull << (pos / 262144)) : 0xffffffffffffffff);
        return *this;
    }
    constexpr BitTree& flip() noexcept {
        for (u32 i = 0; i != s3; ++i) v3[i] = ~v3[i];
        build();
    }
    constexpr BitTree& flip(u32 pos) noexcept { return set(pos, !test(pos)); }
    class reference {
        friend class BitTree;
        BitTree& ref;
        u32 idx;
        constexpr reference(BitTree& ref_, u32 idx_) noexcept : ref(ref_), idx(idx_) {}
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
    constexpr bool operator[](u32 pos) const { return test(pos); }
    constexpr reference operator[](u32 pos) { return reference(*this, pos); }
    constexpr u32 count() const noexcept {
        u32 res = 0;
        for (u32 i = 0; i != s3; ++i) res += std::popcount(v3[i]);
        return res;
    }
    constexpr u32 size() const noexcept { return Size; }
    constexpr bool test(u32 pos) const { return v3[pos / 64] >> (pos % 64) & 1; }
    constexpr bool all() const noexcept {
        if constexpr (Size + 262143 >= (1ull << 24)) {
            if (v0 != 0xffffffffffffffff) return false;
        } else {
            if (v0 != (1ull << ((Size + 262143) / 262144)) - 1) return false;
        }
        for (u32 i = 0; i != (Size + 4095) / 262144; ++i)
            if (v1[i] != 0xffffffffffffffff) return false;
        if constexpr (constexpr u32 x = (Size + 4095) / 4096; x % 64 != 0) {
            if (v1[x / 64] != (1ull << (x % 64)) - 1) return false;
        }
        for (u32 i = 0; i != (Size + 63) / 4096; ++i)
            if (v2[i] != 0xffffffffffffffff) return false;
        if constexpr (constexpr u32 x = (Size + 63) / 64; x % 64 != 0) {
            if (v2[x / 64] != (1ull << (x % 64)) - 1) return false;
        }
        for (u32 i = 0; i != Size / 64; ++i)
            if (v3[i] != 0xffffffffffffffff) return false;
        if constexpr (Size % 64 != 0) {
            if (v3[Size / 64] != (1ull << (Size % 64)) - 1) return false;
        }
        return true;
    }
    constexpr bool any() const noexcept { return v0 != 0; }
    constexpr bool none() const noexcept { return v0 == 0; }
    constexpr u64 to_u64() const {
        if (v0 > 1 || v1[0] > 1 || v2[0] > 1) throw Exception("gsh::BitTree::to_u64 / Result overflowed.");
        return v3[0];
    }
    constexpr unsigned long to_ulong() const { return to_u64(); }
    constexpr unsigned long long to_ullong() const { return to_u64(); }
    constexpr bool operator==(const BitTree& rhs) const noexcept {
        if (v0 != rhs.v0) return false;
        if (std::is_constant_evaluated()) {
            for (u32 i = 0; i != s1; ++i)
                if (v1[i] != rhs.v1[i]) return false;
            for (u32 i = 0; i != s2; ++i)
                if (v2[i] != rhs.v2[i]) return false;
            for (u32 i = 0; i != s3; ++i)
                if (v3[i] != rhs.v3[i]) return false;
            return true;
        }
        for (u32 i = 0; i != s1; i += 4) {
            const u32 t = _mm256_movemask_epi8(_mm256_cmpeq_epi8(_mm256_load_si256((const __m256i*) &v1[i]), _mm256_load_si256((const __m256i*) &rhs.v1[i])));
            if (t != 0xffffffff) return false;
        }
        for (u32 i = 0; i != s2; i += 4) {
            const u32 t = _mm256_movemask_epi8(_mm256_cmpeq_epi8(_mm256_load_si256((const __m256i*) &v2[i]), _mm256_load_si256((const __m256i*) &rhs.v2[i])));
            if (t != 0xffffffff) return false;
        }
        for (u32 i = 0; i != s3; i += 4) {
            const u32 t = _mm256_movemask_epi8(_mm256_cmpeq_epi8(_mm256_load_si256((const __m256i*) &v3[i]), _mm256_load_si256((const __m256i*) &rhs.v3[i])));
            if (t != 0xffffffff) return false;
        }
        return true;
    }
    constexpr bool operator!=(const BitTree& rhs) const noexcept { return !operator==(rhs); }
    constexpr BitTree operator<<(u32 pos) const noexcept { return BitTree(*this) <<= pos; }
    constexpr BitTree operator>>(u32 pos) const noexcept { return BitTree(*this) >>= pos; }
    friend constexpr BitTree operator&(const BitTree& lhs, const BitTree& rhs) noexcept { return BitTree(lhs) &= rhs; }
    friend constexpr BitTree operator|(const BitTree& lhs, const BitTree& rhs) noexcept { return BitTree(lhs) |= rhs; }
    friend constexpr BitTree operator^(const BitTree& lhs, const BitTree& rhs) noexcept { return BitTree(lhs) ^= rhs; }
    constexpr static u32 npos = -1;
    constexpr u32 find_next(u32 pos) const {
        if (const u64 tmp = v3[pos / 64] & -(1ull << (pos % 64)); tmp != 0) [[unlikely]]
            return pos / 64 * 64 + std::countr_zero(tmp);
        if (const u64 tmp = v2[pos / 4096] & -(2ull << (pos / 64 % 64)); tmp != 0) [[unlikely]] {
            const u64 a = pos / 4096 * 64 + std::countr_zero(tmp), b = v3[a];
            Assume(b != 0);
            return a * 64 + std::countr_zero(b);
        }
        if (const u64 tmp = v1[pos / 262144] & -(2ull << (pos / 4096 % 64)); tmp != 0) [[unlikely]] {
            const u64 a = pos / 262144 * 64 + std::countr_zero(tmp), b = v2[a];
            Assume(b != 0);
            const u64 c = a * 64 + std::countr_zero(b), d = v3[c];
            Assume(d != 0);
            return c * 64 + std::countr_zero(d);
        }
        if (const u64 tmp = v0 & -(2ull << (pos / 262144 % 64)); tmp != 0) [[likely]] {
            const u64 a = std::countr_zero(tmp), b = v1[a];
            Assume(b != 0);
            const u64 c = a * 64 + std::countr_zero(b), d = v2[c];
            Assume(d != 0);
            const u64 e = c * 64 + std::countr_zero(d), f = v3[e];
            Assume(f != 0);
            return e * 64 + std::countr_zero(f);
        }
        return npos;
    }
    constexpr u32 find_first() const noexcept { return find_next(0); }
    constexpr u32 find_prev(u32 pos) const {
        if (const u64 tmp = v3[pos / 64] & ((2ull << (pos % 64)) - 1); tmp != 0) [[unlikely]]
            return pos / 64 * 64 + std::bit_width(tmp) - 1;
        if (const u64 tmp = v2[pos / 4096] & ((1ull << (pos / 64 % 64)) - 1); tmp != 0) [[unlikely]] {
            const u64 a = pos / 4096 * 64 + std::bit_width(tmp) - 1, b = v3[a];
            Assume(b != 0);
            return a * 64 + std::bit_width(b) - 1;
        }
        if (const u64 tmp = v1[pos / 262144] & ((1ull << (pos / 4096 % 64)) - 1); tmp != 0) [[unlikely]] {
            const u64 a = pos / 262144 * 64 + std::bit_width(tmp) - 1, b = v2[a];
            Assume(b != 0);
            const u64 c = a * 64 + std::bit_width(b) - 1, d = v3[c];
            Assume(d != 0);
            return c * 64 + std::bit_width(d) - 1;
        }
        if (const u64 tmp = v0 & ((1ull << (pos / 262144 % 64)) - 1); tmp != 0) [[likely]] {
            const u64 a = std::bit_width(tmp) - 1, b = v1[a];
            Assume(b != 0);
            const u64 c = a * 64 + std::bit_width(b) - 1, d = v2[c];
            Assume(d != 0);
            const u64 e = c * 64 + std::bit_width(d) - 1, f = v3[e];
            Assume(f != 0);
            return e * 64 + std::bit_width(f) - 1;
        }
        return npos;
    }
    constexpr u32 find_last() const noexcept { return find_prev(Size - 1); }
};

template<u32 Size>
    requires(Size <= (1u << 24))
class CountableBitTree : private BitTree<Size>, private RangeSumQuery<u32> {
    using bt = BitTree<Size>;
    using fw = RangeSumQuery<u32>;
    constexpr void init_fw() {
        fw::bit.resize(bt::s3);
        for (u32 i = 0; i != bt::s3; ++i) fw::bit[i] = std::popcount(bt::v3[i]);
        if (bt::s3 == 0) return;
        const auto tmp = fw::bit[0];
        for (u32 i = 0; i != bt::s3 - 1; ++i) {
            const u32 j = i + ((i + 1) & -(i + 1));
            fw::bit[j < bt::s3 ? j : 0] += fw::bit[i];
        }
        fw::bit[0] = tmp;
    }
public:
    constexpr CountableBitTree() noexcept(noexcept(RangeSumQuery<u32>())) : fw(bt::s3) {}
    constexpr CountableBitTree(u64 val) : bt(val), fw(bt::s3) { fw::add(0, std::popcount(val)); }
    constexpr CountableBitTree(const c8* p) : bt(p) { init_fw(); }
    constexpr CountableBitTree(const c8* p, u32 sz, const c8 one = '1') : bt(p, sz, one) { init_fw(); }
    constexpr CountableBitTree(const u64* p, u32 sz) : bt(p, sz) { init_fw(); }
    constexpr CountableBitTree& operator=(const CountableBitTree&) = default;
    constexpr void assign(const c8* p) {
        bt::assign(p);
        init_fw();
    }
    constexpr void assign(const c8* p, u32 sz, const c8 one = '1') {
        bt::assign(p, sz, one);
        init_fw();
    }
    constexpr void assign(const u64* p, u32 sz) {
        bt::assign(p, sz);
        init_fw();
    }
    constexpr CountableBitTree& operator&=(const CountableBitTree& rhs) {
        bt::operator&=(rhs);
        init_fw();
        return *this;
    }
    constexpr CountableBitTree& operator|=(const CountableBitTree& rhs) {
        bt::operator|=(rhs);
        init_fw();
        return *this;
    }
    constexpr CountableBitTree& operator^=(const CountableBitTree& rhs) {
        bt::operator^=(rhs);
        init_fw();
        return *this;
    }
    constexpr CountableBitTree& set() {
        bt::set();
        init_fw();  // Can be improved
        return *this;
    }
    constexpr CountableBitTree& set(u32 pos) {
        if (!bt::test(pos)) {
            bt::set(pos);
            fw::inc(pos / 64);
        }
        return *this;
    }
    constexpr CountableBitTree& set(u32 pos, bool val) {
        if (val) return set(pos);
        else return reset(pos);
    }
    constexpr CountableBitTree& reset() {
        bt::reset();
        fw::assign(bt::s3, 0);
        return *this;
    }
    constexpr CountableBitTree& reset(u32 pos) {
        if (bt::test(pos)) {
            bt::reset(pos);
            fw::dec(pos / 64);
        }
        return *this;
    }
    constexpr CountableBitTree& flip() {
        bt::flip();
        init_fw();
        return *this;
    }
    constexpr CountableBitTree& flip(u32 pos) { return set(pos, !bt::test(pos)); }
    class reference {
        friend class CountableBitTree;
        CountableBitTree& ref;
        u32 idx;
        constexpr reference(CountableBitTree& ref_, u32 idx_) noexcept : ref(ref_), idx(idx_) {}
    public:
        constexpr ~reference() noexcept {}
        constexpr reference& operator=(bool x) {
            ref.set(idx, x);
            return *this;
        }
        constexpr reference& operator=(const reference& x) {
            ref.set(idx, x);
            return *this;
        }
        constexpr bool operator~() const noexcept { return ~ref.test(idx); }
        constexpr operator bool() const noexcept { return ref.test(idx); }
        constexpr reference& flip() const {
            ref.flip(idx);
            return *this;
        }
    };
    constexpr bool operator[](u32 pos) const { return bt::test(pos); }
    constexpr reference operator[](u32 pos) { return reference(*this, pos); }
    constexpr u32 count() const { return fw::sum(bt::s3); }
    constexpr u32 count(u32 pos) const { return fw::sum(pos / 64) + std::popcount(bt::v3[pos / 64] & ((1ull << (pos % 64)) - 1)); }
    constexpr u32 count(u32 l, u32 r) const { return count(r) - count(l); /* Can be improved */ }
    constexpr u32 size() const noexcept { return Size; }
    constexpr bool test(u32 pos) const { return bt::test(pos); }
    constexpr bool all() const { return count() == size(); }
    constexpr bool any() const noexcept { return bt::any(); }
    constexpr bool none() const noexcept { return bt::none(); }
    constexpr u64 to_u64() const { return bt::to_u64(); }
    constexpr unsigned long to_ulong() const { return bt::to_ulong(); }
    constexpr unsigned long long to_ullong() const { return bt::to_ullong(); }
    constexpr bool operator==(const CountableBitTree& rhs) const noexcept { return bt::operator==(rhs); }
    constexpr bool operator!=(const CountableBitTree& rhs) const noexcept { return bt::operator!=(rhs); }
    friend constexpr CountableBitTree operator&(const CountableBitTree& lhs, const CountableBitTree& rhs) noexcept { return CountableBitTree(lhs) &= rhs; }
    friend constexpr CountableBitTree operator|(const CountableBitTree& lhs, const CountableBitTree& rhs) noexcept { return CountableBitTree(lhs) |= rhs; }
    friend constexpr CountableBitTree operator^(const CountableBitTree& lhs, const CountableBitTree& rhs) noexcept { return CountableBitTree(lhs) ^= rhs; }
    constexpr static u32 npos = -1;
    constexpr u32 find_next(u32 pos) const { return bt::find_next(pos); }
    constexpr u32 find_first() const noexcept { return bt::find_first(); }
    constexpr u32 find_prev(u32 pos) const { return bt::find_prev(pos); }
    constexpr u32 find_last() const noexcept { return bt::find_last(); }
    constexpr u32 nth_element(u32 n) const {
        u32 idx = 0, rem = n;
        for (u32 len = std::bit_floor(bt::s3); len != 0; len >>= 1) {
            u32 tmp = fw::bit[idx + len - 1];
            if (idx + len <= bt::s3 && rem >= tmp) {
                rem -= tmp;
                idx += len;
            }
        }
        if (idx == bt::s3) return npos;
        u64 f = bt::v3[idx];
#ifdef __BMI2__
        f = _pdep_u64(1ull << rem, f);
#else
        for (u32 i = 0; i != rem; ++i) f &= f - 1;
#endif
        Assume(f != 0);
        return idx * 64 + std::countr_zero(f);
    }
};

}  // namespace gsh
