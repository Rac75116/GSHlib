#if defined(EVAL) || defined(ONLINE_JUDGE)
#define NDEBUG
#endif
#pragma GCC optimize("Ofast")
#pragma GCC optimize("unroll-loops")
//#include <cmath>
#include <gsh/InOut.hpp>
#include <gsh/TypeDef.hpp>
#include <gsh/Random.hpp>
#include <gsh/Timer.hpp>
#include <gsh/Algorithm.hpp>

#ifdef EVAL
gsh::MmapReader r;
#else
gsh::BasicReader r;
#endif
gsh::BasicWriter w;
int main() {
    [[maybe_unused]] gsh::Rand32 engine;
    try {
        using namespace std;
        using namespace gsh;
        using namespace gsh::itype;
        using namespace gsh::ftype;
        using namespace gsh::ctype;
        u32 N = Parser<u32>{}(r);
        Arr<u32> A(N);
        for (u32 i = 0; i != N; ++i) A[i] = Parser<u32>{}(r);
        auto res = LongestIncreasingSubsequenceIndex(A);
        Formatter<u32>{}(w, res.size());
        Formatter<c8>{}(w, '\n');
        for (u32 i = 0; i != res.size(); ++i) {
            Formatter<u32>{}(w, res[i]);
            Formatter<c8>{}(w, ' ');
        }
    } catch (gsh::Exception& e) {
        printf("gsh::Exception was throwed: ");
        puts(e.what());
    }
}
