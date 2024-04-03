#pragma once

namespace gsh {

namespace itype {
    using i8 = char;
    using u8 = unsigned char;
    using i16 = short;
    using u16 = unsigned short;
    using i32 = int;
    using u32 = unsigned;
    using ilong = long;
    using ulong = unsigned long;
    using i64 = long long;
    using u64 = unsigned long long;
    using i128 = __int128_t;
    using u128 = __uint128_t;
    using isize = i32;
    using usize = u32;
}  // namespace itype

namespace ftype {
    using f32 = float;
    using f64 = double;
    using flong = long double;
}  // namespace ftype

}  // namespace gsh
