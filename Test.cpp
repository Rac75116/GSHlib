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
    u64 N = Parser<u64>{}(r);
    u32 cnt = 0;
    for (u32 i = 2; i != 1000001; ++i) {
        while (N % i == 0) {
            N /= i;
            ++cnt;
        }
    }
    cnt += N != 1;
    Formatter<u16>{}(w, std::bit_width(cnt - 1));
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
