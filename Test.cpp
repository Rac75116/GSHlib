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
    using mint = gsh::StaticModint32<43>;
    std::cout << mint(4).sqrt().value().val() << std::endl;
}
