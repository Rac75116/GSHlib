#if !defined(__clang__) && defined(__GNUC__)
#pragma GCC optimize("O3")
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
#include <gsh/Algorithm.hpp>

#if 0 && !defined ONLINE_JUDGE
#include <fcntl.h>
gsh::BasicReader r(open("in.txt", O_RDONLY));
gsh::BasicWriter w(open("out.txt", O_WRONLY | O_TRUNC));
#else
#ifdef ONLINE_JUDGE
gsh::MmapReader r;
#else
gsh::BasicReader r;
#endif
gsh::BasicWriter w;
#endif
void Main() {
    using namespace std;
    using namespace gsh;
    using namespace gsh::itype;
    using namespace gsh::ftype;
    using namespace gsh::ctype;
    u32 N = Parser<u8dig>{}(r), M = Parser<u8dig>{}(r);
    i32 K = Parser<u32>{}(r);
    static u32 A[250000];
    for (u32 i = 0; i != N; ++i) A[i] = Parser<u32>{}(r);
    for (u32 i = 0; i != M; ++i) {
        u32 B = Parser<u32>{}(r);
        u32* p = LowerBound(Subrange{ A, A + N }, B);
        i32 res = 0;
        if (p != A + N) Chmax(res, K - (i32) (*p - B));
        if (p != A) Chmax(res, K - (i32) (B - *(p - 1)));
        Formatter<u32>{}(w, res);
        Formatter<c8>{}(w, '\n');
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
        return 1;
    }
#endif
}
