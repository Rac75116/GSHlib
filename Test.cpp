#if defined(EVAL) || defined(ONLINE_JUDGE)
#define NDEBUG
#endif
#pragma GCC optimize("Ofast")
#pragma GCC optimize("unroll-loops")
#include <gsh/InOut.hpp>
#include <gsh/Exception.hpp>
#include <gsh/Geometry.hpp>
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
        u32 T = Parser<u32>{}(r);
        while (T--) {
            u32 N = Parser<u8dig>{}(r).val;
            Arr<Point2<i32>> p(N);
            for (u32 i = 0; i != N; ++i) p[i].x = Parser<i32>{}(r), p[i].y = Parser<i32>{}(r);
            auto res = ConvexHull(p);
            Formatter<u32>{}(w, res.size());
            Formatter<c8>{}(w, '\n');
            for (auto [x, y] : res) {
                Formatter<i32>{}(w, x);
                Formatter<c8>{}(w, ' ');
                Formatter<i32>{}(w, y);
                Formatter<c8>{}(w, '\n');
            }
        }
    } catch (gsh::Exception& e) {
        printf("gsh::Exception was throwed: ");
        puts(e.what());
    }
}
