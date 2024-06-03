#pragma GCC optimize("Ofast")
#pragma GCC optimize("unroll-loops")
//#include <cmath>
#include <gsh/TypeDef.hpp>
#include <gsh/Vec.hpp>
#include <gsh/Random.hpp>
#include <gsh/Timer.hpp>
#include <gsh/Numeric.hpp>
#include <gsh/Macro.hpp>
#include <gsh/Util.hpp>
#include <iostream>
#include <cassert>
#include <set>

using namespace gsh::itype;
using namespace gsh::ftype;

int main() {
    try {
        gsh::ClockTimer t;
        [[maybe_unused]] gsh::Rand64 engine;
        u64 res = 0;
        REP(i, 10, 12) {
            res += i;
        }
        std::cout << res << std::endl;
        t.print();
    } catch (gsh::Exception& e) {
        std::cerr << e.what() << std::endl;
    }
}
