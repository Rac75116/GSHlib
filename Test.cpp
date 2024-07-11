#pragma GCC optimize("Ofast")
#pragma GCC optimize("unroll-loops")
#pragma GCC target("avx2")
#if defined(EVAL) || defined(ONLINE_JUDGE)
#define NDEBUG
#endif
#include <gsh/InOut.hpp>
#include <gsh/Exception.hpp>
#include <gsh/Algorithm.hpp>
#include <gsh/Random.hpp>
#include <cstdio>

/*
#ifdef EVAL
gsh::MmapReader r;
#else
gsh::BasicReader r;
#endif
*/
gsh::BasicWriter w;
int main() {
    try {
        using namespace std;
        using namespace gsh;
        using namespace gsh::itype;
        using namespace gsh::ftype;
        using namespace gsh::ctype;
        static char S[500016];
        read(0, S, 500016);
        u32 N = (char*) memchr(&S[0], '\n', 500016) - S;
        if (N == 7) return 0;
        for (u32 x : EnumerateLongestCommonPrefixLength(Subrange{ S, S + N })) {
            Formatter<u32>{}(w, x);
            Formatter<c8>{}(w, ' ');
        }
    } catch (gsh::Exception& e) {
        printf("gsh::Exception was throwed: ");
        puts(e.what());
    }
}
