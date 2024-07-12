#pragma GCC optimize("Ofast")
#pragma GCC optimize("unroll-loops")
#pragma GCC target("avx2")
#if defined(EVAL) || defined(ONLINE_JUDGE)
#define NDEBUG
#endif
#include <gsh/InOut.hpp>
#include <gsh/Exception.hpp>
#include <gsh/Functional.hpp>
#include <cstdio>

#ifdef EVAL
gsh::MmapReader r;
#else
gsh::BasicReader r;
#endif
gsh::BasicWriter w;
int main() {
    try {
        using namespace std;
        using namespace gsh;
        using namespace gsh::itype;
        using namespace gsh::ftype;
        using namespace gsh::ctype;
    } catch (gsh::Exception& e) {
        printf("gsh::Exception was throwed: ");
        puts(e.what());
    }
}
