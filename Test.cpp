#pragma GCC optimize("Ofast")
#pragma GCC optimize("unroll-loops")
#pragma GCC target("avx2")
#if defined(EVAL) || defined(ONLINE_JUDGE)
#define NDEBUG
#endif
#include <gsh/InOut.hpp>
#include <gsh/Exception.hpp>
#include <cstdio>
#include <unordered_set>

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
    } catch (gsh::Exception& e) {
        printf("gsh::Exception was throwed: ");
        puts(e.what());
    }
}
