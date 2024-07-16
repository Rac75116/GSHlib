#pragma GCC optimize("Ofast")
#pragma GCC optimize("unroll-loops")
#pragma GCC target("avx2")
#if defined(EVAL) || defined(ONLINE_JUDGE)
#define NDEBUG
#endif
#include <gsh/InOut.hpp>
#include <gsh/Exception.hpp>
#include <gsh/Geometry.hpp>
#include <cstdio>

#if false
#include <fcntl.h>
gsh::BasicReader r(open("in.txt", O_RDONLY));
gsh::BasicWriter w(open("out.txt", O_WRONLY | O_TRUNC));
#else
gsh::BasicReader r;
gsh::BasicWriter w;
#endif
int main() {
    try {
        using namespace std;
        using namespace gsh;
        using namespace gsh::itype;
        using namespace gsh::ftype;
        using namespace gsh::ctype;
        u32 T = Parser<u32>{}(r);
        for (u32 t = 0; t != T; ++t) {
            u32 N = Parser<u32>{}(r);
            Arr<Point2<i32>> p(N);
            for (u32 i = 0; i != N; ++i) {
                i32 A = Parser<i32>{}(r), B = Parser<i32>{}(r);
                p[i] = { A, B };
            }
            auto res = FurthestPair(p);
            for (u32 i = 0; i != N; ++i) {
                if (p[i] == res.first()) {
                    Formatter<u32>{}(w, i);
                    break;
                }
            }
            Formatter<c8>{}(w, ' ');
            for (u32 i = N; i != 0; --i) {
                if (p[i - 1] == res.second()) {
                    Formatter<u32>{}(w, i - 1);
                    break;
                }
            }
            Formatter<c8>{}(w, '\n');
        }
    } catch (gsh::Exception& e) {
        printf("gsh::Exception was throwed: ");
        puts(e.what());
    }
}
