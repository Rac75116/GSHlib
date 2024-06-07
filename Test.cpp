#pragma GCC optimize("Ofast")
#pragma GCC optimize("unroll-loops")
//#include <cmath>
#include <gsh/TypeDef.hpp>
#include <gsh/Algorithm.hpp>
#include <gsh/Vec.hpp>
#include <gsh/Arr.hpp>
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
        gsh::StaticArr v{ gsh::AutoArrInit, 2, 3, 4 };
        t.print();
    } catch (gsh::Exception& e) {
        std::cerr << e.what() << std::endl;
    }
}
