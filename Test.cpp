//#define NDEBUG
#pragma GCC optimize("Ofast")
#pragma GCC optimize("unroll-loops")
//#include <cmath>
#include <gsh/InOut.hpp>
#include <gsh/TypeDef.hpp>
#include <gsh/Random.hpp>
#include <gsh/Timer.hpp>
#include <gsh/Geometry.hpp>
#include <assert.h>

#ifdef EVAL
gsh::MmapReader r;
#else
gsh::BasicReader r;
#endif
gsh::BasicWriter w;
int main() {
    [[maybe_unused]] gsh::Rand32 engine;
    {
        using namespace std;
        using namespace gsh;
        using namespace gsh::itype;
        using namespace gsh::ftype;
        using namespace gsh::ctype;
        /*
        for (u32 i = 0; i != 1000000000; ++i) {
            u32 a = engine() / 4, b = engine() / 4, c = engine() / 4, d = engine() / 4;
            assert(((u64) c * b < (u64) a * d) == ((u64(b) << 32) / a < (u64(d) << 32) / c));
        }
        */
        u32 N = Parser<u32>{}(r);
        Arr<Point2<itype::i32>> p(N);
        for (u32 i = 0; i != N; ++i) {
            i32 x = Parser<i32>{}(r), y = Parser<i32>{}(r);
            p[i] = { x, y };
        }
        auto res = ArgumentSort(p);
        for (auto [x, y] : res) {
            Formatter<i32>{}(w, x);
            Formatter<c8>{}(w, ' ');
            Formatter<i32>{}(w, y);
            Formatter<c8>{}(w, '\n');
        }
        /*
        using T = u16;
        constexpr u32 n = 100000000;
        Vec<T> rnd(n);
        T sum = 0;
        REP(i, n) {
            rnd[i] = engine();
            sum += rnd[i];
        }
        cout << sum << endl;
        Vec<c8> buf(21 * n + 64);
        StaticStrWriter w(buf.data());
        ClockTimer t;
        REP(i, n) {
            Formatter<T>{}(w, rnd[i]);
            *w.current() = '\n';
            w.skip(1);
            //Formatter<c8>{}(w, '\n');
        }
        t.print();
        StaticStrReader r(buf.data());
        t.restart();
        T res = 0;
        REP(i, n) {
            res += Parser<T>{}(r);
        }
        t.print();
        cout << res << endl;
        */
    }
}
