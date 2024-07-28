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
#include <gsh/Modint.hpp>
#include <gsh/Numeric.hpp>
#include <gsh/Prime.hpp>
#include <gsh/Random.hpp>
#include <gsh/Timer.hpp>
#include <gsh/Algorithm.hpp>

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
    /*
    internal::MontgomeryModint64Impl mint;
    mint.set_mod((1ull << 61) - 1);
    auto a = mint.raw(2), b = mint.raw(3), c = mint.raw(5);
    const auto d = a, e = b, f = c;
    ClockTimer t;
    for (u32 i = 0; i != 100000000; ++i) {
        a = mint.add(mint.mul(a, a), d);
        b = mint.add(mint.mul(b, b), e);
        c = mint.add(mint.mul(c, c), f);
    }
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
