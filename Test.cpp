#pragma GCC optimize("Ofast")
#pragma GCC optimize("unroll-loops")
#pragma GCC target("avx2")
#ifdef EVAL
#define ONLINE_JUDGE
#endif
#ifdef ONLINE_JUDGE
#define NDEBUG
#endif
#include <gsh/InOut.hpp>
#include <gsh/Exception.hpp>
#include <gsh/Modint.hpp>

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
    u32 T = Parser<u8dig>{}(r).val;
    while (T--) {
        i128 A = Parser<i128>{}(r), B = Parser<i128>{}(r);
        Formatter<i128>{}(w, A + B);
        Formatter<c8>{}(w, '\n');
    }
    */
    using mint = StaticModint32<1000000007>;
    u64 L = Parser<u64>{}(r), R = Parser<u64>{}(r);
    u64 k = 1;
    mint res = 0;
    for (u32 i = 1; i <= 18; ++i) {
        u64 l = k * 10 - 1;
        if (!(l < L || k > R)) {
            u64 a = k < L ? L : k;
            u64 b = l < R ? l : R;
            res += mint(i) * (mint(a) + mint(b)) * mint(b - a + 1) / 2;
        }
        k *= 10;
    }
    Formatter<u32>{}(w, res.val());
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
    }
#endif
}
