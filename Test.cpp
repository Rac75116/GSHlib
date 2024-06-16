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

gsh::BasicReader r;
gsh::BasicWriter w;
int main() {
    try {
        [[maybe_unused]] gsh::Rand32 engine;
        {
            using namespace std;
            using namespace gsh;
            using namespace gsh::itype;
            using namespace gsh::ftype;
            using namespace gsh::ctype;
            u32 N = Parser<u32>{}(r);
            for (u32 i = 0; i != N; ++i) {
                u64 A = Parser<u64>{}(r), B = Parser<u64>{}(r);
                Formatter<u64>{}(w, A + B);
                Formatter<c8>{}(w, '\n');
            }
        }
    } catch (gsh::Exception& e) {
        std::cerr << e.what() << std::endl;
    }
}
