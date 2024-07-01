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
#include <gsh/Geometry.hpp>

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
        Arr<Point2<itype::i32>> p(N);
        for (u32 i = 0; i != N; ++i) {
            i32 x = Parser<i32>{}(r), y = Parser<i32>{}(r);
            p[i] = { x, y };
        }
        auto res = ArgumentSort(p);
        for (auto [x, y] : res) {
            Formatter<i32>{}(w, x);
            Formatter<c8>{}(w, ' ');
            Formatter<i32>{}(w, y);
            Formatter<c8>{}(w, '\n');
        }
    } catch (gsh::Exception& e) {
        printf("gsh::Exception was throwed: ");
        puts(e.what());
    }
}
