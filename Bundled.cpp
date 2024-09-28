#if !defined(__clang__) && defined(__GNUC__)
#pragma GCC optimize("O3")
#pragma GCC optimize("unroll-loops")
#pragma GCC target("avx2")
#endif
#ifdef EVAL
#define ONLINE_JUDGE
#endif
#ifdef ONLINE_JUDGE
#define NDEBUG
#endif
#include <cstdlib>  
#include <cstring>  
#include <utility>  
#include <tuple>    
#if __has_include(<unistd.h>)
#include <unistd.h>  
#endif
#ifndef _WIN32
#include <sys/mman.h>  
#include <sys/stat.h>  
#endif
#include "TypeDef.hpp"     
#include "Parser.hpp"      
#include "Formatter.hpp"   
#include "Functional.hpp"  

namespace gsh {

namespace internal {
    template<class D> class IstreamInterface {
        constexpr D& derived() { return *static_cast<D*>(this); }
        constexpr const D& derived() const { return *static_cast<const D*>(this); }
    public:
        constexpr auto read() { return std::tuple{}; }
        template<class T, class... Types> constexpr auto read() {
            if constexpr (sizeof...(Types) == 0) return Parser<T>{}(derived());
            else if constexpr (sizeof...(Types) == 1) {
                auto res = Parser<T>{}(derived());
                return std::tuple_cat(std::tuple(std::move(res)), std::tuple(read<Types...>()));
            } else {
                auto res = Parser<T>{}(derived());
                return std::tuple_cat(std::tuple(std::move(res)), read<Types...>());
            }
        }
    };
    template<class D> class OstreamInterface {
        constexpr D& derived() { return *static_cast<D*>(this); }
        constexpr const D& derived() const { return *static_cast<const D*>(this); }
    public:
        template<class Sep> constexpr void write_sep(Sep&&) {}
        template<class Sep, class T, class... Args> constexpr void write_sep(Sep&& sep, T&& x, Args&&... args) {
            Formatter<std::decay_t<T>>{}(derived(), std::forward<T>(x));
            if constexpr (sizeof...(Args) != 0) {
                Formatter<std::decay_t<Sep>>{}(derived(), sep);
                write_sep(std::forward<Sep>(sep), std::forward<Args>(args)...);
            }
        }
        template<class... Args> constexpr void write(Args&&... args) { write_sep(' ', std::forward<Args>(args)...); }
        template<class Sep, class... Args> constexpr void writeln_sep(Sep&& sep, Args&&... args) {
            write_sep(std::forward<Sep>(sep), std::forward<Args>(args)...);
            Formatter<ctype::c8>{}(derived(), '\n');
        }
        template<class... Args> constexpr void writeln(Args&&... args) {
            write_sep(' ', std::forward<Args>(args)...);
            Formatter<ctype::c8>{}(derived(), '\n');
        }
    };
}  

template<itype::u32 Bufsize = (1 << 17)> class BasicReader : public internal::IstreamInterface<BasicReader<Bufsize>> {
    itype::i32 fd = 0;
    ctype::c8 buf[Bufsize + 1] = {};
    ctype::c8 *cur = buf, *eof = buf;
public:
    BasicReader() {}
    BasicReader(itype::i32 filehandle) : fd(filehandle) {}
    BasicReader(const BasicReader& rhs) {
        fd = rhs.fd;
        std::memcpy(buf, rhs.buf, rhs.eof - rhs.cur);
        cur = buf + (rhs.cur - rhs.buf);
        eof = buf + (rhs.cur - rhs.eof);
    }
    BasicReader& operator=(const BasicReader& rhs) {
        fd = rhs.fd;
        std::memcpy(buf, rhs.buf, rhs.eof - rhs.cur);
        cur = buf + (rhs.cur - rhs.buf);
        eof = buf + (rhs.cur - rhs.eof);
        return *this;
    }
    void reload() {
        if (eof == buf + Bufsize || eof == cur || [&] {
                auto p = cur;
                while (*p >= '!') ++p;
                return p;
            }() == eof) [[likely]] {
            itype::u32 rem = eof - cur;
            std::memmove(buf, cur, rem);
            *(eof = buf + rem + read(fd, buf + rem, Bufsize - rem)) = '\0';
            cur = buf;
        }
    }
    void reload(itype::u32 len) {
        if (avail() < len) [[unlikely]]
            reload();
    }
    itype::u32 avail() const { return eof - cur; }
    const ctype::c8* current() const { return cur; }
    void skip(itype::u32 n) { cur += n; }
};
class MmapReader : public internal::IstreamInterface<MmapReader> {
    const itype::i32 fh;
    ctype::c8* buf;
    ctype::c8 *cur, *eof;
public:
    MmapReader() : fh(0) {
#ifdef _WIN32
        write(1, "gsh::MmapReader / gsh::MmapReader is not available for Windows.\n", 64);
        std::exit(1);
#else
        struct stat st;
        fstat(0, &st);
        buf = reinterpret_cast<ctype::c8*>(mmap(nullptr, st.st_size + 64, PROT_READ, MAP_PRIVATE, 0, 0));
        cur = buf;
        eof = buf + st.st_size;
#endif
    }
    void reload() const {}
    void reload(itype::u32) const {}
    itype::u32 avail() const { return eof - cur; }
    const ctype::c8* current() const { return cur; }
    void skip(itype::u32 n) { cur += n; }
};
class StaticStrReader : public internal::IstreamInterface<StaticStrReader> {
    const ctype::c8* cur;
public:
    constexpr StaticStrReader() {}
    constexpr StaticStrReader(const ctype::c8* c) : cur(c) {}
    constexpr void reload() const {}
    constexpr void reload(itype::u32) const {}
    constexpr itype::u32 avail() const { return static_cast<itype::u32>(-1); }
    constexpr const ctype::c8* current() { return cur; }
    constexpr void skip(itype::u32 n) { cur += n; }
};

template<itype::u32 Bufsize = (1 << 17)> class BasicWriter : public internal::OstreamInterface<BasicWriter<Bufsize>> {
    itype::i32 fd = 1;
    ctype::c8 buf[Bufsize + 1] = {};
    ctype::c8 *cur = buf, *eof = buf + Bufsize;
public:
    BasicWriter() {}
    BasicWriter(itype::i32 filehandle) : fd(filehandle) {}
    BasicWriter(const BasicWriter& rhs) {
        fd = rhs.fd;
        std::memcpy(buf, rhs.buf, rhs.cur - rhs.buf);
        cur = buf + (rhs.cur - rhs.buf);
    }
    BasicWriter& operator=(const BasicWriter& rhs) {
        fd = rhs.fd;
        std::memcpy(buf, rhs.buf, rhs.cur - rhs.buf);
        cur = buf + (rhs.cur - rhs.buf);
        return *this;
    }
    void reload() {
        [[maybe_unused]] itype::i32 tmp = write(fd, buf, cur - buf);
        cur = buf;
    }
    void reload(itype::u32 len) {
        if (eof - cur < len) [[unlikely]]
            reload();
    }
    itype::u32 avail() const { return eof - cur; }
    ctype::c8* current() { return cur; }
    void skip(itype::u32 n) { cur += n; }
};
class StaticStrWriter : public internal::OstreamInterface<StaticStrWriter> {
    ctype::c8* cur;
public:
    constexpr StaticStrWriter() {}
    constexpr StaticStrWriter(ctype::c8* c) : cur(c) {}
    constexpr void reload() const {}
    constexpr void reload(itype::u32) const {}
    constexpr itype::u32 avail() const { return static_cast<itype::u32>(-1); }
    constexpr ctype::c8* current() { return cur; }
    constexpr void skip(itype::u32 n) { cur += n; }
};

}  

#include "TypeDef.hpp"

namespace gsh {

class Exception {
    char str[512];
    char* cur = str;
    void write(const char* x) {
        for (int i = 0; i != 512; ++i, ++cur) {
            if (x[i] == '\0') break;
            *cur = x[i];
        }
    }
    void write(long long x) {
        if (x == 0) *(cur++) = '0';
        else {
            if (x < 0) {
                *(cur++) = '-';
                x = -x;
            }
            char buf[20];
            int i = 0;
            while (x != 0) buf[i++] = x % 10 + '0', x /= 10;
            while (i--) *(cur++) = buf[i];
        }
    }
    template<class T, class... Args> void generate_message(T x, Args... args) {
        write(x);
        if constexpr (sizeof...(Args) > 0) generate_message(args...);
    }
public:
    Exception() noexcept { *cur = '\0'; }
    Exception(const Exception& x) noexcept {
        for (int i = 0; i != 512; ++i) str[i] = x.str[i];
        cur = x.cur;
    }
    explicit Exception(const char* what_arg) noexcept {
        for (int i = 0; i != 512; ++i, ++cur) {
            *cur = what_arg[i];
            if (what_arg[i] == '\0') break;
        }
    }
    template<class... Args> explicit Exception(Args... args) noexcept {
        generate_message(args...);
        *cur = '\0';
    }
    Exception& operator=(const Exception& x) noexcept {
        for (int i = 0; i != 512; ++i) str[i] = x.str[i];
        cur = x.cur;
        return *this;
    }
    const char* what() const noexcept { return str; }
};

}  

#include <type_traits>     
#include <cstring>         
#include <cstdlib>         
#include <algorithm>       
#include <cmath>           
#include <limits>          
#include "TypeDef.hpp"     
#include "Arr.hpp"         
#include "Vec.hpp"         
#include "Range.hpp"       
#include "Functional.hpp"  
#include <immintrin.h>

namespace gsh {

template<class T, class U> constexpr std::common_type_t<T, U> Min(const T& a, const U& b) {
    return a < b ? a : b;
}
template<class T, class... Args> constexpr auto Min(const T& x, const Args&... args) {
    return Min(x, Min(args...));
}
template<class T, class U> constexpr std::common_type_t<T, U> Max(const T& a, const U& b) {
    return a < b ? b : a;
}
template<class T, class... Args> constexpr auto Max(const T& x, const Args&... args) {
    return Max(x, Max(args...));
}
template<class T, class U> constexpr bool Chmin(T& a, const U& b) {
    const bool f = b < a;
    a = f ? b : a;
    return f;
}
template<class T, class... Args> constexpr bool Chmin(T& a, const Args&... b) {
    return Chmin(a, Min(b...));
}
template<class T, class U> constexpr bool Chmax(T& a, const U& b) {
    const bool f = a < b;
    a = f ? b : a;
    return f;
}
template<class T, class... Args> constexpr bool Chmax(T& a, const Args&... b) {
    return Chmax(a, Max(b...));
}

template<ForwardRange R, class Proj = Identity, std::indirect_strict_weak_order<std::projected<typename RangeTraits<R>::iterator, Proj>> Comp = Less>
    requires std::indirectly_copyable_storable<typename RangeTraits<R>::iterator, typename RangeTraits<R>::value_type*>
constexpr typename RangeTraits<R>::value_type MinValue(R&& r, Comp&& comp = {}, Proj&& proj = {}) {
    using traits = RangeTraits<R>;
    auto first = traits::begin(r);
    auto last = traits::end(r);
    if (first == last) throw Exception("gsh::Min / The input is empty.");
    auto res = *(first++);
    for (; first != last; ++first) {
        const bool f = Invoke(comp, Invoke(proj, static_cast<const decltype(res)&>(*first)), Invoke(proj, res));
        if constexpr (std::is_trivial_v<decltype(res)>) res = f ? *first : res;
        else if (f) res = *first;
    }
    return res;
}
template<ForwardRange R, class Proj = Identity, std::indirect_strict_weak_order<std::projected<typename RangeTraits<R>::iterator, Proj>> Comp = Less>
    requires std::indirectly_copyable_storable<typename RangeTraits<R>::iterator, typename RangeTraits<R>::value_type*>
constexpr typename RangeTraits<R>::value_type MaxValue(R&& r, Comp&& comp = {}, Proj&& proj = {}) {
    if constexpr (std::same_as<std::remove_cvref_t<Comp>, Less>) return MinValue(std::forward<R>(r), Greater(), proj);
    else if constexpr (std::same_as<std::remove_cvref_t<Comp>, Greater>) return MinValue(std::forward<R>(r), Less(), proj);
    else return MinValue(std::forward<R>(r), SwapArgs(std::forward<Comp>(comp)), proj);
}

namespace internal {
    template<class F, class T, class I, class U> concept IndirectlyBinaryLeftFoldableImpl = std::movable<T> && std::movable<U> && std::convertible_to<T, U> && std::invocable<F&, U, std::iter_reference_t<I>> && std::assignable_from<U&, std::invoke_result_t<F&, U, std::iter_reference_t<I>>>;
    template<class F, class T, class I> concept IndirectlyBinaryLeftFoldable = std::copy_constructible<F> && std::indirectly_readable<I> && std::invocable<F&, T, std::iter_reference_t<I>> && std::convertible_to<std::invoke_result_t<F&, T, std::iter_reference_t<I>>, std::decay_t<std::invoke_result_t<F&, T, std::iter_reference_t<I>>>> && IndirectlyBinaryLeftFoldableImpl<F, T, I, std::decay_t<std::invoke_result_t<F&, T, std::iter_reference_t<I>>>>;
}  
template<ForwardRange R, class T = typename RangeTraits<R>::value_type, internal::IndirectlyBinaryLeftFoldable<T, typename RangeTraits<R>::iterator> F = Plus> constexpr auto Fold(R&& r, T init = {}, F&& f = {}) {
    for (auto&& x : std::forward<R>(r)) init = Invoke(f, std::move(init), std::forward<decltype(x)>(x));
    return init;
}

template<BidirectionalRange R>
    requires std::permutable<typename RangeTraits<R>::iterator>
void Reverse(R&& r) {
    using traits = RangeTraits<R>;
    auto first = traits::begin(r);
    auto last = traits::end(r);
}

namespace internal {
    template<class T, class Proj = Identity> void SortUnsigned32(T* const p, const itype::u32 n, Proj&& proj = {}) {
        static itype::u32 cnt1[1 << 16], cnt2[1 << 16];
        std::memset(cnt1, 0, sizeof(cnt1));
        std::memset(cnt2, 0, sizeof(cnt2));
        for (itype::u32 i = 0; i != n; ++i) {
            auto tmp = Invoke(proj, p[i]);
            const itype::u16 a = tmp & 0xffff;
            const itype::u16 b = tmp >> 16 & 0xffff;
            ++cnt1[a];
            ++cnt2[b];
        }
        for (itype::u32 i = 0; i != (1 << 16) - 1; ++i) {
            cnt1[i + 1] += cnt1[i];
            cnt2[i + 1] += cnt2[i];
        }
        T* const tmp = reinterpret_cast<T*>(std::malloc(sizeof(T) * n));
        for (itype::u32 i = n; i != 0;) --i, tmp[--cnt1[Invoke(proj, p[i]) & 0xffff]] = std::move(p[i]);
        for (itype::u32 i = n; i != 0;) --i, p[--cnt2[Invoke(proj, tmp[i]) >> 16 & 0xffff]] = std::move(tmp[i]);
        std::free(tmp);
    }
    template<class T, class Proj = Identity> void SortUnsigned64(T* const p, const itype::u32 n, Proj&& proj = {}) {
        static itype::u32 cnt1[1 << 16], cnt2[1 << 16], cnt3[1 << 16], cnt4[1 << 16];
        std::memset(cnt1, 0, sizeof(cnt1));
        std::memset(cnt2, 0, sizeof(cnt2));
        std::memset(cnt3, 0, sizeof(cnt3));
        std::memset(cnt4, 0, sizeof(cnt4));
        for (itype::u32 i = 0; i != n; ++i) {
            auto tmp = Invoke(proj, p[i]);
            const itype::u16 a = tmp & 0xffff;
            const itype::u16 b = tmp >> 16 & 0xffff;
            const itype::u16 c = tmp >> 32 & 0xffff;
            const itype::u16 d = tmp >> 48 & 0xffff;
            ++cnt1[a];
            ++cnt2[b];
            ++cnt3[c];
            ++cnt4[d];
        }
        for (itype::u32 i = 0; i != (1 << 16) - 1; ++i) {
            cnt1[i + 1] += cnt1[i];
            cnt2[i + 1] += cnt2[i];
            cnt3[i + 1] += cnt3[i];
            cnt4[i + 1] += cnt4[i];
        }
        T* const tmp = reinterpret_cast<T*>(std::malloc(sizeof(T) * n));
        for (itype::u32 i = n; i != 0;) --i, tmp[--cnt1[Invoke(proj, p[i]) & 0xffff]] = std::move(p[i]);
        for (itype::u32 i = n; i != 0;) --i, p[--cnt2[Invoke(proj, tmp[i]) >> 16 & 0xffff]] = std::move(tmp[i]);
        for (itype::u32 i = n; i != 0;) --i, tmp[--cnt3[Invoke(proj, p[i]) >> 32 & 0xffff]] = std::move(p[i]);
        for (itype::u32 i = n; i != 0;) --i, p[--cnt4[Invoke(proj, tmp[i]) >> 48 & 0xffff]] = std::move(tmp[i]);
        std::free(tmp);
    }
}  


template<ForwardRange R, class T, class Proj = Identity, std::indirect_strict_weak_order<const T*, std::projected<typename RangeTraits<R>::iterator, Proj>> Comp = Less> constexpr auto LowerBound(R&& r, const T& value, Comp&& comp = {}, Proj&& proj = {}) {
    using traits = RangeTraits<R>;
    auto st = traits::begin(r);
    for (auto len = traits::size(r) + 1; len > 1;) {
        auto half = len / 2;
        len -= half;
        auto tmp = std::next(st, half);
        auto md = std::next(tmp, -1);
        st = Invoke(comp, Invoke(proj, *md), value) ? tmp : st;
    }
    return st;
}
template<ForwardRange R, class T, class Proj = Identity, std::indirect_strict_weak_order<const T*, std::projected<typename RangeTraits<R>::iterator, Proj>> Comp = Less> constexpr auto UpperBound(R&& r, const T& value, Comp&& comp = {}, Proj&& proj = {}) {
    using traits = RangeTraits<R>;
    auto st = traits::begin(r);
    for (auto len = traits::size(r) + 1; len > 1;) {
        auto half = len / 2;
        len -= half;
        auto tmp = std::next(st, half);
        auto md = std::next(tmp, -1);
        st = !static_cast<bool>(Invoke(comp, value, Invoke(proj, *md))) ? tmp : st;
    }
    return st;
}

template<ForwardRange R, class Proj = Identity, class Comp = Less> constexpr Arr<itype::u32> LongestIncreasingSubsequence(R&& r, Comp&& comp = {}, Proj&& proj = {}) {
    using T = typename RangeTraits<R>::value_type;
    Arr<T> dp(RangeTraits<R>::size(r));
    Arr<itype::u32> idx(dp.size());
    itype::u32 i = 0;
    T *begin = dp.data(), *last = dp.data();
    for (const T& x : r) {
        T* loc = LowerBound(Subrange{ begin, last }, x, comp, proj);
        idx[i++] = loc - begin;
        last += (loc == last);
        *loc = x;
    }
    itype::u32 cnt = last - begin - 1;
    Arr<itype::u32> res(last - begin);
    for (itype::u32 i = dp.size(); i != 0;)
        if (idx[--i] == cnt) res[cnt--] = i;
    return res;
}

template<RandomAccessRange R> constexpr Arr<itype::u32> LongestCommonPrefix(R&& r) {
    using traits = RangeTraits<R>;
    const itype::u32 n = traits::size(r);
    Arr<itype::u32> res(n);
    if (n == 0) return res;
    res[0] = n;
    const auto itr = traits::begin(r);
    itype::u32 i = 1, j = 0;
    while (i != n) {
        while (i + j < n && *std::next(itr, j) == *std::next(itr, i + j)) ++j;
        res[i] = j;
        if (j == 0) {
            ++i;
            continue;
        }
        itype::u32 k = 1;
        while (k < j && k + res[k] < j) ++k;
        const auto a = res.data() + i + 1;
        const auto b = res.data() + 1;
        for (itype::u32 l = 0; l != k - 1; ++l) a[l] = b[l];
        i += k;
        j -= k;
    }
    return res;
}

template<class T = itype::u64, Rangeof<itype::u32> R> constexpr T CountDistinctSubsequences(R&& r) {
    using traits = RangeTraits<R>;
    const itype::u32 n = traits::size(r);
    if (n == 0) return 0;
    Arr<itype::u64> s(n);
    for (itype::u32 i = 0; itype::u32 x : r) {
        s[i] = static_cast<itype::u64>(i) << 32 | x;
        ++i;
    }
    internal::SortUnsigned32(s.data(), n);
    Arr<itype::u32> rank(n);
    rank[s[0] >> 32] = 0;
    itype::u32 cnt = 0, end = s[0];
    for (itype::u32 i = 1; i != n; ++i) {
        cnt += end != static_cast<itype::u32>(s[i]);
        rank[s[i] >> 32] = cnt;
        end = s[i];
    }
    Arr<itype::u32> last(cnt + 1, n);
    Arr<T> dp(n + 1);
    dp[0] = 1;
    const T m = 2;
    for (itype::u32 i = 0; i != n; ++i) {
        dp[i + 1] = m * dp[i] - dp[last[rank[i]]];
        last[rank[i]] = i;
    }
    return dp[n] - dp[0];
}

template<Range R> constexpr auto Majority(R&& r) {
    using traits = RangeTraits<R>;
    itype::u32 c = 0;
    itype::u32 len = 0;
    auto i = traits::begin(r);
    auto j = traits::end(r), k = j;
    for (; i != j; ++i) {
        ++len;
        if (c == 0) k = i, c = 1;
        else c += static_cast<itype::u32>(static_cast<bool>(*i == *k)) * 2 - 1;
    }
    c = 0;
    for (i = traits::begin(r); i != j; ++i) c += static_cast<bool>(*i == *k);
    return (2 * c >= len ? k : j);
}

class Mo {
    struct rg {
        itype::u32 l, r;
        constexpr rg(itype::u32 a, itype::u32 b) : l(a), r(b) {}
    };
    Vec<rg> qu;
public:
    constexpr Mo() {}
    constexpr void reserve(itype::u32 q) { qu.reserve(q); }
    constexpr void query(itype::u32 l, itype::u32 r) { qu.emplace_back(l, r); }
    template<class F1, class F2, class F3> void run(F1&& add, F2&& del, F3&& slv) const { solve(add, add, del, del, std::forward<F3>(slv)); }
    template<class F1, class F2, class F3, class F4, class F5> void run(F1 addl, F2 addr, F3 dell, F4 delr, F5 slv) const {
        const itype::u32 Q = qu.size();
        itype::u32 N = 0;
        for (itype::u32 i = 0; i != Q; ++i) N = N < qu[i].r ? qu[i].r : N;
        itype::u32 width = 1.1 * std::sqrt(static_cast<ftype::f64>(3ull * N * N) / (2 * Q));
        width += width == 0;
        Arr<itype::u32> cnt(N + 1), buf(Q), block(Q), idx(Q);
        for (itype::u32 i = 0; i != Q; ++i) ++cnt[qu[i].r];
        for (itype::u32 i = 0; i != N; ++i) cnt[i + 1] += cnt[i];
        for (itype::u32 i = 0; i != Q; ++i) buf[--cnt[qu[i].r]] = i;
        cnt.assign(N / width + 2, 0);
        for (itype::u32 i = 0; i != Q; ++i) block[i] = qu[i].l / width;
        for (itype::u32 i = 0; i != Q; ++i) ++cnt[block[i]];
        for (itype::u32 i = 0; i != cnt.size() - 1; ++i) cnt[i + 1] += cnt[i];
        for (itype::u32 i = 0; i != Q; ++i) idx[--cnt[block[buf[i]]]] = buf[i];
        for (itype::u32 i = 0; i < cnt.size() - 1; i += 2) {
            const itype::u32 l = cnt[i], r = cnt[i + 1];
            for (itype::u32 j = 0; j != (r - l) / 2; ++j) {
                const itype::u32 t = idx[l + j];
                idx[l + j] = idx[r - j - 1], idx[r - j - 1] = t;
            }
        }
        itype::u32 nl = 0, nr = 0;
        for (itype::u32 i : idx) {
            while (nl > qu[i].l) Invoke(addl, --nl);
            while (nr < qu[i].r) Invoke(addr, nr++);
            while (nl < qu[i].l) Invoke(dell, nl++);
            while (nr > qu[i].r) Invoke(delr, --nr);
            Invoke(slv, i);
        }
    }
};

}  


#if 0 && !defined ONLINE_JUDGE
#include <fcntl.h>
gsh::BasicReader r(open("in.txt", O_RDONLY));
gsh::BasicWriter w(open("out.txt", O_WRONLY | O_TRUNC));
#else
#if defined(ONLINE_JUDGE)
gsh::MmapReader rd;
#else
gsh::BasicReader rd;
#endif
gsh::BasicWriter wt;
#endif
void Main() {
    using namespace std;
    using namespace gsh;
    using namespace gsh::itype;
    using namespace gsh::ftype;
    using namespace gsh::ctype;
    u32 N = rd.read<u4dig>();
    u32 K = rd.read<u4dig>();
    static u32 dist[100][100];
    for (u32 i = 0; i != 100; ++i) {
        for (u32 j = 0; j != 100; ++j) {
            dist[i][j] = 1000000000;
        }
        dist[i][i] = 0;
    }
    for (u32 i = 0; i != K; ++i) {
        if (rd.read<c8>() == '1') {
            u32 a = rd.read<u4dig>() - 1;
            u32 b = rd.read<u4dig>() - 1;
            u32 c = rd.read<u8dig>();
            if (Chmin(dist[a][b], c)) {
                for (u32 j = 0; j != 100; ++j) {
                    for (u32 k = 0; k != 100; ++k) {
                        Chmin(dist[j][k], Min(dist[j][a] + dist[b][k], dist[j][b] + dist[a][k]) + c);
                    }
                }
            }
        } else {
            u32 a = rd.read<u4dig>() - 1;
            u32 b = rd.read<u4dig>() - 1;
            if (dist[a][b] == 1000000000) Formatter<const c8*>{ 3 }(wt, "-1\n");
            else wt.writeln(dist[a][b]);
        }
    }
    
}
int main() {
#ifdef ONLINE_JUDGE
    Main();
    wt.reload();
#else
    try {
        Main();
        wt.reload();
    } catch (gsh::Exception& e) {
        wt.writeln("gsh::Exception was throwed:", e.what());
        wt.reload();
        return 1;
    }
#endif
}

