#pragma once
#include <bit>    // std::rotr, std::bit_cast
#include <ctime>  // std::time, std::clock
#include <ranges>
#if __has_include(<source_location>)
#include <source_location>  // std::source_location
#endif
#include "TypeDef.hpp"     // gsh::itype, gsh::ftype
#include "Functional.hpp"  // gsh::Hash, gsh::internal::HashBytes, gsh::internal::MixIntegers, gsh::Invoke

namespace gsh {

namespace internal {
    constexpr itype::u64 Splitmix(itype::u64 x) {
        itype::u64 z = (x + 0x9e3779b97f4a7c15);
        z = (z ^ (z >> 30)) * 0xbf58476d1ce4e5b9;
        z = (z ^ (z >> 27)) * 0x94d049bb133111eb;
        return z ^ (z >> 31);
    }
}  // namespace internal

// @brief 64bit pseudo random number generator using xoroshiro128+
class Rand64 {
    itype::u64 s0, s1;
public:
    using result_type = itype::u64;
    static constexpr itype::u32 word_size = sizeof(result_type) * 8;
    static constexpr result_type default_seed = 0xcafef00dd15ea5e5;
    constexpr Rand64() : Rand64(default_seed) {}
    constexpr explicit Rand64(result_type value) : s0(value), s1(internal::Splitmix(value)) {}
    constexpr result_type operator()() {
        itype::u64 t0 = s0, t1 = s1;
        const itype::u64 res = t0 + t1;
        t1 ^= t0;
        s0 = std::rotr(t0, 9) ^ t1 ^ (t1 << 14);
        s1 = std::rotr(t1, 28);
        return res;
    };
    constexpr void discard(itype::u64 z) {
        for (itype::u64 i = 0; i < z; ++i) operator()();
    }
    static constexpr result_type max() { return 18446744073709551615u; }
    static constexpr result_type min() { return 0; }
    constexpr void seed(result_type value = default_seed) { s0 = value, s1 = internal::Splitmix(value); }
    friend constexpr bool operator==(Rand64 x, Rand64 y) { return x.s0 == y.s0 && x.s1 == y.s1; }
};

// @brief 32bit pseudo random number generator using Permuted congruential generator
class Rand32 {
    itype::u64 val;
public:
    using result_type = itype::u32;
    static constexpr itype::u32 word_size = sizeof(result_type) * 8;
    static constexpr result_type default_seed = 0xcafef00d;
    constexpr Rand32() : Rand32(default_seed) {}
    constexpr explicit Rand32(result_type value) : val(internal::Splitmix((itype::u64) value << 32 | value)) {}
    constexpr result_type operator()() {
        itype::u64 x = val;
        const itype::i32 count = x >> 61;
        val = x * 0xcafef00dd15ea5e5;
        x ^= x >> 22;
        return x >> (22 + count);
    };
    constexpr void discard(itype::u64 z) {
        itype::u64 pow = 0xcafef00dd15ea5e5;
        while (z != 0) {
            if (z & 1) val *= pow;
            z >>= 1;
            pow *= pow;
        }
    }
    static constexpr result_type max() { return 4294967295u; }
    static constexpr result_type min() { return 0; }
    constexpr void seed(result_type value = default_seed) { val = internal::Splitmix((itype::u64) value << 32 | value); }
    friend constexpr bool operator==(Rand32 x, Rand32 y) { return x.val == y.val; }
};

template<itype::u32 Size, class URBG> class RandBuffer : public URBG {
    typename URBG::result_type buf[Size];
    itype::u32 x, cnt;
public:
    constexpr RandBuffer() { init(); }
    constexpr explicit RandBuffer(URBG::result_type value) : URBG(value) { init(); }
    constexpr void reload() { x = Invoke(static_cast<URBG&>(*this)), cnt = 0; }
    constexpr void init() {
        for (itype::u32 i = 0; i != Size; ++i) buf[i] = Invoke(static_cast<URBG&>(*this));
        x = Invoke(static_cast<URBG&>(*this)), cnt = 0;
    }
    constexpr URBG::result_type operator()() { return x ^ buf[cnt++]; }
};
template<itype::u32 Size> using RandBuffer32 = RandBuffer<Size, Rand32>;
template<itype::u32 Size> using RandBuffer64 = RandBuffer<Size, Rand64>;

// @brief Generate 32bit uniform random numbers in [0, max) (https://www.pcg-random.org/posts/bounded-rands.html)
template<class URBG> constexpr itype::u32 Uniform32(URBG&& g, itype::u32 max) {
    return (static_cast<itype::u64>(Invoke(g) & 4294967295u) * max) >> 32;
}
// @brief Generate 32bit uniform random numbers in [min, max) (https://www.pcg-random.org/posts/bounded-rands.html)
template<class URBG> constexpr itype::u32 Uniform32(URBG&& g, itype::u32 min, itype::u32 max) {
    return static_cast<itype::u32>((static_cast<itype::u64>(Invoke(g) & 4294967295u) * (max - min)) >> 32) + min;
}
// @brief Generate 64bit uniform random numbers in [0, max) (https://www.pcg-random.org/posts/bounded-rands.html)
template<class URBG> constexpr itype::u64 Uniform64(URBG&& g, itype::u64 max) {
    return (static_cast<itype::u128>(Invoke(g)) * max) >> 64;
}
// @brief Generate 64bit uniform random numbers in [min, max) (https://www.pcg-random.org/posts/bounded-rands.html)
template<class URBG> constexpr itype::u64 Uniform64(URBG&& g, itype::u64 min, itype::u64 max) {
    return static_cast<itype::u64>((static_cast<itype::u128>(Invoke(g)) * (max - min)) >> 64) + min;
}

template<std::ranges::random_access_range R, class URBG> constexpr void Shuffle(R&& r, URBG&& g) {
    itype::u32 sz = std::ranges::size(r);
    auto itr = std::ranges::begin(r);
    for (itype::u32 i = 0; i != sz; ++i, ++itr) {
        std::ranges::swap(*itr, *std::ranges::next(itr, Uniform32(g, sz - i)));
    }
}


template<class URBG> constexpr itype::u32 UnbiasedUniform32(URBG&& g, itype::u32 max) {
    itype::u32 mask = ~0u;
    --max;
    mask >>= std::countl_zero(max | 1);
    itype::u32 x;
    do {
        x = Invoke(g) & mask;
    } while (x > max);
    return x;
}
template<class URBG> constexpr itype::u32 UnbiasedUniform32(URBG&& g, itype::u32 min, itype::u32 max) {
    return min + UnbiasedUniform32(g, max - min);
}
template<class URBG> constexpr itype::u64 UnbiasedUniform64(URBG&& g, itype::u64 max) {
    itype::u64 mask = ~0ull;
    --max;
    mask >>= std::countl_zero(max | 1);
    itype::u64 x;
    do {
        x = Invoke(g) & mask;
    } while (x > max);
    return x;
}
template<class URBG> constexpr itype::u32 UnbiasedUniform64(URBG&& g, itype::u64 min, itype::u64 max) {
    return min + UnbiasedUniform64(g, max - min);
}

//https://speakerdeck.com/hole/rand01?slide=31
template<class URBG> constexpr ftype::f32 Canocicaled32(URBG&& g) {
    return std::bit_cast<ftype::f32>((127u << 23) | (static_cast<itype::u32>(Invoke(g)) & 0x7fffff)) - 1.0f;
}
template<class URBG> constexpr ftype::f32 Uniformf32(URBG&& g, ftype::f32 max) {
    return Canocicaled32(g) * max;
}
template<class URBG> constexpr ftype::f32 Uniformf32(URBG&& g, ftype::f32 min, ftype::f32 max) {
    return Canocicaled32(g) * (max - min) + min;
}
template<class URBG> constexpr ftype::f64 Canocicaled64(URBG&& g) {
    return std::bit_cast<ftype::f64>((1023ull << 52) | (static_cast<itype::u64>(Invoke(g)) & 0xfffffffffffffull)) - 1.0;
}
template<class URBG> constexpr ftype::f64 Uniformf64(URBG&& g, ftype::f64 max) {
    return Canocicaled64(g) * max;
}
template<class URBG> constexpr ftype::f64 Uniformf64(URBG&& g, ftype::f64 min, ftype::f64 max) {
    return Canocicaled64(g) * (max - min) + min;
}

template<class URBG> constexpr bool Bernoulli(URBG&& g) {
    return Invoke(g) & 1;
}
template<class URBG> constexpr bool Bernoulli32(URBG&& g, ftype::f32 p) {
    return Canocicaled32(g) < p;
}
template<class URBG> constexpr bool Bernoulli64(URBG&& g, ftype::f64 p) {
    return Canocicaled64(g) < p;
}

}  // namespace gsh
