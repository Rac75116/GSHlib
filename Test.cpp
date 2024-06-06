#pragma GCC optimize("Ofast")
#pragma GCC optimize("unroll-loops")
//#include <cmath>
#include <gsh/TypeDef.hpp>
#include <gsh/Algorithm.hpp>
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
        gsh::Vec<u32> v{ 5, 5, 5 };
        std::cout << v.count(5) << std::endl;
        t.print();
    } catch (gsh::Exception& e) {
        std::cerr << e.what() << std::endl;
    }
}
