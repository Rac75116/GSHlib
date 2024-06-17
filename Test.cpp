#define NDEBUG
#pragma GCC optimize("Ofast")
#pragma GCC optimize("unroll-loops")
//#include <cmath>
#include <gsh/InOut.hpp>
#include <gsh/TypeDef.hpp>
#include <gsh/Random.hpp>
#include <gsh/Timer.hpp>
#include <gsh/Macro.hpp>
#include <gsh/Vec.hpp>
#include <iostream>
#include <cassert>
#include <sstream>

gsh::BasicReader r;
gsh::BasicWriter w;
int main() {
    try {
        [[maybe_unused]] gsh::Rand64 engine;
        {
            using namespace std;
            using namespace gsh;
            using namespace gsh::itype;
            using namespace gsh::ftype;
            using namespace gsh::ctype;
            /*
            u32 N = Parser<u32>{}(r);
            for (u32 i = 0; i != N; ++i) {
                u64 A = Parser<u64>{}(r), B = Parser<u64>{}(r);
                Formatter<u64>{}(w, A + B);
                *w.current() = '\n';
                w.skip(1);
            }
            */
            constexpr u32 n = 100000000;
            Vec<u64> rnd(n);
            u64 sum = 0;
            REP(i, n) {
                rnd[i] = engine();
                sum += rnd[i];
            }
            cout << sum << endl;
            Vec<c8> buf(21 * n + 64);
            StaticStrWriter w(buf.data());
            ClockTimer t;
            REP(i, n) {
                Formatter<u64>{}(w, rnd[i]);
                *w.current() = '\n';
                //Formatter<c8>{}(w, '\n');
            }
            t.print();
            StaticStrReader r(buf.data());
            t.restart();
            u64 res = 0;
            REP(i, n) {
                res += Parser<u64>{}(r);
            }
            t.print();
            cout << res << endl;
        }
    } catch (gsh::Exception& e) {
        puts(e.what());
    }
}
