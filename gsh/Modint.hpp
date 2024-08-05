#pragma once
#include <type_traits>        // std::conditional_t, std::decay_t, std::is_same_v, std::is_integral_v, std::is_unsigned_v
#include <limits>             // std::numeric_limits
#include <bit>                // std::countr_zero, std::has_single_bit
#include <gsh/TypeDef.hpp>    // gsh::itype
#include <gsh/Exception.hpp>  // gsh::Exception

namespace gsh {

namespace internal {

    // https://lpha-z.hatenablog.com/entry/2020/05/24/231500
    template<class T> constexpr T calc_gcd(T x, T y) {
        if (x == 0 || y == 0) return x | y;
        const itype::i32 n = std::countr_zero(x);
        const itype::i32 m = std::countr_zero(y);
        const itype::i32 l = n < m ? n : m;
        x >>= n;
        y >>= m;
        while (x != y) {
            const T a = y - x, b = x - y;
            const itype::i32 m = std::countr_zero(a), n = std::countr_zero(b);
            GSH_INTERNAL_ASSUME(m == n);
            const T s = y < x ? b : a;
            const T t = x < y ? x : y;
            x = s >> m;
            y = t;
        }
        return x << l;
    }

    template<class T> concept IsStaticModint = !requires(T x, typename T::value_type m) { x.set(m); };
    template<class T, itype::u32 id> class ModintBase {
    protected:
        static inline T mint{};
    };
    template<IsStaticModint T, itype::u32 id> class ModintBase<T, id> {
    protected:
        constexpr static T mint{};
    };

    template<class T, itype::u32 id = 0> class ModintImpl : public ModintBase<T, id> {
        typename T::value_type val_{};
        constexpr static auto& mint() noexcept { return ModintBase<T, id>::mint; }
        constexpr static ModintImpl construct(typename T::value_type x) noexcept {
            ModintImpl res;
            res.val_ = x;
            return res;
        }
    public:
        using value_type = typename T::value_type;
        constexpr static bool is_static_mod = IsStaticModint<T>;
        constexpr ModintImpl() noexcept {}
        template<class U> constexpr ModintImpl(U x) noexcept { operator=(x); }
        constexpr explicit operator value_type() const noexcept { return val(); }
        constexpr static void set_mod(value_type x) { mint().set(x); }
        constexpr value_type val() const noexcept { return mint().val(val_); }
        constexpr static value_type mod() noexcept { return mint().mod(); }
        template<class U> constexpr ModintImpl& operator=(U x) noexcept {
            val_ = mint().build(x);
            return *this;
        }
        constexpr static ModintImpl raw(value_type x) noexcept { return construct(mint().raw(x)); }
        constexpr ModintImpl inv() const noexcept { return construct(mint().inv(val_)); }
        constexpr ModintImpl pow(itype::u64 e) const noexcept { return construct(mint().pow(val_, e)); }
        constexpr ModintImpl operator+() const noexcept { return *this; }
        constexpr ModintImpl operator-() const noexcept { return construct(mint().neg(val_)); }
        constexpr ModintImpl& operator++() noexcept {
            val_ = mint().inc(val_);
            return *this;
        }
        constexpr ModintImpl& operator--() noexcept {
            val_ = mint().dec(val_);
            return *this;
        }
        constexpr ModintImpl operator++(int) noexcept {
            ModintImpl copy = *this;
            val_ = mint().inc(val_);
            return copy;
        }
        constexpr ModintImpl operator--(int) noexcept {
            ModintImpl copy = *this;
            val_ = mint().dec(val_);
            return copy;
        }
        constexpr ModintImpl& operator+=(ModintImpl x) noexcept {
            val_ = mint().add(val_, x.val_);
            return *this;
        }
        constexpr ModintImpl& operator-=(ModintImpl x) noexcept {
            val_ = mint().sub(val_, x.val_);
            return *this;
        }
        constexpr ModintImpl& operator*=(ModintImpl x) noexcept {
            val_ = mint().mul(val_, x.val_);
            return *this;
        }
        constexpr ModintImpl& operator/=(ModintImpl x) {
            val_ = mint().div(val_, x.val_);
            return *this;
        }
        friend constexpr ModintImpl operator+(ModintImpl l, ModintImpl r) noexcept { return construct(mint().add(l.val_, r.val_)); }
        friend constexpr ModintImpl operator-(ModintImpl l, ModintImpl r) noexcept { return construct(mint().sub(l.val_, r.val_)); }
        friend constexpr ModintImpl operator*(ModintImpl l, ModintImpl r) noexcept { return construct(mint().mul(l.val_, r.val_)); }
        friend constexpr ModintImpl operator/(ModintImpl l, ModintImpl r) { return construct(mint().div(l.val_, r.val_)); }
        friend constexpr bool operator==(ModintImpl l, ModintImpl r) noexcept { return mint().same(l.val_, r.val_); }
        friend constexpr bool operator!=(ModintImpl l, ModintImpl r) noexcept { return !mint().same(l.val_, r.val_); }
    };

    template<class D, class T> class ModintInterface {
        constexpr D& derived() noexcept { return *static_cast<D*>(this); }
        constexpr const D& derived() const noexcept { return *static_cast<const D*>(this); }
    public:
        using value_type = T;
        constexpr value_type val(value_type x) const noexcept { return x; }
        constexpr value_type build(itype::u32 x) const noexcept { return x % derived().mod(); }
        constexpr value_type build(itype::u64 x) const noexcept { return x % derived().mod(); }
        template<class U> constexpr value_type build(U x) const noexcept {
            static_assert(std::is_integral_v<U>, "gsh::internal::ModintInterface::build<U> / Only integer types can be assigned.");
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
        constexpr value_type raw(value_type x) const noexcept { return x; }
        constexpr value_type zero() const noexcept { return derived().raw(0); }
        constexpr value_type one() const noexcept { return derived().raw(1); }
        constexpr value_type neg(value_type x) const noexcept { return x == 0 ? 0 : derived().mod() - x; }
        constexpr value_type inc(value_type x) const noexcept { return x == derived().mod() - 1 ? 0 : x + 1; }
        constexpr value_type dec(value_type x) const noexcept { return x == 0 ? derived().mod() - 1 : x - 1; }
        constexpr value_type add(value_type x, value_type y) const noexcept { return derived().mod() - x > y ? x + y : y - (derived().mod() - x); }
        constexpr value_type sub(value_type x, value_type y) const noexcept { return x >= y ? x - y : derived().mod() - (y - x); }
        constexpr value_type fma(value_type x, value_type y, value_type z) const noexcept { return derived().add(derived().mul(x, y), z); }
        constexpr value_type div(value_type x, value_type y) const noexcept {
            const value_type iv = derived().inv(y);
            if (derived().same(iv, derived().zero())) [[unlikely]]
                throw gsh::Exception("gsh::internal::ModintInterface::div / Cannot calculate inverse.");
            return derived().mul(x, iv);
        }
        constexpr bool same(value_type x, value_type y) const noexcept { return x == y; }
        constexpr value_type pow(value_type x, itype::u64 e) const noexcept {
            value_type res = derived().one();
            while (e) {
                value_type tmp = derived().mul(x, x);
                if (e & 1) res = derived().mul(res, x);
                x = tmp;
                e >>= 1;
            }
            return res;
        }
        constexpr value_type pow(value_type x, itype::u32 e) const noexcept {
            value_type res = derived().one();
            while (e) {
                value_type tmp = derived().mul(x, x);
                if (e & 1) res = derived().mul(res, x);
                x = tmp;
                e >>= 1;
            }
            return res;
        }
        constexpr value_type inv(value_type t) const noexcept {
            auto a = 1, b = 0, x = derived().val(t), y = derived().mod();
            while (true) {
                if (x <= 1) {
                    if (x == 0) [[unlikely]]
                        return derived().zero();
                    else return derived().build(a);
                }
                b += a * (y / x);
                y %= x;
                if (y <= 1) {
                    if (y == 0) [[unlikely]]
                        return derived().zero();
                    else return derived().build(derived().mod() - b);
                }
                a += b * (x / y);
                x %= y;
            }
        }
    };

    template<itype::u32 mod_> class StaticModint32Impl : public ModintInterface<StaticModint32Impl<mod_>, itype::u32> {
    public:
        constexpr StaticModint32Impl() noexcept {}
        constexpr itype::u32 mod() const noexcept { return mod_; }
        constexpr itype::u32 mul(itype::u32 x, itype::u32 y) const noexcept { return static_cast<itype::u64>(x) * y % mod_; }
    };

    template<itype::u64 mod_> class StaticModint64Impl : public ModintInterface<StaticModint64Impl<mod_>, itype::u64> {
    public:
        constexpr StaticModint64Impl() noexcept {}
        constexpr itype::u64 mod() const noexcept { return mod_; }
        constexpr itype::u64 mul(itype::u64 x, itype::u64 y) const noexcept {
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

    class DynamicModint32Impl : public ModintInterface<DynamicModint32Impl, itype::u32> {
        itype::u32 mod_ = 0;
        itype::u64 M_ = 0;
    public:
        constexpr DynamicModint32Impl() noexcept {}
        constexpr void set(itype::u32 n) {
            if (n <= 1) [[unlikely]]
                throw Exception("gsh::internal::DynamicModint32Impl::set / Mod must be at least 2.");
            mod_ = n;
            M_ = std::numeric_limits<itype::u64>::max() / mod_ + std::has_single_bit(mod_);
        }
        constexpr itype::u32 mod() const noexcept { return mod_; }
        constexpr itype::u32 mul(itype::u32 x, itype::u32 y) const noexcept {
            const itype::u64 a = static_cast<itype::u64>(x) * y;
            const itype::u64 b = (static_cast<itype::u128>(M_) * a) >> 64;
            const itype::u64 c = a - b * mod_;
            return c < mod_ ? c : c - mod_;
        }
    };

    class DynamicModint64Impl : public ModintInterface<DynamicModint64Impl, itype::u64> {
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
            const itype::u64 a = (((M_ * x) >> 64) * y) >> 64;
            const itype::u64 b = x * y;
            const itype::u64 c = a * mod_;
            const itype::u64 d = b - c;
            const bool e = d < mod_;
            const itype::u64 f = d - mod_;
            return e ? d : f;
        }
    };

    class MontgomeryModint64Impl : public ModintInterface<MontgomeryModint64Impl, itype::u64> {
        itype::u64 mod_ = 0, R2 = 0, ninv = 0;
        constexpr itype::u64 reduce(const itype::u128 t) const noexcept {
            const itype::u64 a = t, b = t >> 64;
            const itype::u64 c = (static_cast<itype::u128>(a * ninv) * mod_) >> 64;
            return b + c + (a != 0);
        }
    public:
        constexpr MontgomeryModint64Impl() noexcept {}
        constexpr void set(itype::u64 n) {
            if (n <= 1) [[unlikely]]
                throw Exception("gsh::internal::MontgomeryModint64Impl::set / Mod must be at least 2.");
            if (n % 2 == 0) [[unlikely]]
                throw Exception("gsh::internal::MontgomeryModint64Impl::set / It is not allowed to set the modulo to an even number.");
            mod_ = n;
            R2 = -static_cast<itype::u128>(mod_) % mod_;
            ninv = mod_;
            for (itype::u32 i = 0; i != 5; ++i) ninv *= 2 - mod_ * ninv;
            ninv = -ninv;
        }
        constexpr itype::u64 val(itype::u64 x) const noexcept {
            const itype::u64 res = static_cast<itype::u64>((static_cast<itype::u128>(x * ninv) * mod_) >> 64) + (x != 0);
            return res == mod_ ? 0 : res;
        }
        constexpr itype::u64 mod() const noexcept { return mod_; }
        constexpr itype::u64 build(itype::u32 x) const noexcept { return reduce(static_cast<itype::u128>(x % mod_) * R2); }
        constexpr itype::u64 build(itype::u64 x) const noexcept { return reduce(static_cast<itype::u128>(x % mod_) * R2); }
        template<class U> constexpr itype::u64 build(U x) const noexcept { return ModintInterface::build(x); }
        constexpr itype::u64 raw(itype::u64 x) const noexcept { return reduce(static_cast<itype::u128>(x) * R2); }
        constexpr itype::u64 neg(itype::u64 x) const noexcept {
            const itype::u64 tmp = 2 * mod_ - x;
            return x == 0 ? 0 : tmp;
        }
        constexpr itype::u64 inc(itype::u64 x) const noexcept { return x + 1 == 2 * mod_ ? 0 : x + 1; }
        constexpr itype::u64 dec(itype::u64 x) const noexcept { return x == 0 ? 2 * mod_ - 1 : x - 1; }
        constexpr itype::u64 add(itype::u64 x, itype::u64 y) const noexcept { return x + y >= 2 * mod_ ? x + y - 2 * mod_ : x + y; }
        constexpr itype::u64 sub(itype::u64 x, itype::u64 y) const noexcept { return x - y < 2 * mod_ ? x - y : 2 * mod_ + (x - y); }
        constexpr itype::u64 mul(itype::u64 x, itype::u64 y) const noexcept { return reduce(static_cast<itype::u128>(x) * y); }
        constexpr itype::u64 fma(itype::u64 x, itype::u64 y, itype::u64 z) const noexcept {
            const itype::u128 t = static_cast<itype::u128>(x) * y;
            const itype::u64 a = t, b = t >> 64;
            const itype::u64 c = (static_cast<itype::u128>(a * ninv) * mod_) >> 64;
            const itype::u64 res = b + c + (a != 0) + z;
            return res < 2 * mod_ ? res : res - 2 * mod_;
        }
        constexpr bool same(itype::u64 x, itype::u64 y) const noexcept { return x + mod_ == y || x == y; }
    };

}  // namespace internal

template<itype::u32 mod_ = 998244353> using StaticModint32 = internal::ModintImpl<internal::StaticModint32Impl<mod_>>;
template<itype::u64 mod_ = 998244353> using StaticModint64 = internal::ModintImpl<internal::StaticModint64Impl<mod_>>;
template<itype::u64 mod_ = 998244353> using StaticModint = std::conditional_t<(mod_ <= 0xffffffff), StaticModint32<mod_>, StaticModint64<mod_>>;
template<itype::u32 id = 0> using DynamicModint32 = internal::ModintImpl<internal::DynamicModint32Impl, id>;
template<itype::u32 id = 0> using DynamicModint64 = internal::ModintImpl<internal::DynamicModint64Impl, id>;
template<itype::u32 id = 0> using MontgomeryModint64 = internal::ModintImpl<internal::MontgomeryModint64Impl, id>;

}  // namespace gsh
