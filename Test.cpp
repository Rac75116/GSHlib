#if !defined(__clang__) && defined(__GNUC__)
#pragma GCC optimize("Ofast")
#pragma GCC optimize("unroll-loops")
#pragma GCC target("avx2")
#endif
#ifdef EVAL
#define ONLINE_JUDGE
#endif
#ifdef ONLINE_JUDGE
#define NDEBUG
#endif
#include <gsh/InOut.hpp>
#include <gsh/Exception.hpp>
#include <gsh/Random.hpp>
#include <gsh/Numeric.hpp>
#include <gsh/Timer.hpp>
#include <gsh/Macro.hpp>

#if 1 && !defined ONLINE_JUDGE
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
    Rand64 r;
    ClockTimer t;
    u64 n = 0;
    for (u32 i = 0; i != 10000000; ++i) {
        n += KthRoot(r(), 3);
    }
    Formatter<u64>{}(w, n);
    Formatter<c8>{}(w, '\n');
    t.print();
    /*
    u32 T = Parser<u8dig>{}(r).val;
    if (T == 10) return;
    while (T--) {
        u64 A = Parser<u64>{}(r);
        u8 K = Parser<u8>{}(r);
        Formatter<u64>{}(w, KthRoot(A, K));
        Formatter<c8>{}(w, '\n');
    }
    */
    /*
    //internal::StaticModint32Impl<998244353> mint;
    internal::DynamicModint32Impl mint;
    mint.set(998244353);
    //mint.set((51ull << 53) + 1);
    //mint.set(4 * 123456789ull + 3);
    auto a = mint.build(2u), b = mint.build(3u), c = mint.build(5u);
    ClockTimer t;
    [&]() __attribute__((noinline)) {
        for (u32 i = 0; i != 100000000; ++i) {
            auto d = mint.neg(mint.inc(mint.sub(mint.dec(mint.add(mint.mul(a, a), b)), c)));
            auto e = mint.neg(mint.inc(mint.sub(mint.dec(mint.add(mint.mul(b, b), c)), a)));
            auto f = mint.neg(mint.inc(mint.sub(mint.dec(mint.add(mint.mul(c, c), a)), b)));
            a = d, b = e, c = f;
        }
        R"(
        for (u32 i = 0; i != 200000; ++i) {
            auto tmp_a = mint.sqrt(a);
            auto tmp_b = mint.sqrt(b);
            auto tmp_c = mint.sqrt(c);
            if (tmp_a) assert(mint.same(mint.mul(*tmp_a, *tmp_a), a));
            if (tmp_b) assert(mint.same(mint.mul(*tmp_b, *tmp_b), b));
            if (tmp_c) assert(mint.same(mint.mul(*tmp_c, *tmp_c), c));
            a = mint.mul(tmp_a ? tmp_a.val() : a, a);
            b = mint.mul(tmp_b ? tmp_b.val() : b, b);
            c = mint.mul(tmp_c ? tmp_c.val() : c, c);
        }
        )";
    }
    ();
    t.print();
    Formatter<u64>{}(w, mint.val(a));
    Formatter<c8>{}(w, '\n');
    Formatter<u64>{}(w, mint.val(b));
    Formatter<c8>{}(w, '\n');
    Formatter<u64>{}(w, mint.val(c));
    Formatter<c8>{}(w, '\n');
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