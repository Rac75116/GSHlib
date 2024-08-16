#include <chrono>
#include <iostream>
int main() {
    unsigned rnd = 1;
    unsigned a = 2, b = 3, c = 4;

    auto st = std::chrono::system_clock::now();
    for (int i = 0; i != 100000000; ++i) {
        rnd = 1103515245 * rnd + 12345;
        a = (rnd >> 31 & 1) ? 13 * a + 17 : a;
        b = (rnd >> 30 & 1) ? 31 * b + 29 : b;
        c = (rnd >> 29 & 1) ? 15 * c + 11 : c;
    }
    std::cout << std::chrono::system_clock::now() - st << std::endl;

    std::cout << a << ' ' << b << ' ' << c;
}
/*
#include <chrono>
#include <iostream>
int main() {
    unsigned rnd = 1;
    unsigned a = 2, b = 3, c = 4;

    auto st = std::chrono::system_clock::now();
    for (int i = 0; i != 100000000; ++i) {
        rnd = 1103515245 * rnd + 12345;
        volatile unsigned tmp1 = 13 * a + 17;
        volatile unsigned tmp2 = 31 * b + 29;
        volatile unsigned tmp3 = 15 * c + 11;
        unsigned tmp4 = tmp1, tmp5 = tmp2, tmp6 = tmp3;
        a = (rnd >> 31 & 1) ? tmp4 : a;
        b = (rnd >> 30 & 1) ? tmp5 : b;
        c = (rnd >> 29 & 1) ? tmp6 : c;
    }
    std::cout << std::chrono::system_clock::now() - st << std::endl;

    std::cout << a << ' ' << b << ' ' << c << '\n';
}
*/