#pragma GCC optimize("Ofast")
#pragma GCC optimize("unroll-loops")
//#include <cmath>
#include <gsh/TypeDef.hpp>
#include <gsh/Vec.hpp>
#include <gsh/Random.hpp>
#include <gsh/Timer.hpp>
#include <gsh/Numeric.hpp>
#include <vector>
#include <iostream>
#include <cassert>
//#include <gsh/InOut.hpp>

using namespace gsh::itype;
using namespace gsh::ftype;

int main() {
    gsh::Rand32 engine;
    gsh::ClockTimer t;
    u64 cnt = 0;
    for (u32 i = 0; i != 100000000; ++i) {
        cnt += gsh::isSquare(engine());
    }
    std::cout << cnt << std::endl;
    t.print();
}
