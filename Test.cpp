//#include <cmath>
#include <gsh/Modint.hpp>
#include <gsh/Timer.hpp>
#include <iostream>

using namespace gsh::itype;
using namespace gsh::ftype;
int main() {
    using mint = gsh::MontgomeryModint32<>;
    //mint::set_mod((1ll << 61) - 1);
    mint::set_mod(998244353);
    mint a = 2, b = 3;
    gsh::ClockTimer t;
    for (i64 i = 0; i != 1000000000; ++i) {
        b *= a;
        a *= b;
    }
    std::cout << a << std::endl;
    t.print();
}
