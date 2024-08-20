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
#include <gsh/BitTree.hpp>
#include <gsh/Timer.hpp>

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
    u32 N = Parser<u8dig>{}(r), Q = Parser<u8dig>{}(r);
    if (N == 6) return;
    static c8 T[10000064];
    Parser<c8*>{ T, N }(r);
    static BitTree24<10000000> b(T, N);
    for (u32 i = 0; i != Q; ++i) {
        ctype::c8 t = Parser<c8>{}(r);
        itype::u32 k = Parser<u8dig>{}(r);
        if (t == '0') {
            b.set(k);
        } else if (t == '1') {
            b.reset(k);
        } else if (t == '2') {
            Formatter<c8>{}(w, '0' + b.test(k));
            Formatter<c8>{}(w, '\n');
        } else if (t == '3') {
            u32 n = b.find_next(k);
            if (n == b.npos) {
                Formatter<c8>{}(w, '-');
                Formatter<c8>{}(w, '1');
            } else {
                Formatter<u32>{}(w, n);
            }
            Formatter<c8>{}(w, '\n');
        } else if (t == '4') {
            u32 n = b.find_prev(k);
            if (n == b.npos) {
                Formatter<c8>{}(w, '-');
                Formatter<c8>{}(w, '1');
            } else {
                Formatter<u32>{}(w, n);
            }
            Formatter<c8>{}(w, '\n');
        }
    }
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