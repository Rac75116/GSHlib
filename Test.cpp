#define NDEBUG
#pragma GCC optimize("Ofast")
#pragma GCC optimize("unroll-loops")
//#include <cmath>
#include <gsh/InOut.hpp>
#include <gsh/TypeDef.hpp>
#include <gsh/Random.hpp>
#include <gsh/Timer.hpp>
#include <gsh/Macro.hpp>
#include <iostream>
#include <cassert>
#include <sstream>

int main() {
    try {
        //[[maybe_unused]] gsh::Rand64 engine;
        {
            using namespace std;
            using namespace gsh;
            using namespace itype;
            using namespace ftype;
            BasicReader r;
            u32 N = Parser<u32>{}(r);
            for (u32 i = 0; i != N; ++i) {
                u64 A = Parser<u64>{}(r), B = Parser<u64>{}(r);
                printf("%llu\n", A + B);
            }
        }
    } catch (gsh::Exception& e) {
        std::cerr << e.what() << std::endl;
    }
}
