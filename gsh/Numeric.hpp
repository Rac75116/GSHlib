#pragma once
#include <type_traits>       // std::is_same_v, std::is_integral_v, std::is_unsigned_v, std::make_unsigned_t, std::common_type_t
#include <bit>               // std::countr_zero, std::bit_width
#include <initializer_list>  // std::initializer_list
#include <gsh/TypeDef.hpp>   // gsh::itype
#include <gsh/Modint.hpp>    // gsh::DynamicModint

namespace gsh {

//@brief Find the largest x for which x * x <= n (https://math.stackexchange.com/questions/2469446/what-is-a-fast-algorithm-for-finding-the-integer-square-root)
constexpr itype::u32 IntSqrt(const itype::u32 x) {
    if (x < 2) return x;
    const itype::u32 log2x = std::bit_width(x) - 1;
    const itype::u32 log2y = log2x / 2u;
    itype::u32 y = 1 << log2y;
    itype::u32 y_squared = 1 << (2u * log2y);
    itype::i32 sqr_diff = x - y_squared;
    y += (sqr_diff / 3u) >> log2y;
    y_squared = y * y;
    sqr_diff = x - y_squared;
    y += sqr_diff / (2 * y);
    y_squared = y * y;
    sqr_diff = x - y_squared;
    if (sqr_diff >= 0) return y;
    y -= (-sqr_diff / (2 * y)) + 1;
    y_squared = y * y;
    sqr_diff = x - y_squared;
    y -= (sqr_diff < 0);
    return y;
}


// @brief Find the greatest common divisor as in std::gcd. (https://lpha-z.hatenablog.com/entry/2020/05/24/231500)
// @return std::common_type_t<T, U>
template<class T, class U> constexpr auto GCD(T x, U y) {
    static_assert(!std::is_same_v<T, bool> && !std::is_same_v<U, bool> && std::is_integral_v<T> && std::is_integral_v<U>, "GCD / The input must be an integral type.");
    if constexpr (std::is_same_v<T, U>) {
        if constexpr (std::is_unsigned_v<T>) {
            return internal::calc_gcd(x, y);
        } else {
            return static_cast<T>(GCD<std::make_unsigned_t<T>, std::make_unsigned<T>>((x < 0 ? -x : x), (y < 0 ? -y : y)));
        }
    } else {
        return GCD<std::common_type_t<T, U>, std::common_type_t<T, U>>(x, y);
    }
}
// @brief Find the greatest common divisor of multiple numbers.
template<class T, class... Args> auto GCD(T x, Args... y) {
    return GCD(x, GCD(y...));
}
// @brief Find the  least common multiple as in std::lcm.
template<class T, class U> auto LCM(T x, U y) {
    return static_cast<std::common_type_t<T, U>>(x < 0 ? -x : x) / GCD(x, y) * static_cast<std::common_type_t<T, U>>(y < 0 ? -y : y);
}
// @brief Find the least common multiple of multiple numbers.
template<class T, class... Args> auto LCM(T x, Args... y) {
    return LCM(x, LCM(y...));
}

namespace internal {

    bool isPrime32(const itype::u32 x) {
        if (x % 2 == 0 || x % 3 == 0 || x % 5 == 0 || x % 7 == 0 || x % 11 == 0 || x % 13 == 0 || x % 17 == 0 || x % 19 == 0 || x % 23 == 0 || x % 29 == 0 || x % 31 == 0 || x % 37 == 0 || x % 41 == 0 || x % 43 == 0) return x <= 43 && (x == 2 || x == 3 || x == 5 || x == 7 || x == 11 || x == 13 || x == 17 || x == 19 || x == 23 || x == 29 || x == 31 || x == 37 || x == 41 || x == 43);
        if (x < 47 * 47) return (x > 1);
        const static itype::u16 bases[] = { 1216, 1836,  8885,  4564, 10978, 5228, 15613, 13941, 1553, 173,   3615, 3144, 10065, 9259,  233,  2362, 6244,  6431, 10863, 5920, 6408, 6841, 22124, 2290,  45597, 6935,  4835, 7652, 1051, 445,  5807, 842,  1534, 22140, 1282, 1733, 347,   6311,  14081, 11157, 186,  703,  9862,  15490, 1720, 17816, 10433, 49185, 2535, 9158,  2143,  2840,  664,  29074, 24924, 1035, 41482, 1065,  10189, 8417,  130,  4551,  5159,  48886,
                                            786,  1938,  1013,  2139, 7171,  2143, 16873, 188,   5555, 42007, 1045, 3891, 2853,  23642, 148,  3585, 3027,  280,  3101,  9918, 6452, 2716, 855,   990,   1925,  13557, 1063, 6916, 4965, 4380, 587,  3214, 1808, 1036,  6356, 8191, 6783,  14424, 6929,  1002,  840,  422,  44215, 7753,  5799, 3415,  231,   2013,  8895, 2081,  883,   3855,  5577, 876,   3574,  1925, 1192,  865,   7376,  12254, 5952, 2516,  20463, 186,
                                            5411, 35353, 50898, 1084, 2127,  4305, 115,   7821,  1265, 16169, 1705, 1857, 24938, 220,   3650, 1057, 482,   1690, 2718,  4309, 7496, 1515, 7972,  3763,  10954, 2817,  3430, 1423, 714,  6734, 328,  2581, 2580, 10047, 2797, 155,  5951,  3817,  54850, 2173,  1318, 246,  1807,  2958,  2697, 337,   4871,  2439,  736,  37112, 1226,  527,   7531, 5418,  7242,  2421, 16135, 7015,  8432,  2605,  5638, 5161,  11515, 14949,
                                            748,  5003,  9048,  4679, 1915,  7652, 9657,  660,   3054, 15469, 2910, 775,  14106, 1749,  136,  2673, 61814, 5633, 1244,  2567, 4989, 1637, 1273,  11423, 7974,  7509,  6061, 531,  6608, 1088, 1627, 160,  6416, 11350, 921,  306,  18117, 1238,  463,   1722,  996,  3866, 6576,  6055,  130,  24080, 7331,  3922,  8632, 2706,  24108, 32374, 4237, 15302, 287,   2296, 1220,  20922, 3350,  2089,  562,  11745, 163,   11951 };
        using mint = DynamicModint32<-1>;
        mint::set_mod(x);
        const itype::u32 h = x * 0xad625b89;
        itype::u32 d = x - 1;
        mint cur = bases[h >> 24];
        itype::i32 s = std::countr_zero(d);
        d >>= s;
        cur = cur.pow(d);
        if (cur.val() == 1) return true;
        while (--s) {
            if (cur.val() == x - 1) return true;
            cur *= cur;
        }
        return cur.val() == x - 1;
    }

    bool isPrime64(const itype::u64 x) {
        if (x % 2 == 0 || x % 3 == 0 || x % 5 == 0 || x % 7 == 0 || x % 11 == 0 || x % 13 == 0 || x % 17 == 0 || x % 19 == 0 || x % 23 == 0 || x % 29 == 0 || x % 31 == 0 || x % 37 == 0 || x % 41 == 0 || x % 43 == 0) return false;
        using mint = DynamicModint64<-1>;
        mint::set_mod(x);
        itype::u64 d = x - 1;
        const itype::i32 s = std::countr_zero(d);
        d >>= s;
        auto test = [&](itype::u64 a) -> bool {
            mint cur = mint(a).pow(d);
            if (cur.val() <= 1) return true;
            itype::i32 i = s;
            while (--i) {
                if (cur.val() == x - 1) return true;
                cur *= cur;
            }
            return cur.val() == x - 1;
        };
        if (x < 585226005592931977ull) {
            if (x < 7999252175582851ull) {
                if (x < 350269456337ull) return test(4230279247111683200ull) && test(14694767155120705706ull) && test(16641139526367750375ull);
                else if (x < 55245642489451ull) return test(2ull) && test(141889084524735ull) && test(1199124725622454117ull) && test(11096072698276303650ull);
                else return test(2ull) && test(4130806001517ull) && test(149795463772692060ull) && test(186635894390467037ull) && test(3967304179347715805ull);
            } else return test(2ull) && test(123635709730000ull) && test(9233062284813009ull) && test(43835965440333360ull) && test(761179012939631437ull) && test(1263739024124850375ull);
        } else return test(2ull) && test(325ull) && test(9375ull) && test(28178ull) && test(450775ull) && test(9780504ull) && test(1795265022ull);
    }

}  // namespace internal

// @brief Prime number determination (https://www.techneon.com/download/is.prime.32.base.data / http://miller-rabin.appspot.com/)
bool isPrime(const itype::u64 x) {
    if (x < 2147483648) return internal::isPrime32(x);
    else return internal::isPrime64(x);
}

}  // namespace gsh
