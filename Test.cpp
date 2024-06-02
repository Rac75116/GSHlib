#pragma GCC optimize("Ofast")
#pragma GCC optimize("unroll-loops")
//#include <cmath>
#include <gsh/TypeDef.hpp>
#include <gsh/Vec.hpp>
#include <gsh/Random.hpp>
#include <gsh/Timer.hpp>
#include <gsh/Numeric.hpp>
#include <gsh/Macro.hpp>
#include <iostream>

using namespace gsh::itype;
using namespace gsh::ftype;

int main() {
    try {
        gsh::Vec<u32> ps;
        gsh::Rand32 engine;
        for (u32 j = 0; j != 100; ++j) {
            for (u32 i = 2; i != 30; ++i) {
                u32 t = engine() / 4;
                t = t ^ (t & ((1 << i) - 1));
                ++t;
                if (gsh::isPrime(t)) {
                    ps.emplace_back(t);
                }
            }
        }
        u32 res = 0;
        gsh::ClockTimer t;
        for (u32 i = 0; i != 10000000; ++i) {
            using mint = gsh::DynamicModint32<>;
            u32 m = ps[gsh::Uniform32(engine, ps.size())];
            mint::set_mod(m);
            UNWRAP(v, mint::raw(gsh::Uniform32(engine, m)).sqrt()) {
                res += v.val();
            }
        }
        t.print();
        std::cout << res << std::endl;
    } catch (gsh::Exception& e) {
        std::cerr << e.what() << std::endl;
    }
}
