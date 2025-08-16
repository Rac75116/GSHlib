#pragma once
#include <type_traits>     // std::common_type
#include <cstring>         // std::memset
#include <cstdlib>         // std::malloc, std::free
#include <algorithm>       // std::lower_bound
#include <cmath>           // std::sqrt
#include <limits>          // std::numeric_limits
#include "TypeDef.hpp"     // gsh::itype
#include "Arr.hpp"         // gsh::Arr
#include "Vec.hpp"         // gsh::Vec
#include "Range.hpp"       // gsh::Range
#include "Functional.hpp"  // gsh::Less, gsh::Greater

namespace gsh {

template<class T, class U> constexpr std::common_type_t<T, U> Min(const T& a, const U& b) {
    return a < b ? a : b;
}
template<class T, class... Args>
    requires(sizeof...(Args) >= 2)
constexpr auto Min(const T& x, const Args&... args) {
    return Min(x, Min(args...));
}
template<class T, class U> constexpr std::common_type_t<T, U> Max(const T& a, const U& b) {
    return a < b ? b : a;
}
template<class T, class... Args>
    requires(sizeof...(Args) >= 2)
constexpr auto Max(const T& x, const Args&... args) {
    return Max(x, Max(args...));
}
template<class T, class U> constexpr bool Chmin(T& a, const U& b) {
    const bool f = b < a;
    a = f ? b : a;
    return f;
}
template<class T, class... Args>
    requires(sizeof...(Args) >= 2)
constexpr bool Chmin(T& a, const Args&... b) {
    return Chmin(a, Min(b...));
}
template<class T, class U> constexpr bool Chmax(T& a, const U& b) {
    const bool f = a < b;
    a = f ? b : a;
    return f;
}
template<class T, class... Args>
    requires(sizeof...(Args) >= 2)
constexpr bool Chmax(T& a, const Args&... b) {
    return Chmax(a, Max(b...));
}

namespace internal {

    template<class R, class Comp, class Proj> constexpr auto MinImpl(R&& r, Comp&& comp, Proj&& proj) {
        auto first = std::ranges::begin(r);
        auto last = std::ranges::end(r);
        if (first == last) {
            if constexpr (std::same_as<Proj, Identity>) {
                if constexpr (std::same_as<Comp, Less> && requires {
                                  { std::numeric_limits<std::ranges::range_value_t<R>>::max() } -> std::same_as<std::ranges::template range_value_t<R>>;
                              }) {
                    return std::numeric_limits<std::ranges::range_value_t<R>>::max();
                }
                if constexpr (std::same_as<Comp, Greater> && requires {
                                  { std::numeric_limits<std::ranges::range_value_t<R>>::lowest() } -> std::same_as<std::ranges::template range_value_t<R>>;
                              }) {
                    return std::numeric_limits<std::ranges::range_value_t<R>>::lowest();
                }
            }
            throw Exception("gsh::internal::MinImpl / The input is empty.");
        }
        auto res = *(first++);
        for (; first != last; ++first) {
            const bool f = Invoke(comp, Invoke(proj, static_cast<const decltype(res)&>(*first)), Invoke(proj, res));
            if constexpr (std::is_trivial_v<decltype(res)>) res = f ? *first : res;
            else if (f) res = *first;
        }
        return res;
    }
    template<class R, class Comp, class Proj> constexpr auto MaxImpl(R&& r, Comp&& comp, Proj&& proj) {
        if constexpr (std::same_as<std::remove_cvref_t<Comp>, Less>) return MinImpl(std::forward<R>(r), Greater(), proj);
        else if constexpr (std::same_as<std::remove_cvref_t<Comp>, Greater>) return MinImpl(std::forward<R>(r), Less(), proj);
        else return MinImpl(std::forward<R>(r), SwapArgs(std::forward<Comp>(comp)), proj);
    }

    template<class R, class T, class F> constexpr auto FoldImpl(R&& r, T init, F&& f) {
        for (auto&& x : std::forward<R>(r)) init = Invoke(f, std::move(init), std::forward<decltype(x)>(x));
        return init;
    }
    template<class R, class F> constexpr auto SumImpl(R&& r, F&& f) {
        auto itr = std::ranges::begin(r);
        auto sent = std::ranges::end(r);
        if (!(itr != sent)) {
            if constexpr (std::is_arithmetic_v<std::ranges::range_value_t<R>> && std::is_constructible_v<std::ranges::range_value_t<R>, itype::u32>) {
                return std::ranges::range_value_t<R>(static_cast<itype::u32>(0));
            } else {
                throw Exception("gsh::internal::SumImpl / The input is empty.");
            }
        }
        auto res = *itr;
        for (++itr; itr != sent; ++itr) res = Invoke(f, std::move(res), *itr);
        return res;
    }
    template<class R, class F> constexpr void AdjacentDifferenceImpl(R&& r, F&& f) {
        auto itr = std::ranges::begin(r);
        auto sent = std::ranges::end(r);
        if (!(itr != sent)) return;
        using value_type = std::ranges::range_value_t<R>;
        auto prev = std::move(*itr);
        while (true) {
            auto nx = std::ranges::next(itr);
            if (!(nx != sent)) break;
            value_type tmp = Invoke(f, *nx, std::move(*itr));
            *itr = std::move(prev);
            prev = std::move(tmp);
            itr = nx;
        }
        *itr = prev;
    }

    template<class R> constexpr void ReverseImpl(R&& r) {
        std::ranges::reverse(std::forward<R>(r));
    }

    template<class T, class Proj = Identity> void SortUnsigned8(T* const p, const itype::u32 n, Proj&& proj = {}) {
        std::unique_ptr<itype::u32[]> cnt(new itype::u32[1 << 8]{});
        for (itype::u32 i = 0; i != n; ++i) ++cnt[Invoke(proj, p[i]) & 0xff];
        for (itype::u32 i = 0; i != (1 << 8) - 1; ++i) cnt[i + 1] += cnt[i];
        Arr<T> tmp(ArrNoInit, n);
        for (itype::u32 i = n; i--;) std::construct_at(&tmp[--cnt[Invoke(proj, p[i]) & 0xffff]], std::move(p[i]));
        for (itype::u32 i = 0; i != n; ++i) p[i] = std::move(tmp[i]);
    }
    template<class T, class Proj = Identity> void SortUnsigned16(T* const p, const itype::u32 n, Proj&& proj = {}) {
        std::unique_ptr<itype::u32[]> cnt(new itype::u32[1 << 16]{});
        for (itype::u32 i = 0; i != n; ++i) ++cnt[Invoke(proj, p[i]) & 0xffff];
        for (itype::u32 i = 0; i != (1 << 16) - 1; ++i) cnt[i + 1] += cnt[i];
        Arr<T> tmp(ArrNoInit, n);
        for (itype::u32 i = n; i--;) std::construct_at(&tmp[--cnt[Invoke(proj, p[i]) & 0xffff]], std::move(p[i]));
        for (itype::u32 i = 0; i != n; ++i) p[i] = std::move(tmp[i]);
    }
    template<class T, class Proj = Identity> void SortUnsigned32(T* const p, const itype::u32 n, Proj&& proj = {}) {
        std::unique_ptr<itype::u32[]> cnt(new itype::u32[2 * (1 << 16)]{});
        itype::u32 *const cnt1 = cnt.get(), *const cnt2 = cnt.get() + (1 << 16);
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
        Arr<T> tmp(ArrNoInit, n);
        for (itype::u32 i = n; i--;) std::construct_at(&tmp[--cnt1[Invoke(proj, p[i]) & 0xffff]], std::move(p[i]));
        if (cnt2[0] == n) {
            for (itype::u32 i = 0; i != n; ++i) p[i] = std::move(tmp[i]);
            return;
        }
        for (itype::u32 i = n; i--;) p[--cnt2[Invoke(proj, tmp[i]) >> 16 & 0xffff]] = std::move(tmp[i]);
    }
    template<class T, class Proj = Identity> void SortUnsigned64(T* const p, const itype::u32 n, Proj&& proj = {}) {
        std::unique_ptr<itype::u32[]> cnt(new itype::u32[4 * (1 << 16)]{});
        itype::u32 *const cnt1 = cnt.get(), *const cnt2 = cnt.get() + (1 << 16), *const cnt3 = cnt.get() + 2 * (1 << 16), *const cnt4 = cnt.get() + 3 * (1 << 16);
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
        Arr<T> tmp(ArrNoInit, n);
        for (itype::u32 i = n; i--;) std::construct_at(&tmp[--cnt1[Invoke(proj, p[i]) & 0xffff]], std::move(p[i]));
        if (cnt2[0] == n) {
            for (itype::u32 i = 0; i != n; ++i) p[i] = std::move(tmp[i]);
            return;
        }
        for (itype::u32 i = n; i--;) p[--cnt2[Invoke(proj, tmp[i]) >> 16 & 0xffff]] = std::move(tmp[i]);
        if (cnt3[0] == n) return;
        for (itype::u32 i = n; i--;) tmp[--cnt3[Invoke(proj, p[i]) >> 32 & 0xffff]] = std::move(p[i]);
        if (cnt4[0] == n) {
            for (itype::u32 i = 0; i != n; ++i) p[i] = std::move(tmp[i]);
            return;
        }
        for (itype::u32 i = n; i--;) p[--cnt4[Invoke(proj, tmp[i]) >> 48 & 0xffff]] = std::move(tmp[i]);
    }
    struct Revmsb {
        template<class T> constexpr auto operator()(T x) const {
            using result_type = std::make_unsigned_t<T>;
            return std::bit_cast<result_type>(x) ^ (result_type(1) << (sizeof(T) * 8 - 1));
        }
    };
    struct ToUnsigned {
        template<class T> constexpr auto operator()(T x) const {
            if constexpr (std::same_as<T, ftype::f16>) {
                itype::u16 y = std::bit_cast<itype::u16>(x);
                return itype::u16(y ^ ((y >> 15) != 0 ? ~itype::u16(0) : (itype::u16(1) << 15)));
            } else if constexpr (std::same_as<T, ftype::f32>) {
                itype::u32 y = std::bit_cast<itype::u32>(x);
                return itype::u32(y ^ ((y >> 31) != 0 ? ~itype::u32(0) : (itype::u32(1) << 31)));
            } else if constexpr (std::same_as<T, ftype::f64>) {
                itype::u64 y = std::bit_cast<itype::u64>(x);
                return itype::u64(y ^ ((y >> 63) != 0 ? ~itype::u64(0) : (itype::u64(1) << 63)));
            } else if constexpr (std::same_as<T, ftype::f128>) {
                itype::u128 y = std::bit_cast<itype::u128>(x);
                return itype::u128(y ^ ((y >> 127) != 0 ? ~itype::u128(0) : (itype::u128(1) << 127)));
            }
        }
    };
    template<class T, class Comp, class Proj>
        requires std::is_scalar_v<T>
    GSH_INTERNAL_INLINE constexpr void CompSwap(T* const p, itype::u32 a, itype::u32 b, Comp&& comp = {}, Proj&& proj = {}) {
        bool f = Invoke(comp, Invoke(proj, p[a]), Invoke(proj, p[b]));
        const auto tmp_a = p[a], tmp_b = p[b];
        p[a] = f ? tmp_a : tmp_b;
        p[b] = f ? tmp_b : tmp_a;
    }
    template<class T, class Comp, class Proj> GSH_INTERNAL_NOINLINE constexpr void CompSwap(T* const p, itype::u32 a, itype::u32 b, Comp&& comp = {}, Proj&& proj = {}) {
        using std::swap;
        if (!Invoke(comp, Invoke(proj, p[a]), Invoke(proj, p[b]))) swap(p[a], p[b]);
    }
    template<class T, class Comp = Less, class Proj = Identity> constexpr void SortBlock(T* const p, Comp&& comp = {}, Proj&& proj = {}) {
        // clang-format off
#ifdef F
#define GSH_INTERNAL_DEFINED_MACRO_F
#pragma push_macro("F")
#undef F
#endif
#define F(A,B) CompSwap(p,A,B,comp,proj);
F(0,13)F(1,12)F(2,15)F(3,14)F(4,8)F(5,6)F(7,11)F(9,10)F(0,5)F(1,7)F(2,9)F(3,4)F(6,13)F(8,14)F(10,15)F(11,12)F(0,1)F(2,3)F(4,5)F(6,8)F(7,9)F(10,11)F(12,13)F(14,15)F(0,2)F(1,3)F(4,10)F(5,11)F(6,7)F(8,9)F(12,14)F(13,15)F(1,2)F(3,12)F(4,6)F(5,7)F(8,10)F(9,11)F(13,14)F(1,4)F(2,6)F(5,8)F(7,10)F(9,13)F(11,14)F(2,4)F(3,6)F(9,12)F(11,13)F(3,5)F(6,8)F(7,9)F(10,12)F(3,4)F(5,6)F(7,8)F(9,10)F(11,12)F(6,7)F(8,9)
        // clang-format on
#undef F
#ifdef GSH_INTERNAL_DEFINED_MACRO_F
#undef GSH_INTERNAL_DEFINED_MACRO_F
#pragma pop_macro("F")
#endif
    }
    template<class R, class Comp, class Proj> constexpr void SortImpl(R&& r, Comp&& comp, Proj&& proj) {
        if constexpr (!requires { std::ranges::data(r); }) {
            Arr tmp(std::move_iterator(std::ranges::begin(r)), std::move_sentinel(std::ranges::end(r)));
            SortImpl(tmp, std::forward<Comp>(comp), std::forward<Proj>(proj));
            for (itype::u32 i = 0; auto&& el : r) el = std::move(tmp[i++]);
            return;
        } else {
            const itype::u32 n = std::ranges::size(r);
            auto* const p = std::ranges::data(r);
            using value_type = std::remove_cvref_t<decltype(*p)>;
            if (!std::is_constant_evaluated()) {
                constexpr bool is_less = std::same_as<std::remove_cvref_t<Comp>, Less>;
                constexpr bool is_greater = std::same_as<std::remove_cvref_t<Comp>, Greater>;
                if constexpr ((is_less || is_greater) && std::is_nothrow_move_constructible_v<value_type>) {
                    using inv_result = std::remove_cvref_t<std::invoke_result_t<Proj, std::ranges::range_value_t<R>>>;
                    if constexpr (std::same_as<inv_result, itype::u8> || std::same_as<inv_result, itype::i8> || std::same_as<inv_result, ctype::c8>) {
                        if (n >= 33) {
                            if constexpr (std::is_unsigned_v<inv_result>) internal::SortUnsigned8(p, n, std::forward<Proj>(proj));
                            else internal::SortUnsigned8(p, n, BindFront<Proj, internal::Revmsb>(std::forward<Proj>(proj), internal::Revmsb()));
                            if constexpr (is_greater) ReverseImpl(std::forward<R>(r));
                            return;
                        }
                    }
                    if constexpr (std::same_as<inv_result, itype::u16> || std::same_as<inv_result, itype::i16>) {
                        if (n >= 1200) {
                            if constexpr (std::is_unsigned_v<inv_result>) internal::SortUnsigned16(p, n, std::forward<Proj>(proj));
                            else internal::SortUnsigned16(p, n, BindFront<Proj, internal::Revmsb>(std::forward<Proj>(proj), internal::Revmsb()));
                            if constexpr (is_greater) ReverseImpl(std::forward<R>(r));
                            return;
                        }
                    }
                    if constexpr (std::same_as<inv_result, itype::u32> || std::same_as<inv_result, itype::i32>) {
                        if (n >= 3000) {
                            if constexpr (std::is_unsigned_v<inv_result>) internal::SortUnsigned32(p, n, std::forward<Proj>(proj));
                            else internal::SortUnsigned32(p, n, BindFront<Proj, internal::Revmsb>(std::forward<Proj>(proj), internal::Revmsb()));
                            if constexpr (is_greater) ReverseImpl(std::forward<R>(r));
                            return;
                        }
                    }
                    if constexpr (std::same_as<inv_result, itype::u64> || std::same_as<inv_result, itype::i64>) {
                        if (n >= 8000) {
                            if constexpr (std::is_unsigned_v<inv_result>) internal::SortUnsigned64(p, n, std::forward<Proj>(proj));
                            else internal::SortUnsigned64(p, n, BindFront<Proj, internal::Revmsb>(std::forward<Proj>(proj), internal::Revmsb()));
                            if constexpr (is_greater) ReverseImpl(std::forward<R>(r));
                            return;
                        }
                    }
                    if constexpr (std::same_as<inv_result, ftype::f16>) {
                        if (n >= 1500) {
                            internal::SortUnsigned16(p, n, BindFront<Proj, internal::ToUnsigned>(std::forward<Proj>(proj), internal::ToUnsigned()));
                            if constexpr (is_greater) ReverseImpl(std::forward<R>(r));
                            return;
                        }
                    }
                    if constexpr (std::same_as<inv_result, ftype::f32>) {
                        if (n >= 4000) {
                            internal::SortUnsigned32(p, n, BindFront<Proj, internal::ToUnsigned>(std::forward<Proj>(proj), internal::ToUnsigned()));
                            if constexpr (is_greater) ReverseImpl(std::forward<R>(r));
                            return;
                        }
                    }
                    if constexpr (std::same_as<inv_result, ftype::f64>) {
                        if (n >= 10000) {
                            internal::SortUnsigned64(p, n, BindFront<Proj, internal::ToUnsigned>(std::forward<Proj>(proj), internal::ToUnsigned()));
                            if constexpr (is_greater) ReverseImpl(std::forward<R>(r));
                            return;
                        }
                    }
                }
            }
            constexpr itype::u32 minrun = 16;
            itype::u32 rem = n % minrun;
            for (itype::u32 i = 1; i < rem; ++i) {
                auto tmp = std::move(p[i]);
                itype::u32 j = i;
                for (; j != 0 && Invoke(comp, Invoke(proj, tmp), Invoke(proj, p[j - 1])); --j) p[j] = std::move(p[j - 1]);
                p[j] = std::move(tmp);
            }
            for (itype::u32 i = 0, blocks = n / minrun; i != blocks; ++i) {
                SortBlock(p + rem + i * minrun, comp, proj);
            }
            if (n <= 16) return;
            rem = rem == 0 ? 16 : rem;
            Arr<value_type> tmp(ArrNoInit, n);
            auto merge_seq = [&](value_type* a, itype::u32 a_size, value_type* b, itype::u32 b_size, value_type* dst, auto construct) {
                constexpr bool construct_v = std::same_as<decltype(construct), std::true_type>;
                itype::u32 i = 0, j = 0, k = 0;
                while (i != a_size && j != b_size) {
                    bool f = Invoke(comp, Invoke(proj, a[i]), Invoke(proj, b[j]));
                    if constexpr (construct_v) std::construct_at(&dst[k], std::move(f ? a[i] : b[j]));
                    else dst[k] = std::move(f ? a[i] : b[j]);
                    i += f;
                    j += !f;
                    k += 1;
                }
                for (; i != a_size; ++i, ++k) {
                    if constexpr (construct_v) std::construct_at(&dst[k], std::move(a[i]));
                    else dst[k] = std::move(a[i]);
                }
                for (; j != b_size; ++j, ++k) {
                    if constexpr (construct_v) std::construct_at(&dst[k], std::move(b[j]));
                    else dst[k] = std::move(b[j]);
                }
            };
            merge_seq(p, rem, p + rem, minrun, tmp.data(), std::true_type());
            itype::u32 i = rem + minrun;
            for (; i + 2 * minrun <= n; i += 2 * minrun) {
                merge_seq(p + i, minrun, p + i + minrun, minrun, tmp.data() + i, std::true_type());
            }
            for (; i < n; ++i) {
                std::construct_at(tmp.data() + i, std::move(p[i]));
            }
            value_type *from = tmp.data(), *to = p;
            itype::u32 run = 2 * minrun;
            while (true) {
                itype::u32 nrem = run - minrun + rem;
                if (nrem + run >= n) {
                    merge_seq(from, nrem, from + nrem, n - nrem, to, std::false_type());
                    break;
                }
                merge_seq(from, nrem, from + nrem, run, to, std::false_type());
                itype::u32 i = nrem + run;
                for (; i + 2 * run <= n; i += 2 * run) {
                    merge_seq(from + i, run, from + i + run, run, to + i, std::false_type());
                }
                if (i + run < n) merge_seq(from + i, run, from + i + run, n - (i + run), to + i, std::false_type());
                else {
                    for (; i != n; ++i) to[i] = std::move(from[i]);
                }
                run *= 2;
                std::swap(from, to);
            }
            if (to != p) {
                for (itype::u32 i = 0; i != n; ++i) p[i] = std::move(tmp[i]);
            }
        }
    }
    template<class R, class Comp, class Proj> constexpr auto SortIndexImpl(R&& r, Comp&& comp, Proj&& proj) {
        itype::u32 n = std::ranges::size(r);
        Arr<itype::u32> res(n);
        for (itype::u32 i = 0; i != n; ++i) res[i] = i;
        SortImpl(res, std::forward<Comp>(comp), [start = std::ranges::begin(r), pj = std::forward<Proj>(proj)](itype::u32 n) { return Invoke(pj, *std::ranges::next(start, n)); });
        return res;
    }
    template<class R, class Comp, class Proj> constexpr auto OrderImpl(R&& r, Comp&& comp, Proj&& proj) {
        itype::u32 n = std::ranges::size(r);
        if (n == 0) return Arr<itype::u32>();
        auto idx = SortIndexImpl(r, comp, proj);
        Arr<itype::u32> res(n);
        itype::u32 cnt = 0;
        auto new_proj = [start = std::ranges::begin(r), pj = std::forward<Proj>(proj)](itype::u32 n) {
            return Invoke(pj, *std::ranges::next(start, n));
        };
        for (itype::u32 i = 1; i != n; ++i) {
            cnt += Invoke(comp, new_proj(idx[i - 1]), new_proj(idx[i]));
            res[idx[i]] = cnt;
        }
        return res;
    }

    template<class R, class Comp, class Proj> constexpr auto IsSortedImpl(R&& r, Comp&& comp, Proj&& proj) {
        return std::ranges::is_sorted(std::forward<R>(r), std::forward<Comp>(comp), std::forward<Proj>(proj));
    }
    template<class R, class Comp, class Proj> constexpr auto IsSortedUntilImpl(R&& r, Comp&& comp, Proj&& proj) {
        return std::ranges::is_sorted_until(std::forward<R>(r), std::forward<Comp>(comp), std::forward<Proj>(proj));
    }

    template<class R, class Equal> constexpr auto IsPalindromeImpl(R&& r, Equal&& equal) {
        itype::u32 n = std::ranges::size(r);
        if (n == 0) return true;
        auto first = std::ranges::begin(r);
        auto last = std::ranges::next(first, n - 1);
        for (itype::u32 i = 0; i != n / 2; ++i) {
            if (!Invoke(equal, *first, *last)) return false;
            ++first;
            --last;
        }
        return true;
    }

    template<class Iter, class Sent, class T, class Proj, class Comp> constexpr auto LowerBoundImpl(Iter first, Sent last, const T& value, Comp&& comp, Proj&& proj) {
        itype::u32 len = std::ranges::distance(first, last);
        if (len == 0) [[unlikely]]
            return first;
        auto back = std::ranges::next(first, len - 1);
        if (Invoke(comp, Invoke(proj, *back), value)) [[unlikely]]
            return std::ranges::next(back);
        while (len > 1) {
            itype::u32 half = len / 2;
            std::ranges::advance(first, static_cast<bool>(Invoke(comp, Invoke(proj, *std::ranges::next(first, half - 1)), value)) * half);
            len -= half;
        }
        return first;
    }
    template<class Iter, class Sent, class T, class Proj, class Comp> constexpr auto UpperBoundImpl(Iter first, Sent last, const T& value, Comp&& comp, Proj&& proj) {
        itype::u32 len = std::ranges::distance(first, last);
        if (len == 0) [[unlikely]]
            return first;
        auto back = std::ranges::next(first, len - 1);
        if (!Invoke(comp, value, Invoke(proj, *back))) [[unlikely]]
            return std::ranges::next(back);
        while (len > 1) {
            itype::u32 half = len / 2;
            std::ranges::advance(first, static_cast<bool>(!Invoke(comp, value, Invoke(proj, *std::ranges::next(first, half - 1)))) * half);
            len -= half;
        }
        return first;
    }

}  // namespace internal

template<std::ranges::forward_range R, class Proj = Identity, class Comp = Less> constexpr Arr<itype::u32> LongestIncreasingSubsequence(R&& r, Comp&& comp = {}, Proj&& proj = {}) {
    using T = std::ranges::range_value_t<R>;
    Arr<itype::u32> idx(std::ranges::size(r));
    itype::u32 len = 0;
    {
        Arr<T> dp(idx.size());
        itype::u32 i = 0;
        T *begin = dp.data(), *last = dp.data();
        for (auto&& x : r) {
            if (begin == last || Invoke(comp, *(last - 1), Invoke(proj, x))) [[unlikely]] {
                idx[i++] = last - begin;
                *last = x;
                ++last;
            } else {
                T* loc = LowerBound(Subrange{ begin, last - 1 }, x, comp, proj);
                idx[i++] = loc - begin;
                *loc = x;
            }
        }
        len = last - begin;
    }
    itype::u32 cnt = len - 1;
    Arr<itype::u32> res(len);
    for (itype::u32 i = idx.size(); i--;) {
        if (idx[i] == cnt) res[cnt--] = i;
    }
    return res;
}
template<std::ranges::forward_range R, class Proj = Identity, class Comp = Less> constexpr itype::u32 LongestIncreasingSubsequenceLength(R&& r, Comp&& comp = {}, Proj&& proj = {}) {
    using T = std::ranges::range_value_t<R>;
    Arr<T> dp(std::ranges::size(r));
    T *begin = dp.data(), *last = dp.data();
    for (auto&& x : r) {
        if (begin == last || Invoke(comp, *(last - 1), Invoke(proj, x))) [[unlikely]] {
            *last = x;
            ++last;
        } else {
            T* loc = LowerBound(Subrange{ begin, last - 1 }, x, comp, proj);
            *loc = x;
        }
    }
    return last - begin;
}

template<std::ranges::random_access_range R> constexpr Arr<itype::u32> LongestCommonPrefixArray(R&& r) {
    const itype::u32 n = std::ranges::size(r);
    Arr<itype::u32> res(n);
    if (n == 0) return res;
    res[0] = n;
    const auto itr = std::ranges::begin(r);
    itype::u32 i = 1, j = 0;
    while (i != n) {
        while (i + j < n && *std::ranges::next(itr, j) == *std::ranges::next(itr, i + j)) ++j;
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

template<class T = itype::u64, std::ranges::range R>
    requires std::same_as<std::ranges::range_value_t<R>, itype::u32>
constexpr T CountDistinctSubsequences(R&& r) {
    const itype::u32 n = std::ranges::size(r);
    if (n == 0) return 0;
    Arr<itype::u64> s(n);
    for (itype::u32 i = 0; itype::u32 x : r) {
        s[i] = static_cast<itype::u64>(i) << 32 | x;
        ++i;
    }
    s.sort({}, [](itype::u64 x) { return static_cast<itype::u32>(x); });
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

template<std::ranges::forward_range R> constexpr auto Majority(R&& r) {
    itype::u32 c = 0;
    itype::u32 len = 0;
    auto i = std::ranges::begin(r);
    auto j = std::ranges::end(r), k = j;
    for (; i != j; ++i) {
        ++len;
        if (c == 0) k = i, c = 1;
        else c += static_cast<itype::u32>(static_cast<bool>(*i == *k)) * 2 - 1;
    }
    c = 0;
    for (i = std::ranges::begin(r); i != j; ++i) c += static_cast<bool>(*i == *k);
    return (2 * c >= len ? k : j);
}

class Mo {
    struct rg {
        itype::u32 l, r;
        constexpr rg(itype::u32 a, itype::u32 b) : l(a), r(b) {}
    };
    Vec<rg> qu;
    double coef = 1.05;
public:
    constexpr Mo() {}
    constexpr void reserve(itype::u32 q) { qu.reserve(q); }
    constexpr void query(itype::u32 l, itype::u32 r) { qu.emplace_back(l, r); }
    constexpr void set_coef(double c) { coef = c; }
    template<class F1, class F2, class F3> void run(F1&& add, F2&& del, F3&& slv) const { run(add, add, del, del, slv); }
    template<class F1, class F2, class F3, class F4, class F5> void run(F1&& addl, F2&& addr, F3&& dell, F4&& delr, F5&& slv) const {
        const itype::u32 Q = qu.size();
        itype::u32 N = 0;
        for (itype::u32 i = 0; i != Q; ++i) N = N < qu[i].r ? qu[i].r : N;
        itype::u32 width = coef * std::sqrt(static_cast<ftype::f64>(3ull * N * N) / (2 * Q));
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

}  // namespace gsh
