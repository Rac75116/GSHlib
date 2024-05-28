#pragma GCC optimize("Ofast")
#pragma GCC optimize("unroll-loops")
//#include <cmath>
#include <gsh/Timer.hpp>
#include <gsh/Random.hpp>
#include <gsh/Numeric.hpp>
#include <gsh/Vec.hpp>
#include <iostream>
#include <cassert>
#include <numeric>
#include <vector>

using namespace gsh::itype;
using namespace gsh::ftype;
#include <bits/stdc++.h>
#include <unistd.h>
#include <immintrin.h>
// Thanks for https://zenn.dev/mizar/articles/fc87d667153080
class FastIstream : public std::ios_base {
    constexpr static int buffersize = (1 << 18) - 1;
    char buffer[buffersize + 1];
    char* cur = buffer;
    char* eof = buffer;
    inline void reload(ptrdiff_t w) {
        if (eof - w < cur) [[unlikely]] {
            if (eof == buffer + buffersize) [[likely]] {
                ptrdiff_t rem = eof - cur;
                std::memcpy(buffer, cur, rem);
                *(eof = buffer + rem + read(0, buffer + rem, buffersize - rem)) = '\0';
                cur = buffer;
            } else if (eof <= cur) {
                *(eof = buffer + read(0, buffer, buffersize)) = '\0';
                cur = buffer;
            }
        }
    }
public:
    FastIstream& operator>>(bool& n) {
        reload(2);
        n = *cur == '1';
        cur += 2;
        return *this;
    }
    FastIstream& operator>>(short& n) {
        reload(8);
        short neg = (*cur == '-') * -2 + 1;
        cur += neg == -1;
        uint64_t tmp = *(uint64_t*) cur ^ 0x3030303030303030u;
        int clz = std::countl_zero((tmp & 0x1010101010101010u) & (-(tmp & 0x1010101010101010u))) + 5;
        cur += (72 - clz) >> 3;
        tmp = ((tmp << clz) * 0xa01ull) >> 8 & 0x00ff00ff00ff00ffull;
        tmp = (tmp * 0x640001ull) >> 16 & 0x0000ffff0000ffffull;
        n = (short) ((tmp * 0x271000000001ull) >> 32) * neg;
        return *this;
    }
    FastIstream& operator>>(unsigned short& n) {
        reload(8);
        uint64_t tmp = *(uint64_t*) cur ^ 0x3030303030303030u;
        int clz = std::countl_zero((tmp & 0x1010101010101010u) & (-(tmp & 0x1010101010101010u))) + 5;
        cur += (72 - clz) >> 3;
        tmp = ((tmp << clz) * 0xa01ull) >> 8 & 0x00ff00ff00ff00ffull;
        tmp = (tmp * 0x640001ull) >> 16 & 0x0000ffff0000ffffull;
        n = (unsigned short) ((tmp * 0x271000000001ull) >> 32);
        return *this;
    }
    FastIstream& operator>>(unsigned int& n) {
        reload(16);
        uint64_t tmp = *(uint64_t*) cur ^ 0x3030303030303030u, tmp2 = tmp & 0x1010101010101010u;
        if (tmp2) {
            int clz = std::countl_zero(tmp2 & -tmp2) + 5;
            cur += (72 - clz) >> 3;
            tmp = ((tmp << clz) * 0xa01ull) >> 8 & 0x00ff00ff00ff00ffull;
            tmp = (tmp * 0x640001ull) >> 16 & 0x0000ffff0000ffffull;
            n = (unsigned) ((tmp * 0x271000000001ull) >> 32);
        } else {
            cur += 8;
            tmp = (tmp * 0xa01ull) >> 8 & 0x00ff00ff00ff00ffull;
            tmp = (tmp * 0x640001ull) >> 16 & 0x0000ffff0000ffffull;
            n = (unsigned) ((tmp * 0x271000000001ull) >> 32);
            if (char c = *(cur++); c >= '0') {
                n = 10 * n + (c - '0');
                if ((c = *(cur++)) >= '0') n = 10 * n + (c - '0'), ++cur;
            }
        }
        return *this;
    }
    FastIstream& operator>>(int& n) {
        reload(16);
        int neg = (*cur == '-') * -2 + 1;
        cur += neg == -1;
        uint64_t tmp = *(uint64_t*) cur ^ 0x3030303030303030u, tmp2 = tmp & 0x1010101010101010u;
        if (tmp2) {
            int clz = std::countl_zero(tmp2 & -tmp2) + 5;
            cur += (72 - clz) >> 3;
            tmp = ((tmp << clz) * 0xa01ull) >> 8 & 0x00ff00ff00ff00ffull;
            tmp = (tmp * 0x640001ull) >> 16 & 0x0000ffff0000ffffull;
            n = (int) ((tmp * 0x271000000001ull) >> 32);
        } else {
            cur += 8;
            tmp = (tmp * 0xa01ull) >> 8 & 0x00ff00ff00ff00ffull;
            tmp = (tmp * 0x640001ull) >> 16 & 0x0000ffff0000ffffull;
            n = (int) ((tmp * 0x271000000001ull) >> 32);
            if (char c = *(cur++); c >= '0') {
                n = 10 * n + (c - '0');
                if ((c = *(cur++)) >= '0') n = 10 * n + (c - '0'), ++cur;
            }
        }
        n *= neg;
        return *this;
    }
    FastIstream& operator>>(unsigned long long& n) {
        reload(32);
#ifndef __AVX512VL__
        n = 0;
        while (*cur >= '0') n = 10 * n + (*(cur++) - '0');
        ++cur;
#else
        unsigned long long tmp[3], tmp2[3];
        std::memcpy(tmp, cur, 24);
        int width;
        if ((tmp2[0] = (tmp[0] ^= 0x3030303030303030) & 0x1010101010101010)) [[unlikely]] {
            width = std::countr_zero(tmp2[0]) - 4;
            n = ((((((tmp[0] << (64 - width)) * 0xa01ull) >> 8 & 0x00ff00ff00ff00ffull) * 0x640001ull) >> 16 & 0x0000ffff0000ffffull) * 0x271000000001ull) >> 32;
            cur += (width >> 3) + 1;
        } else {
            __m256i tmp3;
            if ((tmp2[1] = (tmp[1] ^= 0x3030303030303030) & 0x1010101010101010)) [[unlikely]] {
                width = 60 + std::countr_zero(tmp2[1]);
                if (width == 64) [[unlikely]]
                    tmp3 = _mm256_setr_epi64x(0, 0, 0, tmp[0]);
                else tmp3 = _mm256_setr_epi64x(0, 0, tmp[0] << (128 - width), tmp[1] << (128 - width) | tmp[0] >> (width - 64));
            } else {
                width = 124 + std::countr_zero((tmp[2] ^= 0x3030303030303030) & 0x1010101010101010);
                if (width == 128) [[unlikely]]
                    tmp3 = _mm256_setr_epi64x(0, 0, tmp[0], tmp[1]);
                else tmp3 = _mm256_setr_epi64x(0, tmp[0] << (192 - width), tmp[1] << (192 - width) | tmp[0] >> (width - 128), tmp[2] << (192 - width) | tmp[1] >> (width - 128));
            }
            cur += (width >> 3) + 1;
            alignas(32) unsigned long long res[4];
            _mm256_store_epi64(res, _mm256_srli_epi64(_mm256_mullo_epi64(_mm256_srli_epi32(_mm256_mullo_epi32(_mm256_srli_epi16(_mm256_mullo_epi16(_mm256_and_si256(tmp3, _mm256_set1_epi8(0x0f)), _mm256_set1_epi16(0xa01)), 8), _mm256_set1_epi32(0x640001)), 16), _mm256_set1_epi64x(0x271000000001)), 32));
            n = res[1] * 10000000000000000 + res[2] * 100000000 + res[3];
        }
#endif
        return *this;
    }
    FastIstream& operator>>(long long& n) {
        reload(32);
        long long neg = (*cur == '-') * -2 + 1;
        cur += neg == -1;
#ifndef __AVX512VL__
        n = 0;
        while (*cur >= '0') n = 10 * n + (*(cur++) - '0');
        ++cur;
        n *= neg;
#else
        unsigned long long tmp[3], tmp2[3];
        std::memcpy(tmp, cur, 24);
        int width;
        if ((tmp2[0] = (tmp[0] ^= 0x3030303030303030) & 0x1010101010101010)) [[unlikely]] {
            width = std::countr_zero(tmp2[0]) - 4;
            n = neg * (((((((tmp[0] << (64 - width)) * 0xa01ull) >> 8 & 0x00ff00ff00ff00ffull) * 0x640001ull) >> 16 & 0x0000ffff0000ffffull) * 0x271000000001ull) >> 32);
            cur += (width >> 3) + 1;
        } else {
            __m256i tmp3;
            if ((tmp2[1] = (tmp[1] ^= 0x3030303030303030) & 0x1010101010101010)) [[unlikely]] {
                width = 60 + std::countr_zero(tmp2[1]);
                if (width == 64) [[unlikely]]
                    tmp3 = _mm256_setr_epi64x(0, 0, 0, tmp[0]);
                else tmp3 = _mm256_setr_epi64x(0, 0, tmp[0] << (128 - width), tmp[1] << (128 - width) | tmp[0] >> (width - 64));
            } else {
                width = 124 + std::countr_zero((tmp[2] ^= 0x3030303030303030) & 0x1010101010101010);
                if (width == 128) [[unlikely]]
                    tmp3 = _mm256_setr_epi64x(0, 0, tmp[0], tmp[1]);
                else tmp3 = _mm256_setr_epi64x(0, tmp[0] << (192 - width), tmp[1] << (192 - width) | tmp[0] >> (width - 128), tmp[2] << (192 - width) | tmp[1] >> (width - 128));
            }
            cur += (width >> 3) + 1;
            alignas(32) long long res[4];
            _mm256_store_epi64(res, _mm256_srli_epi64(_mm256_mullo_epi64(_mm256_srli_epi32(_mm256_mullo_epi32(_mm256_srli_epi16(_mm256_mullo_epi16(_mm256_and_si256(tmp3, _mm256_set1_epi8(0x0f)), _mm256_set1_epi16(0xa01)), 8), _mm256_set1_epi32(0x640001)), 16), _mm256_set1_epi64x(0x271000000001)), 32));
            n = neg * (res[1] * 10000000000000000 + res[2] * 100000000 + res[3]);
        }
#endif
        return *this;
    }
    FastIstream& operator>>(long& n) {
        long long x;
        operator>>(x);
        n = x;
        return *this;
    }
    FastIstream& operator>>(unsigned long& n) {
        unsigned long long x;
        operator>>(x);
        n = x;
        return *this;
    }
    friend FastIstream& operator>>(FastIstream& is, char& c) {
        is.reload(2);
        c = *is.cur;
        is.cur += 2;
        return is;
    }
    friend FastIstream& operator>>(FastIstream& is, unsigned char& c) {
        is.reload(2);
        c = *is.cur;
        is.cur += 2;
        return is;
    }
    friend FastIstream& operator>>(FastIstream& is, signed char& c) {
        is.reload(2);
        c = *is.cur;
        is.cur += 2;
        return is;
    }
    friend FastIstream& operator>>(FastIstream& is, char* s) {
        while (true) {
            while (*is.cur > ' ' && is.cur != is.eof) *(s++) = *is.cur, ++is.cur;
            if (is.cur == is.eof) is.reload(is.buffersize);
            else break;
        }
        ++is.cur;
        *s = '\0';
        return is;
    }
    friend FastIstream& operator>>(FastIstream& is, std::string& s) {
        s.clear();
        while (true) {
            char* st = is.cur;
            while (*is.cur > ' ' && is.cur != is.eof) ++is.cur;
            s += std::string_view(st, is.cur - st);
            if (is.cur == is.eof) is.reload(is.buffersize);
            else break;
        }
        ++is.cur;
        return is;
    }
    FastIstream& operator>>(float& f) {
        std::string s;
        (*this) >> s;
        std::from_chars(s.c_str(), s.c_str() + s.length(), f);
        return *this;
    }
    FastIstream& operator>>(double& f) {
        std::string s;
        (*this) >> s;
        std::from_chars(s.c_str(), s.c_str() + s.length(), f);
        return *this;
    }
    FastIstream& operator>>(long double& f) {
        std::string s;
        (*this) >> s;
        std::from_chars(s.c_str(), s.c_str() + s.length(), f);
        return *this;
    }
    template<std::ranges::range T> friend FastIstream& operator>>(FastIstream& is, T& x) {
        for (auto& v : x) is >> v;
        return is;
    }
    char getc() {
        reload(1);
        return *(cur++);
    }
    void seek(int n) {
        reload(n);
        cur += n;
    }
} fin;
class FastOstream {
    constexpr static int buffersize = 1 << 18;
    char buffer[buffersize];
    char* cur = buffer;
    inline void reload(ptrdiff_t w) {
        if (buffer + buffersize - w < cur) [[unlikely]] {
            [[maybe_unused]] int r = write(1, buffer, cur - buffer);
            cur = buffer;
        }
    }
    constexpr static std::array<unsigned, 10000> strtable = []() {
        std::array<unsigned, 10000> res;
        for (unsigned i = 0; i < 10000; ++i) {
            unsigned tmp[4];
            unsigned n = i;
            tmp[3] = (n % 10 + '0') << 24, n /= 10;
            tmp[2] = (n % 10 + '0') << 16, n /= 10;
            tmp[1] = (n % 10 + '0') << 8, n /= 10;
            tmp[0] = n % 10 + '0';
            res[i] = tmp[0] + tmp[1] + tmp[2] + tmp[3];
        }
        return res;
    }();
    constexpr static std::array<unsigned, 10000> strtable2 = []() {
        std::array<unsigned, 10000> res;
        for (unsigned i = 0; i < 10000; ++i) {
            unsigned tmp[4];
            unsigned n = i;
            if (i < 10) n *= 1000;
            else if (i < 100) n *= 100;
            else if (i < 1000) n *= 10;
            tmp[3] = (n % 10 + '0') << 24, n /= 10;
            tmp[2] = (n % 10 + '0') << 16, n /= 10;
            tmp[1] = (n % 10 + '0') << 8, n /= 10;
            tmp[0] = n % 10 + '0';
            res[i] = tmp[0] + tmp[1] + tmp[2] + tmp[3];
        }
        return res;
    }();
public:
    FastOstream() {}
    ~FastOstream() { reload(buffersize); }
    FastOstream& flush() {
        reload(buffersize);
        return *this;
    }
    char widen(char c) const { return c; }
    FastOstream& put(char c) {
        reload(1);
        *(cur++) = c;
        return *this;
    }
    FastOstream& operator<<(unsigned short n) {
        reload(5);
        if (n >= 10000) {
            *(cur++) = '0' + n / 10000, n %= 10000;
            *reinterpret_cast<unsigned*>(cur) = strtable[n];
            cur += 4;
        } else if (n >= 1000) {
            *reinterpret_cast<unsigned*>(cur) = strtable[n];
            cur += 4;
        } else if (n >= 100) {
            *reinterpret_cast<unsigned*>(cur) = strtable[n * 10];
            cur += 3;
        } else if (n >= 10) {
            *(cur++) = '0' + n / 10;
            *(cur++) = '0' + n % 10;
        } else *(cur++) = '0' + n;
        return *this;
    }
    FastOstream& operator<<(short n) {
        reload(6);
        if (n < 0) *(cur++) = '-', n = -n;
        if (n >= 10000) {
            *(cur++) = '0' + n / 10000, n %= 10000;
            *reinterpret_cast<unsigned*>(cur) = strtable[n];
            cur += 4;
        } else if (n >= 1000) {
            *reinterpret_cast<unsigned*>(cur) = strtable[n];
            cur += 4;
        } else if (n >= 100) {
            *reinterpret_cast<unsigned*>(cur) = strtable[n * 10];
            cur += 3;
        } else if (n >= 10) {
            *reinterpret_cast<unsigned*>(cur) = strtable[n * 100];
            cur += 2;
        } else *(cur++) = '0' + n;
        return *this;
    }
    FastOstream& operator<<(unsigned n) {
        reload(10);
        unsigned long long buf = 0;
        char d = 0;
        if (n >= 100000000) {
            d = 8;
            buf = static_cast<unsigned long long>(strtable[n % 10000]) << 32 | strtable[(n / 10000) % 10000];
            n /= 100000000;
        } else if (n >= 10000) {
            d = 4;
            buf = strtable[n % 10000];
            n /= 10000;
        }
        *reinterpret_cast<unsigned*>(cur) = strtable2[n];
        cur += (n >= 10) + (n >= 100) + (n >= 1000) + 1;
        *reinterpret_cast<unsigned long long*>(cur) = buf;
        cur += d;
        return *this;
    }
    FastOstream& operator<<(int n) {
        reload(11);
        if (n < 0) *(cur++) = '-', n = -n;
        unsigned long long buf = 0;
        char d = 0;
        if (n >= 100000000) {
            d = 8;
            buf = static_cast<unsigned long long>(strtable[n % 10000]) << 32 | strtable[(n / 10000) % 10000];
            n /= 100000000;
        } else if (n >= 10000) {
            d = 4;
            buf = strtable[n % 10000];
            n /= 10000;
        }
        *reinterpret_cast<unsigned*>(cur) = strtable2[n];
        cur += (n >= 10) + (n >= 100) + (n >= 1000) + 1;
        *reinterpret_cast<unsigned long long*>(cur) = buf;
        cur += d;
        return *this;
    }
    FastOstream& operator<<(unsigned long long n) {
        reload(20);
        static unsigned buf[4];
        int d = 0;
        if (n >= 10000000000000000) {
            d = 16;
            buf[3] = strtable[n % 10000], n /= 10000;
            buf[2] = strtable[n % 10000], n /= 10000;
            buf[1] = strtable[n % 10000], n /= 10000;
            buf[0] = strtable[n % 10000], n /= 10000;
        } else if (n >= 1000000000000) {
            d = 12;
            buf[2] = strtable[n % 10000], n /= 10000;
            buf[1] = strtable[n % 10000], n /= 10000;
            buf[0] = strtable[n % 10000], n /= 10000;
        } else if (n >= 100000000) {
            d = 8;
            buf[1] = strtable[n % 10000], n /= 10000;
            buf[0] = strtable[n % 10000], n /= 10000;
        } else if (n >= 10000) {
            d = 4;
            buf[0] = strtable[n % 10000], n /= 10000;
        }
        *(unsigned*) cur = strtable2[n];
        cur += (n >= 10) + (n >= 100) + (n >= 1000) + 1;
        std::memcpy(cur, buf, d);
        cur += d;
        return *this;
    }
    FastOstream& operator<<(long long n) {
        reload(21);
        if (n < 0) *(cur++) = '-', n = -n;
        static unsigned buf[4];
        char d = 0;
        if (n >= 10000000000000000) {
            d = 16;
            buf[3] = strtable[n % 10000], n /= 10000;
            buf[2] = strtable[n % 10000], n /= 10000;
            buf[1] = strtable[n % 10000], n /= 10000;
            buf[0] = strtable[n % 10000], n /= 10000;
        } else if (n >= 1000000000000) {
            d = 12;
            buf[2] = strtable[n % 10000], n /= 10000;
            buf[1] = strtable[n % 10000], n /= 10000;
            buf[0] = strtable[n % 10000], n /= 10000;
        } else if (n >= 100000000) {
            d = 8;
            buf[1] = strtable[n % 10000], n /= 10000;
            buf[0] = strtable[n % 10000], n /= 10000;
        } else if (n >= 10000) {
            d = 4;
            buf[0] = strtable[n % 10000], n /= 10000;
        }
        *(unsigned*) cur = strtable2[n];
        cur += (n >= 10) + (n >= 100) + (n >= 1000) + 1;
        std::memcpy(cur, buf, d);
        cur += d;
        return *this;
    }
    FastOstream& operator<<(long n) { return operator<<(static_cast<long long>(n)); }
    FastOstream& operator<<(unsigned long n) { return operator<<(static_cast<unsigned long long>(n)); }
    FastOstream& operator<<(std::nullptr_t) {
        reload(7);
        std::memcpy(cur, "nullptr", 7);
        cur += 7;
        return *this;
    }
    friend FastOstream& operator<<(FastOstream& os, char c) {
        os.reload(1);
        *(os.cur++) = c;
        return os;
    }
    friend FastOstream& operator<<(FastOstream& os, signed char c) {
        os.reload(1);
        *(os.cur++) = c;
        return os;
    }
    friend FastOstream& operator<<(FastOstream& os, unsigned char c) {
        os.reload(1);
        *(os.cur++) = c;
        return os;
    }
    friend FastOstream& operator<<(FastOstream& os, const char* s) {
        size_t n = std::strlen(s);
        if (n >= os.buffersize) {
            os.reload(buffersize);
            write(1, s, n);
        } else {
            os.reload(n);
            std::memcpy(os.cur, s, n);
            os.cur += n;
        }
        return os;
    }
    friend FastOstream& operator<<(FastOstream& os, const std::string& s) {
        size_t n = s.length();
        if (n >= os.buffersize) {
            os.reload(buffersize);
            write(1, s.data(), n);
        } else {
            os.reload(n);
            std::memcpy(os.cur, s.data(), n);
            os.cur += n;
        }
        return os;
    }
    friend FastOstream& operator<<(FastOstream& os, std::string_view s) {
        size_t n = s.length();
        if (n >= os.buffersize) {
            os.reload(buffersize);
            write(1, s.data(), n);
        } else {
            os.reload(n);
            std::memcpy(os.cur, s.data(), n);
            os.cur += n;
        }
        return os;
    }
    template<std::ranges::range T> friend FastOstream& operator<<(FastOstream& os, const T& v) {
        size_t n = std::distance(std::ranges::begin(v), std::ranges::end(v)), cnt = 0;
        for (const auto& x : v) {
            os << x;
            if (++cnt != n) os << ' ';
        }
        return os;
    }
} fout;
/*
template<size_t Q> class YesNoBuf {
    char buf[4 * Q];
    char* cur = buf;
public:
    inline ~YesNoBuf() noexcept { [[maybe_unused]] auto n = write(1, buf, cur - buf); }
    inline void operator()(bool b) {
        int n = (b ? 4 : 3);
        std::memcpy(cur, (b ? "Yes\n" : "No\n"), n);
        cur += n;
    }
};
YesNoBuf<100000> out;
*/

int main() {
    gsh::Rand32 engine;
    gsh::ClockTimer t;
    u32 r = 0;
    for (u32 i = 0; i != 10000000; ++i) {
        r += std::gcd(engine() / 2, engine() / 2);
    }
    t.print();
    std::cerr << r << std::endl;
    /*
    for (u32 i = 0; i != 16; ++i) {
        std::cerr << '{';
        for (u32 j = 0; j != 16; ++j) {
            std::cerr << gsh::GCD(i, j) << ",}"[j == 15];
        }
        std::cerr << ",\n";
    }*/
}
