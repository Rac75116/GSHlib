#pragma once
#include <gsh/TypeDef.hpp>

namespace gsh {

template<itype::u32 Size>
    requires(Size < (1u << 24))
class BitTree24 {
    itype::u64 data[1 + (Size + 262143) / 262144 + (Size + 4095) / 4096 + (Size + 63) / 64];
    constexpr auto level0() { return data[0]; }
    constexpr auto level1() { return data + 1; }
    constexpr auto level2() { return data + (Size + 262143) / 262144; }
    constexpr auto level3() { return data + (Size + 262143) / 262144 + (Size + 4095) / 4096; }
public:
};

}  // namespace gsh