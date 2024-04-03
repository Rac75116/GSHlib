#pragma once
#include <bit>              // std::rotr, std::bit_cast
#include <gsh/TypeDef.hpp>  // gsh::itype, gsh::ftype

namespace gsh {

namespace internal {
    constexpr gsh::itype::u64 splitmix(gsh::itype::u64 x) {
        gsh::itype::u64 z = (x + 0x9e3779b97f4a7c15);
        z = (z ^ (z >> 30)) * 0xbf58476d1ce4e5b9;
        z = (z ^ (z >> 27)) * 0x94d049bb133111eb;
        return z ^ (z >> 31);
    }
}  // namespace internal

// Xoroshiro128+
class Rand64 {
    gsh::itype::u64 s0, s1;
public:
    using result_type = gsh::itype::u64;
    static constexpr gsh::itype::usize word_size = sizeof(result_type) * 8;
    static constexpr result_type default_seed = 0xcafef00dd15ea5e5;
    constexpr Rand64() : Rand64(default_seed) {}
    constexpr explicit Rand64(result_type value) : s0(value), s1(internal::splitmix(value)) {}
    constexpr result_type operator()() {
        gsh::itype::u64 t0 = s0, t1 = s1;
        const gsh::itype::u64 res = t0 + t1;
        t1 ^= t0;
        s0 = std::rotr(t0, 9) ^ t1 ^ (t1 << 14);
        s1 = std::rotr(t1, 28);
        return res;
    };
    constexpr void discard(unsigned long long z) {
        for (unsigned long long i = 0; i < z; ++i) operator()();
    }
    static constexpr result_type max() { return 18446744073709551615u; }
    static constexpr result_type min() { return 0; }
    constexpr void seed(result_type value = default_seed) { s0 = value, s1 = internal::splitmix(value); }
    friend constexpr bool operator==(Rand64 x, Rand64 y) { return x.s0 == y.s0 && x.s1 == y.s1; }
};

// Permuted congruential generator
class Rand32 {
    gsh::itype::u64 val;
public:
    using result_type = gsh::itype::u32;
    static constexpr gsh::itype::usize word_size = sizeof(result_type) * 8;
    static constexpr result_type default_seed = 0xcafef00d;
    constexpr Rand32() : Rand32(default_seed) {}
    constexpr explicit Rand32(result_type value) : val(internal::splitmix((gsh::itype::u64) value << 32 | value)) {}
    constexpr result_type operator()() {
        gsh::itype::u64 x = val;
        const gsh::itype::i32 count = x >> 61;
        val = x * 0xcafef00dd15ea5e5;
        x ^= x >> 22;
        return x >> (22 + count);
    };
    constexpr void discard(unsigned long long z) {
        for (unsigned long long i = 0; i < z; ++i) val *= 0xcafef00dd15ea5e5;
    }
    static constexpr result_type max() { return 4294967295u; }
    static constexpr result_type min() { return 0; }
    constexpr void seed(result_type value = default_seed) { val = internal::splitmix((gsh::itype::u64) value << 32 | value); }
    friend constexpr bool operator==(Rand32 x, Rand32 y) { return x.val == y.val; }
};

//https://www.pcg-random.org/posts/bounded-rands.html
template<class URBG> constexpr gsh::itype::u32 uniform32(URBG& g, gsh::itype::u32 max) {
    return (static_cast<gsh::itype::u64>(g() & 4294967295u) * max) >> 32;
}
template<class URBG> constexpr gsh::itype::u32 uniform32(URBG& g, gsh::itype::u32 min, gsh::itype::u32 max) {
    return static_cast<gsh::itype::u32>((static_cast<gsh::itype::u64>(g() & 4294967295u) * (max - min)) >> 32) + min;
}
template<class URBG> constexpr gsh::itype::u64 uniform64(URBG& g, gsh::itype::u64 max) {
    return (static_cast<gsh::itype::u128>(g()) * max) >> 64;
}
template<class URBG> constexpr gsh::itype::u64 uniform64(URBG& g, gsh::itype::u64 min, gsh::itype::u64 max) {
    return static_cast<gsh::itype::u64>((static_cast<gsh::itype::u128>(g()) * (max - min)) >> 64) + min;
}

//https://speakerdeck.com/hole/rand01?slide=31
template<class URBG> constexpr gsh::ftype::f32 canocicaled32(URBG& g) {
    return std::bit_cast<gsh::ftype::f32>((127u << 23) | (static_cast<gsh::itype::u32>(g()) & 0x7fffff)) - 1.0f;
}
template<class URBG> constexpr gsh::ftype::f32 uniformf32(URBG& g, gsh::ftype::f32 max) {
    return canocicaled32(g) * max;
}
template<class URBG> constexpr gsh::ftype::f32 uniformf32(URBG& g, gsh::ftype::f32 min, gsh::ftype::f32 max) {
    return canocicaled32(g) * (max - min) + min;
}
template<class URBG> constexpr gsh::ftype::f64 canocicaled64(URBG& g) {
    return std::bit_cast<gsh::ftype::f64>((1023ull << 52) | (g() & 0xfffffffffffffull)) - 1.0;
}
template<class URBG> constexpr gsh::ftype::f64 uniformf64(URBG& g, gsh::ftype::f64 max) {
    return canocicaled64(g) * max;
}
template<class URBG> constexpr gsh::ftype::f64 uniformf64(URBG& g, gsh::ftype::f64 min, gsh::ftype::f64 max) {
    return canocicaled64(g) * (max - min) + min;
}

}  // namespace gsh
