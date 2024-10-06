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
#include "gsh/Random.hpp"
#include "gsh/Timer.hpp"
#include "gsh/Prime.hpp"
#include <cassert>

#if 0 && !defined ONLINE_JUDGE
#include <fcntl.h>
gsh::BasicReader rd(open("in.txt", O_RDONLY));
gsh::BasicWriter wt(open("out.txt", O_WRONLY | O_TRUNC));
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
