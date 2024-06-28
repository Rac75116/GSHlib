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
#include <gsh/Memory.hpp>
#include <gsh/Arr.hpp>
#include <gsh/FenwickTree.hpp>

gsh::BasicReader r;
gsh::BasicWriter w;
int main() {
    [[maybe_unused]] gsh::Rand64 engine;
    {
        using namespace std;
        using namespace gsh;
        using namespace gsh::itype;
        using namespace gsh::ftype;
        using namespace gsh::ctype;
        const u32 N = Parser<u8dig>{}(r).val, Q = Parser<u8dig>{}(r).val;
        static u32 a[500000];
        for (u32 i = 0; i != N; ++i) a[i] = Parser<u32>{}(r);
        RangeSumQuery<u64> fw(a, a + N);
        for (u32 i = 0; i != Q; ++i) {
            if (Parser<c8>{}(r) == '0') {
                const u32 p = Parser<u8dig>{}(r).val, x = Parser<u32>{}(r);
                fw.add(p, x);
            } else {
                const u32 L = Parser<u8dig>{}(r).val, R = Parser<u8dig>{}(r).val;
                Formatter<u64>{}(w, fw.sum(L, R));
                Formatter<c8>{}(w, '\n');
            }
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
