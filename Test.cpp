#pragma GCC optimize("Ofast")
#pragma GCC optimize("unroll-loops")
//#include <cmath>
#include <gsh/TypeDef.hpp>
#include <gsh/Vec.hpp>
#include <gsh/Random.hpp>
#include <gsh/Timer.hpp>
#include <vector>
#include <iostream>
//#include <gsh/InOut.hpp>

using namespace gsh::itype;
using namespace gsh::ftype;

int main() {
    gsh::Rand32 engine;
    gsh::ClockTimer t;
    f64 a = 1;
    u32 b = 0;
    for (u32 i = 8; i != 4200; ++i) {
        gsh::Vec<bool> f(i, false);
        for (u32 j = 0; j != i; ++j) {
            f[(j * j) % i] = true;
        }
        auto p = f64(f.count(true)) / i;
        if (p < a) {
            a = p, b = i;
        }
    }
    std::cout << a << ' ' << b << std::endl;
    t.print();
}
