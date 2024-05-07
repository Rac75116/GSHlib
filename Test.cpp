#pragma GCC optimize("Ofast")
//#include <cmath>
#include <gsh/Timer.hpp>
#include <gsh/Random.hpp>
#include <gsh/Numeric.hpp>
#include <iostream>
#include <cassert>

using namespace gsh::itype;
using namespace gsh::ftype;
int main() {
    gsh::ClockTimer t;
    u64 res = 0;
    for (u64 i = 2; i != 1000000000; ++i) {
        u64 tmp = gsh::IntSqrt(i);
        assert(tmp * tmp <= i && (tmp + 1) * (tmp + 1) > i);
        res += tmp;
    }
    t.print();
    std::cout << res << std::endl;
}
