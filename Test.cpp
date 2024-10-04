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
#else
#define NDEBUG
#endif
#include "gsh/InOut.hpp"
#include "gsh/Exception.hpp"
#include "gsh/Modint.hpp"
#include "gsh/Timer.hpp"
#include <cassert>

#if 0 && !defined ONLINE_JUDGE
#include <fcntl.h>
gsh::BasicReader r(open("in.txt", O_RDONLY));
gsh::BasicWriter w(open("out.txt", O_WRONLY | O_TRUNC));
#else
#if defined(ONLINE_JUDGE)
gsh::MmapReader rd;
#else
gsh::BasicReader rd;
#endif
gsh::BasicWriter wt;
#endif
void Main() {
    using namespace std;
    using namespace gsh;
    using namespace gsh::itype;
    using namespace gsh::ftype;
    using namespace gsh::ctype;
    /*
    internal::MontgomeryModint64Impl mint;
    mint.set(998244353);
    wt.writeln(mint.val(mint.add(mint.one(), mint.neg(mint.one()))));
    */
    //internal::StaticModint32Impl<998244353> mint;
    internal::DynamicModint64Impl mint;
    //internal::MontgomeryModint64Impl mint;
    mint.set(998244353);
    //mint.set((51ull << 53) + 1);
    //mint.set(4 * 123456789ull + 3);
    auto a = mint.build(2u);
    ClockTimer t;
    for (u32 i = 0; i != 1000000; ++i) {
        auto b = mint.sqrt(a);
        a = (mint.isnan(b) ? mint.inc(a) : b);
    }
    t.print();
    wt.writeln_sep('\n', mint.val(a));
}
int main() {
#ifdef ONLINE_JUDGE
    Main();
    wt.reload();
#else
    try {
        Main();
        wt.reload();
    } catch (gsh::Exception& e) {
        wt.writeln("gsh::Exception was throwed:", e.what());
        wt.reload();
        return 1;
    }
#endif
}
