#pragma once
#include <type_traits>    // std::conditional_t, std::decay_t, std::is_same_v, std::is_integral_v, std::is_unsigned_v
#include <limits>         // std::numeric_limits
#include <bit>            // std::countr_zero, std::has_single_bit
#include "TypeDef.hpp"    // gsh::itype
#include "Exception.hpp"  // gsh::Exception
#include "Int128.hpp"     //gsh::itype::u128

namespace gsh {

namespace internal {

    template<class T> concept IsStaticModint = !requires(T x, typename T::value_type m) { x.set(m); };
    template<class T, itype::u32 id, bool IsThreadLocal> class ModintBase {
    protected:
        static inline T mint{};
    };
    template<class T, itype::u32 id> class ModintBase<T, id, true> {
    protected:
        thread_local static inline T mint{};
    };
    template<IsStaticModint T, itype::u32 id> class ModintBase<T, id, false> {
    protected:
        constexpr static T mint{};
    };

    template<class T, itype::u32 id = 0, bool IsThreadLocal = false> class ModintInterface : public ModintBase<T, id, IsThreadLocal> {
        typename T::value_type val_{};
        constexpr static auto& mint() noexcept { return ModintBase<T, id, IsThreadLocal>::mint; }
        constexpr static ModintInterface construct(typename T::value_type x) noexcept {
            ModintInterface res;
            res.val_ = x;
            return res;
        }
    public:
        using value_type = typename T::value_type;
        constexpr static bool is_static_mod = IsStaticModint<T>;
        constexpr ModintInterface() noexcept {}
        template<class U> constexpr ModintInterface(U x) noexcept { operator=(x); }
        constexpr explicit operator value_type() const noexcept { return val(); }
        constexpr static void set_mod(value_type x) { mint().set(x); }
        constexpr value_type val() const noexcept { return mint().val(val_); }
        constexpr static value_type mod() noexcept { return mint().mod(); }
        template<class U> constexpr ModintInterface& operator=(U x) noexcept {
            val_ = mint().build(x);
            return *this;
        }
        constexpr static ModintInterface raw(value_type x) noexcept { return construct(mint().raw(x)); }
        constexpr static ModintInterface nan() noexcept { return construct(mint().nan()); }
        constexpr static bool isnan(value_type x) noexcept { return mint.isnan(x); }
        constexpr ModintInterface inv() const noexcept { return construct(mint().inv(val_)); }
        constexpr ModintInterface pow(itype::u64 e) const noexcept { return construct(mint().pow(val_, e)); }
        constexpr ModintInterface operator+() const noexcept { return *this; }
        constexpr ModintInterface operator-() const noexcept { return construct(mint().neg(val_)); }
        constexpr ModintInterface& operator++() noexcept {
            val_ = mint().inc(val_);
            return *this;
        }
        constexpr ModintInterface& operator--() noexcept {
            val_ = mint().dec(val_);
            return *this;
        }
        constexpr ModintInterface operator++(int) noexcept {
            ModintInterface copy = *this;
            val_ = mint().inc(val_);
            return copy;
        }
        constexpr ModintInterface operator--(int) noexcept {
            ModintInterface copy = *this;
            val_ = mint().dec(val_);
            return copy;
        }
        constexpr ModintInterface& operator+=(ModintInterface x) noexcept {
            val_ = mint().add(val_, x.val_);
            return *this;
        }
        constexpr ModintInterface& operator-=(ModintInterface x) noexcept {
            val_ = mint().sub(val_, x.val_);
            return *this;
        }
        constexpr ModintInterface& operator*=(ModintInterface x) noexcept {
            val_ = mint().mul(val_, x.val_);
            return *this;
        }
        constexpr ModintInterface& operator/=(ModintInterface x) {
            val_ = mint().div(val_, x.val_);
            return *this;
        }
        friend constexpr ModintInterface operator+(ModintInterface l, ModintInterface r) noexcept { return construct(mint().add(l.val_, r.val_)); }
        friend constexpr ModintInterface operator-(ModintInterface l, ModintInterface r) noexcept { return construct(mint().sub(l.val_, r.val_)); }
        friend constexpr ModintInterface operator*(ModintInterface l, ModintInterface r) noexcept { return construct(mint().mul(l.val_, r.val_)); }
        friend constexpr ModintInterface operator/(ModintInterface l, ModintInterface r) { return construct(mint().div(l.val_, r.val_)); }
        friend constexpr bool operator==(ModintInterface l, ModintInterface r) noexcept { return mint().same(l.val_, r.val_); }
        friend constexpr bool operator!=(ModintInterface l, ModintInterface r) noexcept { return !mint().same(l.val_, r.val_); }
    };

    template<class D, class T> class ModintImpl {
        constexpr const D& derived() const noexcept { return *static_cast<const D*>(this); }
        constexpr static bool is_static_mod = IsStaticModint<D>;
    public:
        using value_type = T;
        constexpr value_type val(value_type x) const noexcept { return x; }
        constexpr value_type build(itype::u32 x) const noexcept { return x % derived().mod(); }
        constexpr value_type build(itype::u64 x) const noexcept { return x % derived().mod(); }
        template<class U> constexpr value_type build(U x) const noexcept {
            static_assert(std::is_integral_v<U>, "gsh::internal::ModintImpl::build<U> / Only integer types can be assigned.");
            if constexpr (std::is_unsigned_v<U>) {
                if constexpr (std::is_same_v<U, itype::u128>) return derived().raw(static_cast<value_type>(x % derived().mod()));
                else if constexpr (std::is_same_v<U, unsigned long long> || std::is_same_v<U, unsigned long>) return derived().build(static_cast<itype::u64>(x));
                else return derived().build(static_cast<itype::u32>(x));
            } else {
                if (x < 0) {
                    if constexpr (std::is_same_v<U, itype::i128>) return derived().neg(derived().raw(static_cast<value_type>(-x % derived().mod())));
                    else if constexpr (std::is_same_v<U, long long> || std::is_same_v<U, long>) return derived().neg(derived().build(static_cast<itype::u64>(-x)));
                    else return derived().neg(derived().build(static_cast<itype::u32>(-x)));
                } else {
                    if constexpr (std::is_same_v<U, itype::i128>) return derived().raw(static_cast<value_type>(x % derived().mod()));
                    else if constexpr (std::is_same_v<U, long long> || std::is_same_v<U, long>) return derived().build(static_cast<itype::u64>(x));
                    else return derived().build(static_cast<itype::u32>(x));
                }
            }
        }
        constexpr value_type raw(value_type x) const noexcept {
            Assume(x < derived().mod());
            return x;
        }
        constexpr value_type nan() const noexcept { return std::numeric_limits<value_type>::max(); }
        constexpr value_type isnan(value_type x) const noexcept { return x == derived().nan(); }
        constexpr value_type zero() const noexcept { return derived().raw(0); }
        constexpr value_type one() const noexcept { return derived().raw(1); }
        constexpr value_type neg(value_type x) const noexcept {
            Assume(x < derived().mod());
            return x == 0 ? 0 : derived().mod() - x;
        }
        constexpr value_type inc(value_type x) const noexcept {
            Assume(x < derived().mod());
            return x + 1 == derived().mod() ? 0 : x + 1;
        }
        constexpr value_type dec(value_type x) const noexcept {
            Assume(x < derived().mod());
            return x == 0 ? derived().mod() - 1 : x - 1;
        }
        constexpr value_type add(value_type x, value_type y) const noexcept {
            Assume(x < derived().mod() && y < derived().mod());
            return x + y - (derived().mod() - x <= y) * derived().mod();
        }
        constexpr value_type sub(value_type x, value_type y) const noexcept {
            Assume(x < derived().mod() && y < derived().mod());
            return x - y + (x < y) * derived().mod();
        }
        constexpr value_type fma(value_type x, value_type y, value_type z) const noexcept { return derived().add(derived().mul(x, y), z); }
        constexpr value_type div(value_type x, value_type y) const {
            const value_type iv = derived().inv(y);
            if (derived().same(iv, derived().zero())) [[unlikely]]
                throw gsh::Exception("gsh::internal::ModintImpl::div / Cannot calculate inverse.");
            return derived().mul(x, iv);
        }
        constexpr bool same(value_type x, value_type y) const noexcept { return x == y; }
        constexpr value_type abs(value_type x) const noexcept {
            Assume(x < derived().mod());
            return derived().val(x) > (derived().mod() / 2) ? derived().neg(x) : x;
        }
        constexpr value_type pow(value_type x, itype::u64 e) const noexcept {
            value_type res = derived().one();
            while (e) {
                auto tmp = derived().mul(x, x);
                if (e & 1) res = derived().mul(res, x);
                x = tmp;
                e >>= 1;
            }
            return res;
        }
        constexpr value_type inv(value_type t) const noexcept {
            value_type a = 1, b = 0, x = derived().val(t), y = derived().mod();
            while (true) {
                if (x <= 1) {
                    if (x == 0) [[unlikely]]
                        return derived().nan();
                    return derived().raw(a);
                }
                b += a * (y / x);
                y %= x;
                if (y <= 1) {
                    if (y == 0) [[unlikely]]
                        return derived().nan();
                    return derived().raw(derived().mod() - b);
                }
                a += b * (x / y);
                x %= y;
            }
        }
        constexpr itype::i32 legendre(value_type x) const noexcept {
            auto res = derived().pow(x, (derived().mod() - 1) >> 1);
            const bool a = derived().same(res, derived().zero()), b = derived().same(res, derived().one());
            return a ? 0 : (b ? 1 : -1);
        }
        constexpr itype::i32 jacobi(value_type x) const noexcept {
            auto a = derived().val(x), n = derived().mod();
            if (a == 1) return 1;
            itype::i32 res = 1;
            while (a != 0) {
                while (!(a & 1) && a != 0) {
                    a >>= 1;
                    res = ((n & 0b111) == 3 || (n & 0b111) == 5) ? -res : res;
                }
                res = ((a & 0b11) == 3 || (n & 0b11) == 3) ? -res : res;
                auto tmp = n;
                n = a;
                a = tmp;
                a %= n;
            }
            return n == 1 ? res : 0;
        }
        constexpr value_type sqrt(value_type n) const noexcept {
            const auto md = derived().mod();
            if (md % 4 == 3) {
                auto res = derived().pow(n, (md + 1) >> 2);
                if (!derived().same(derived().mul(res, res), n)) return derived().nan();
                else return derived().abs(res);
            } else if (md % 8 == 5) {
                auto res = derived().pow(n, (md + 3) >> 3);
                if (!derived().same(derived().mul(res, res), n)) {
                    const auto p = derived().pow(derived().raw(2), (md - 1) >> 2);
                    res = derived().mul(res, p);
                    if (!derived().same(derived().mul(res, res), n)) return derived().nan();
                    else return derived().abs(res);
                }
                return derived().abs(res);
            } else {
                if (derived().same(n, derived().zero()) || derived().same(n, derived().one())) return n;
                const itype::u32 S = std::countr_zero(md - 1);
                const itype::u32 W = std::bit_width(md);
                if (S * S <= 12 * W) {
                    const auto Q = (md - 1) >> S;
                    const auto tmp = derived().pow(n, Q / 2);
                    auto R = derived().mul(tmp, n), t = derived().mul(tmp, R);
                    if (derived().same(t, derived().one())) return R;
                    auto u = t;
                    for (itype::u32 i = 0; i != S - 1; ++i) u = derived().mul(u, u);
                    if (!derived().same(u, derived().one())) return derived().nan();
                    const auto base = [&]() GSH_INTERNAL_INLINE {
                        if (md % 3 == 2) return derived().raw(3);
                        if (auto x = md % 5; x == 2 || x == 3) return derived().raw(5);
                        if (auto x = md % 7; x == 3 || x == 5 || x == 6) return derived().raw(7);
                        if (auto x = md % 11; x == 2 || x == 6 || x == 7 || x == 8 || x == 10) return derived().build(11);
                        if (auto x = md % 13; x == 2 || x == 5 || x == 6 || x == 7 || x == 8 || x == 11) return derived().build(13);
                        for (const itype::u32 x : { 17, 19, 23, 29, 31, 37, 41, 43, 47, 53, 59, 61, 67, 71, 73, 79, 83, 89, 97 }) {
                            const auto y = derived().build(x);
                            if (derived().legendre(y) == -1) return y;
                        }
                        auto z = derived().build(101);
                        while (derived().legendre(z) != -1) z = derived().add(z, derived().raw(2));
                        return z;
                    }();
                    const auto z = derived().pow(base, Q);
                    itype::u32 M = S;
                    auto c = z;
                    do {
                        auto U = derived().mul(t, t);
                        itype::u32 i = 1;
                        while (!derived().same(U, derived().one())) U = derived().mul(U, U), ++i;
                        auto b = c;
                        for (itype::u32 j = 0, k = M - i - 1; j != k; ++j) b = derived().mul(b, b);
                        M = i, c = derived().mul(b, b), t = derived().mul(t, c), R = derived().mul(R, b);
                    } while (!derived().same(t, derived().one()));
                    return derived().abs(R);
                } else {
                    if (derived().legendre(n) != 1) return derived().nan();
                    auto a = derived().raw(4);
                    decltype(a) w;
                    while (derived().legendre(w = derived().sub(derived().mul(a, a), n)) != -1) a = derived().inc(a);
                    auto res1 = derived().one(), res2 = derived().zero(), pow1 = a, pow2 = res1;
                    auto e = (md + 1) / 2;
                    while (true) {
                        const auto tmp2 = derived().mul(pow2, w);
                        if (e & 1) {
                            const auto tmp = res1;
                            res1 = derived().add(derived().mul(res1, pow1), derived().mul(res2, tmp2));
                            res2 = derived().add(derived().mul(tmp, pow2), derived().mul(res2, pow1));
                        }
                        e >>= 1;
                        if (e == 0) return derived().abs(res1);
                        const auto tmp = pow1;
                        pow1 = derived().add(derived().mul(pow1, pow1), derived().mul(pow2, tmp2));
                        pow2 = derived().mul(pow2, derived().add(tmp, tmp));
                    }
                }
            }
        }
    };

    template<itype::u32 mod_> class StaticModint32Impl : public ModintImpl<StaticModint32Impl<mod_>, itype::u32> {
    public:
        constexpr StaticModint32Impl() noexcept {}
        constexpr itype::u32 mod() const noexcept { return mod_; }
        constexpr itype::u32 mul(itype::u32 x, itype::u32 y) const noexcept {
            Assume(x < mod_ && y < mod_);
            return static_cast<itype::u64>(x) * y % mod_;
        }
    };
    template<itype::u64 mod_> class StaticModint64Impl : public ModintImpl<StaticModint64Impl<mod_>, itype::u64> {
    public:
        constexpr StaticModint64Impl() noexcept {}
        constexpr itype::u64 mod() const noexcept { return mod_; }
        constexpr itype::u64 mul(itype::u64 x, itype::u64 y) const noexcept {
            Assume(x < mod_ && y < mod_);
            constexpr itype::u128 M_ = std::numeric_limits<itype::u128>::max() / mod_ + std::has_single_bit(mod_);
            if constexpr (mod_ < (1ull << 63)) {
                const itype::u64 a = (((M_ * x) >> 64) * y) >> 64;
                const itype::u64 b = x * y;
                const itype::u64 c = a * mod_;
                const itype::u64 d = b - c;
                const bool e = d < mod_;
                const itype::u64 f = d - mod_;
                return e ? d : f;
            } else {
                const itype::u64 a = (((M_ * x) >> 64) * y) >> 64;
                const itype::u128 b = static_cast<itype::u128>(x) * y;
                const itype::u128 c = static_cast<itype::u128>(a) * mod_;
                const itype::u128 d = b - c;
                const bool e = d < mod_;
                const itype::u128 f = d - mod_;
                return e ? d : f;
            }
        }
    };
    template<itype::u64 mod_> struct SwitchStaticModint {
        using type = StaticModint64Impl<mod_>;
    };
    template<itype::u64 mod_>
        requires(mod_ <= 0xffffffff)
    struct SwitchStaticModint<mod_> {
        using type = StaticModint32Impl<mod_>;
    };
    template<itype::u64 mod_> using StaticModintImpl = typename SwitchStaticModint<mod_>::type;


    class DynamicModint32Impl : public ModintImpl<DynamicModint32Impl, itype::u32> {
        itype::u32 mod_ = 0;
        itype::u64 M_ = 0;
    public:
        constexpr DynamicModint32Impl() noexcept {}
        constexpr void set(itype::u32 n) {
            if (n <= 1) [[unlikely]]
                throw Exception("gsh::internal::DynamicModint32Impl::set / Mod must be at least 2.");
            mod_ = n;
            M_ = std::numeric_limits<itype::u64>::max() / mod_ + 1;
        }
        constexpr itype::u32 mod() const noexcept { return mod_; }
        constexpr itype::u64 build(itype::u32 x) const noexcept {
            itype::u64 lowbit = M_ * x;
            return (static_cast<itype::u128>(lowbit) * mod_) >> 64;
        }
        constexpr itype::u64 build(itype::u64 x) const noexcept { return x % mod_; }
        template<class U> constexpr itype::u64 build(U x) const noexcept { return ModintImpl::build(x); }
        constexpr itype::u32 mul(itype::u32 x, itype::u32 y) const noexcept {
            Assume(x < mod_ && y < mod_);
            const itype::u64 a = static_cast<itype::u64>(x) * y;
            const itype::u64 b = (static_cast<itype::u128>(M_) * a) >> 64;
            const itype::u64 c = a - b * mod_;
            return c + (c >= mod_) * mod_;
        }
    };

    class DynamicModint64Impl : public ModintImpl<DynamicModint64Impl, itype::u64> {
        itype::u64 mod_ = 0;
        itype::u128 M_ = 0;
    public:
        constexpr DynamicModint64Impl() noexcept {}
        constexpr void set(itype::u64 n) {
            if (n <= 1) [[unlikely]]
                throw Exception("gsh::internal::DynamicModint64Impl::set / Mod must be at least 2.");
            mod_ = n;
            M_ = std::numeric_limits<itype::u128>::max() / mod_ + std::has_single_bit(mod_);
        }
        constexpr itype::u64 mod() const noexcept { return mod_; }
        constexpr itype::u64 mul(itype::u64 x, itype::u64 y) const noexcept {
            Assume(x < mod_ && y < mod_);
            const itype::u64 a = (((M_ * x) >> 64) * y) >> 64;
            const itype::u64 b = x * y;
            const itype::u64 c = a * mod_;
            const itype::u64 d = b - c;
            const bool e = d < mod_;
            const itype::u64 f = d - mod_;
            return e ? d : f;
        }
    };

    class MontgomeryModint64Impl : public ModintImpl<MontgomeryModint64Impl, itype::u64> {
        itype::u64 mod_ = 0, rs = 0, nr = 0, np = 0;
        constexpr itype::u64 reduce(const itype::u64 t) const noexcept {
            itype::u64 q = t * nr;
            itype::u64 m = (static_cast<itype::u128>(q) * mod_) >> 64;
            return mod_ - m;
        }
        constexpr itype::u64 reduce(const itype::u64 a, const itype::u64 b) const noexcept {
            itype::u128 t = static_cast<itype::u128>(a) * b;
            itype::u64 c = t, d = t >> 64;
            itype::u64 q = c * nr;
            itype::u64 m = (static_cast<itype::u128>(q) * mod_) >> 64;
            return d + mod_ - m;
        }
    public:
        constexpr MontgomeryModint64Impl() noexcept {}
        constexpr void set(itype::u64 n) {
            if (n <= 1) [[unlikely]]
                throw Exception("gsh::internal::MontgomeryModint64Impl::set / Mod must be at least 2.");
            if (n % 2 == 0) [[unlikely]]
                throw Exception("gsh::internal::MontgomeryModint64Impl::set / It is not allowed to set the modulo to an even number.");
            mod_ = n;
            rs = -static_cast<itype::u128>(n) % n;
            nr = n;
            for (itype::u32 i = 0; i != 6; ++i) nr *= 2 - n * nr;
            np = reduce(1, rs);
        }
        constexpr itype::u64 val(itype::u64 x) const noexcept {
            itype::u64 tmp = reduce(x);
            return tmp - (tmp == mod_) * mod_;
        }
        constexpr itype::u64 mod() const noexcept { return mod_; }
        constexpr itype::u64 build(itype::u32 x) const noexcept { return reduce(x % mod_, rs); }
        constexpr itype::u64 build(itype::u64 x) const noexcept { return reduce(x % mod_, rs); }
        template<class U> constexpr itype::u64 build(U x) const noexcept { return ModintImpl::build(x); }
        constexpr itype::u64 raw(itype::u64 x) const noexcept {
            Assume(x < mod_);
            return reduce(x, rs);
        }
        constexpr itype::u64 zero() const noexcept { return 0; }
        constexpr itype::u64 one() const noexcept {
            Assume(np < 2 * mod_);
            return np;
        }
        constexpr itype::u64 neg(itype::u64 x) const noexcept {
            Assume(x < 2 * mod_);
            return (2 * mod_ - x) * (x != 0);
        }
        constexpr itype::u64 inc(itype::u64 x) const noexcept { return add(x, np); }
        constexpr itype::u64 dec(itype::u64 x) const noexcept { return sub(x, np); }
        constexpr itype::u64 add(itype::u64 x, itype::u64 y) const noexcept {
            Assume(x < 2 * mod_ && y < 2 * mod_);
            return x + y - (x + y >= mod_) * mod_;
        }
        constexpr itype::u64 sub(itype::u64 x, itype::u64 y) const noexcept {
            Assume(x < 2 * mod_ && y < 2 * mod_);
            return x - y + (x < y) * (2 * mod_);
        }
        constexpr itype::u64 mul(itype::u64 x, itype::u64 y) const noexcept {
            Assume(x < 2 * mod_ && y < 2 * mod_);
            return reduce(x, y);
        }
        constexpr bool same(itype::u64 x, itype::u64 y) const noexcept {
            Assume(x < 2 * mod_ && y < 2 * mod_);
            itype::u64 tmp = x - y;
            return (tmp == 0) || (tmp == mod_) || (tmp == -mod_);
        }
        constexpr itype::u64 abs(itype::u64 x) const noexcept {
            itype::u64 tmp = neg(x);
            return val(x) > mod_ / 2 ? tmp : x;
        }
        constexpr itype::u64 norm(itype::u64 x) const noexcept { return x >= mod_ ? x - mod_ : x; }
    };

}  // namespace internal

template<itype::u32 mod_ = 998244353> using StaticModint32 = internal::ModintInterface<internal::StaticModint32Impl<mod_>>;
template<itype::u64 mod_ = 998244353> using StaticModint64 = internal::ModintInterface<internal::StaticModint64Impl<mod_>>;
template<itype::u64 mod_ = 998244353> using StaticModint = internal::ModintInterface<internal::StaticModintImpl<mod_>>;
template<itype::u32 id = 0> using DynamicModint32 = internal::ModintInterface<internal::DynamicModint32Impl, id>;
template<itype::u32 id = 0> using DynamicModint64 = internal::ModintInterface<internal::DynamicModint64Impl, id>;
template<itype::u32 id = 0> using MontgomeryModint64 = internal::ModintInterface<internal::MontgomeryModint64Impl, id>;
template<itype::u32 id = 0> using ThreadLocalDynamicModint32 = internal::ModintInterface<internal::DynamicModint32Impl, id, true>;
template<itype::u32 id = 0> using ThreadLocalDynamicModint64 = internal::ModintInterface<internal::DynamicModint64Impl, id, true>;
template<itype::u32 id = 0> using ThreadLocalMontgomeryModint64 = internal::ModintInterface<internal::MontgomeryModint64Impl, id, true>;

}  // namespace gsh
