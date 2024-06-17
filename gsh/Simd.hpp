#pragma once
#include <immintrin.h>
#include <gsh/TypeDef.hpp>  // gsh::itype, gsh::ftype

namespace gsh {

namespace simd {

    using i8x32 = __attribute__((vector_size(32))) itype::i8;
    using u8x32 = __attribute__((vector_size(32))) itype::u8;
    using i16x16 = __attribute__((vector_size(32))) itype::i16;
    using u16x16 = __attribute__((vector_size(32))) itype::u16;
    using i32x8 = __attribute__((vector_size(32))) itype::i32;
    using u32x8 = __attribute__((vector_size(32))) itype::u32;
    using i64x4 = __attribute__((vector_size(32))) itype::i64;
    using u64x4 = __attribute__((vector_size(32))) itype::u64;
    using f32x8 = __attribute__((vector_size(32))) ftype::f32;
    using f64x4 = __attribute__((vector_size(32))) ftype::f64;
    union v256 {
        __m256i mm;
        i8x32 i8;
        u8x32 u8;
        i16x16 i16;
        u16x16 u16;
        i32x8 i32;
        u32x8 u32;
        i64x4 i64;
        u64x4 u64;
        f32x8 f32;
        f64x4 f64;
    };

    using i8x64 = __attribute__((vector_size(64))) itype::i8;
    using u8x64 = __attribute__((vector_size(64))) itype::u8;
    using i16x32 = __attribute__((vector_size(64))) itype::i16;
    using u16x32 = __attribute__((vector_size(64))) itype::u16;
    using i32x16 = __attribute__((vector_size(64))) itype::i32;
    using u32x16 = __attribute__((vector_size(64))) itype::u32;
    using i64x8 = __attribute__((vector_size(64))) itype::i64;
    using u64x8 = __attribute__((vector_size(64))) itype::u64;
    using f32x16 = __attribute__((vector_size(64))) ftype::f32;
    using f64x8 = __attribute__((vector_size(64))) ftype::f64;
    union v512 {
        __m512i mm;
        i8x64 i8;
        u8x64 u8;
        i16x32 i16;
        u16x32 u16;
        i32x16 i32;
        u32x16 u32;
        i64x8 i64;
        u64x8 u64;
        f32x16 f32;
        f64x8 f64;
    };

}  // namespace simd

template<class T, class U> constexpr T SimdCast(U x) {
    return __builtin_convertvector(x, T);
}

}  // namespace gsh