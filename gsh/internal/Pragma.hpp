#pragma once

// clang-format off
#define GSH_INTERNAL_STR(s) #s
#ifdef __clang__
#define GSH_INTERNAL_UNROLL(n) _Pragma(GSH_INTERNAL_STR(unroll n))
#elif defined __GNUC__
#define GSH_INTERNAL_UNROLL(n) _Pragma(GSH_INTERNAL_STR(GCC unroll n))
#endif
// clang-format on