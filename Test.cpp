#pragma GCC optimize("Ofast")
#pragma GCC optimize("unroll-loops")
#pragma GCC target("avx2")
#if defined(EVAL) || defined(ONLINE_JUDGE)
#define NDEBUG
#endif
#include <gsh/InOut.hpp>
#include <gsh/Exception.hpp>
#include <gsh/UnionFind.hpp>
#include <cstdio>

#if false
#include <fcntl.h>
gsh::BasicReader r(open("in.txt", O_RDONLY));
gsh::BasicWriter w(open("out.txt", O_WRONLY | O_TRUNC));
#else
gsh::MmapReader r;
//gsh::BasicWriter w;
gsh::ctype::c8 w[400000];
#endif
int main() {
    try {
        using namespace std;
        using namespace gsh;
        using namespace gsh::itype;
        using namespace gsh::ftype;
        using namespace gsh::ctype;
        u32 N = Parser<u8dig>{}(r).val, Q = Parser<u8dig>{}(r).val;
        UnionFind uf(N);
        u32 cnt = 0;
        for (u32 i = 0; i != Q; ++i) {
            c8 t = Parser<c8>{}(r);
            u32 u = Parser<u8dig>{}(r).val, v = Parser<u8dig>{}(r).val;
            if (t == '0') {
                uf.merge_same(u, v);
            } else {
                w[cnt] = '0' + uf.same(u, v);
                w[cnt + 1] = '\n';
                cnt += 2;
            }
        }
        write(1, w, cnt);
    } catch (gsh::Exception& e) {
        printf("gsh::Exception was throwed: ");
        puts(e.what());
    }
}
