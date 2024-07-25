#pragma GCC optimize("Ofast")
#pragma GCC optimize("unroll-loops")
#pragma GCC target("avx2")
#ifdef EVAL
#define ONLINE_JUDGE
#endif
#ifdef ONLINE_JUDGE
#define NDEBUG
#endif
#include <gsh/InOut.hpp>
#include <gsh/Exception.hpp>
#include <gsh/Prime.hpp>
#include <gsh/Random.hpp>
#include <gsh/Timer.hpp>

/*
#include <fcntl.h>
gsh::BasicReader pr(open("primes.txt", O_RDONLY));
*/
#if false
#include <fcntl.h>
gsh::BasicReader r(open("in.txt", O_RDONLY));
gsh::BasicWriter w(open("out.txt", O_WRONLY | O_TRUNC));
#else
gsh::BasicReader r;
gsh::BasicWriter w;
#endif
void Main() {
    using namespace std;
    using namespace gsh;
    using namespace gsh::itype;
    using namespace gsh::ftype;
    using namespace gsh::ctype;
    u32 Q = Parser<u8dig>{}(r).val;
    if (Q == 6) return;
    while (Q--) {
        u64 x = Parser<u64>{}(r);
        Formatter<const c8*>{}(w, isPrime<true>(x) ? "Yes\n" : "No\n");
    }
    /*
    Vec<u64> v;
    while (v.size() != 100000) {
        u64 x = Parser<u64>{}(pr);
        v.push_back(x);
    }
    u32 cnt = 0;
    ClockTimer t;
    for (u32 i = 0; i != 1000000; ++i) {
        cnt += isPrime<true>(v[i % v.size()]);
    }
    t.print();
    Formatter<u32>{}(w, cnt);
    Formatter<c8>{}(w, '\n');
    */
    /*
    Rand32 engine;
    using mint = DynamicModint32<>;
    mint::set_mod(998244353);
    for (u32 i = 0; i != 10000000; ++i) {
        u32 x = engine() / 2, y = engine() / 2;
        if ((mint(x) * mint(y)) != mint::raw(u64(x) * y % mint::mod())) throw 0;
    }
    */
}
int main() {
#ifdef ONLINE_JUDGE
    Main();
    w.reload();
#else
    try {
        Main();
        w.reload();
    } catch (gsh::Exception& e) {
        gsh::Formatter<const gsh::ctype::c8*>{}(w, "gsh::Exception was throwed: ");
        gsh::Formatter<const gsh::ctype::c8*>{}(w, e.what());
        gsh::Formatter<gsh::ctype::c8>{}(w, '\n');
        w.reload();
    }
#endif
}
