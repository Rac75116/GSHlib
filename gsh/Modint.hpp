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
        if (x == 0 || y == 0) [[unlikely]]
            return x | y;
        const int n = std::countr_zero(x);
        const int m = std::countr_zero(y);
        const int l = n < m ? n : m;
        x >>= n;
        y >>= m;
        T s;
        int t;
        while (x != y) {
            s = y < x ? x - y : y - x;
            t = std::countr_zero(s);
            y = y < x ? y : x;
            x = s >> t;
        }
        return x << l;
    }

    template<class T> class ModintTraits : public T {
        using base_type = T;
    public:
        using value_type = std::decay_t<decltype(base_type::mod())>;
        using modint_type = ModintTraits;
        constexpr static bool is_staticmod = !requires { base_type::set_mod(0); };
        constexpr ModintTraits() noexcept : T() {}
        template<class U> constexpr ModintTraits(U x) noexcept { operator=(x); }
        constexpr explicit operator value_type() const noexcept { return val(); }
        constexpr static void set_mod(value_type x) {
            static_assert(!is_staticmod, "ModintTraits::set_mod / Mod must be dynamic.");
            if (x <= 1) throw gsh::Exception("ModintTraits::set_mod / Mod must be at least 2.");
            if (x == mod()) return;
            base_type::set_mod(x);
        }
        constexpr value_type val() const noexcept { return base_type::val(); }
        constexpr static value_type mod() noexcept { return base_type::mod(); }
        template<class U> constexpr modint_type& operator=(U x) noexcept {
            static_assert(std::is_integral_v<U>, "ModintTraits::operator= / Only integer types can be assigned.");
            if constexpr (std::is_unsigned_v<U>) {
                if constexpr (std::is_same_v<U, unsigned long long> || std::is_same_v<U, unsigned long>) base_type::assign(static_cast<gsh::itype::u64>(x));
                else base_type::assign(static_cast<gsh::itype::u32>(x));
            } else {
                if (x < 0) {
                    if constexpr (std::is_same_v<U, long long> || std::is_same_v<U, long>) base_type::assign(static_cast<gsh::itype::u64>(-x));
                    else base_type::assign(static_cast<gsh::itype::u32>(-x));
                    base_type::neg();
                } else {
                    if constexpr (std::is_same_v<U, long long> || std::is_same_v<U, long>) base_type::assign(static_cast<gsh::itype::u64>(x));
                    else base_type::assign(static_cast<gsh::itype::u32>(x));
                }
            }
            return *this;
        }
        constexpr static modint_type raw(value_type x) noexcept {
            modint_type res;
            res.rawassign(x);
            return res;
        }
        template<class Istream> friend Istream& operator>>(Istream& ist, modint_type& x) {
            value_type n;
            ist >> n;
            x = n;
            return ist;
        }
        template<class Ostream> friend Ostream& operator<<(Ostream& ost, modint_type x) { return ost << x.val(); }
        constexpr modint_type inv() const {
            value_type a = 1, b = 0, x = val(), y = mod();
            if (x == 0) throw gsh::Exception("ModintTraits::inv / Zero division is not possible.");
            while (true) {
                if (x <= 1) {
                    if (x == 0) [[unlikely]]
                        break;
                    else return modint_type::raw(a);
                }
                b += a * (y / x);
                y %= x;
                if (y <= 1) {
                    if (y == 0) [[unlikely]]
                        break;
                    else return modint_type::raw(mod() - b);
                }
                a += b * (x / y);
                x %= y;
            }
            throw gsh::Exception("ModintTraits::inv / Cannot calculate inverse element.");
        }
        constexpr modint_type pow(gsh::itype::u64 e) const noexcept {
            modint_type res = modint_type::raw(1), pow = *this;
            while (e) {
                modint_type tmp = pow * pow;
                if (e & 1) res *= pow;
                pow = tmp;
                e >>= 1;
            }
            return res;
        }
        constexpr modint_type operator+() const noexcept { return *this; }
        constexpr modint_type operator-() const noexcept {
            modint_type res = *this;
            res.neg();
            return res;
        }
        constexpr modint_type& operator++() noexcept {
            base_type::inc();
            return *this;
        }
        constexpr modint_type& operator--() noexcept {
            base_type::dec();
            return *this;
        }
        constexpr modint_type operator++(int) noexcept {
            modint_type copy = *this;
            operator++();
            return copy;
        }
        constexpr modint_type operator--(int) noexcept {
            modint_type copy = *this;
            operator--();
            return copy;
        }
        constexpr modint_type& operator+=(modint_type x) noexcept {
            base_type::add(x);
            return *this;
        }
        constexpr modint_type& operator-=(modint_type x) noexcept {
            base_type::sub(x);
            return *this;
        }
        constexpr modint_type& operator*=(modint_type x) noexcept {
            base_type::mul(x);
            return *this;
        }
        constexpr modint_type& operator/=(modint_type x) {
            operator*=(x.inv());
            return *this;
        }
        friend constexpr modint_type operator+(modint_type l, modint_type r) noexcept { return modint_type(l) += r; }
        friend constexpr modint_type operator-(modint_type l, modint_type r) noexcept { return modint_type(l) -= r; }
        friend constexpr modint_type operator*(modint_type l, modint_type r) noexcept { return modint_type(l) *= r; }
        friend constexpr modint_type operator/(modint_type l, modint_type r) { return modint_type(l) /= r; }
        friend constexpr bool operator==(modint_type l, modint_type r) noexcept { return l.val() == r.val(); }
        friend constexpr bool operator!=(modint_type l, modint_type r) noexcept { return l.val() != r.val(); }
        constexpr int legendre() const noexcept {
            value_type res = pow((mod() - 1) >> 1).val();
            return (res <= 1 ? static_cast<int>(res) : -1);
        }
        constexpr int jacobi() const noexcept {
            value_type a = val(), n = mod();
            if (a == 1) return 1;
            if (calc_gcd(a, n) != 1) return 0;
            int res = 1;
            while (a != 0) {
                while (!(a & 1) && a != 0) {
                    a >>= 1;
                    if ((n & 0b111) == 3 || (n & 0b111) == 5) res = -res;
                }
                if ((a & 0b11) == 3 || (n & 0b11) == 3) res = -res;
                value_type tmp = n;
                n = a;
                a = tmp;
                a %= n;
            }
            if (n != 1) return 0;
            return res;
        }
        constexpr modint_type sqrt() const noexcept {
            const value_type vl = val(), md = mod();
            if (vl <= 1) return *this;
            auto get_min = [](modint_type x) {
                return x.val() > (mod() >> 1) ? -x : x;
            };
            if ((md & 0b11) == 3) return get_min(pow((md + 1) >> 2));
            else if ((md & 0b111) == 5) {
                modint_type res = pow((md + 3) >> 3);
                if constexpr (is_staticmod) {
                    constexpr modint_type p = modint_type::raw(2).pow((md - 1) >> 2);
                    res *= p;
                } else if (res * res != *this) res *= modint_type::raw(2).pow((md - 1) >> 2);
                return get_min(res);
            } else {
                value_type Q = md - 1;
                gsh::itype::u32 S = 0;
                while ((Q & 1) == 0) Q >>= 1, ++S;
                if (std::countr_zero(md - 1) < 6) {
                    modint_type z = modint_type::raw(1);
                    while (z.legendre() != -1) ++z;
                    modint_type t = pow(Q), R = pow((Q + 1) / 2);
                    if (t.val() == 1) return R;
                    gsh::itype::u32 M = S;
                    modint_type c = z.pow(Q);
                    do {
                        modint_type U = t * t;
                        gsh::itype::u32 i = 1;
                        while (U.val() != 1) U = U * U, ++i;
                        modint_type b = c;
                        for (gsh::itype::u32 j = 0; j < (M - i - 1); ++j) b *= b;
                        M = i, c = b * b, t *= c, R *= b;
                    } while (t.val() != 1);
                    return get_min(R);
                } else {
                    modint_type a = 1;
                    while ((a * a - *this).legendre() != -1) ++a;
                    modint_type res1 = modint_type::raw(1), res2, pow1 = a, pow2 = modint_type::raw(1), w = a * a - *this;
                    value_type e = (md + 1) / 2;
                    while (true) {
                        if (e & 1) {
                            modint_type tmp = res1;
                            res1 = res1 * pow1 + res2 * pow2 * w;
                            res2 = tmp * pow2 + res2 * pow1;
                        }
                        e >>= 1;
                        if (e == 0) return get_min(res1);
                        modint_type tmp = pow1;
                        pow1 = pow1 * pow1 + pow2 * pow2 * w;
                        pow2 *= modint_type::raw(2) * tmp;
                    }
                }
            }
        }
    };

    template<gsh::itype::u32 mod_> class StaticModint32_impl {
        using value_type = gsh::itype::u32;
        using modint_type = StaticModint32_impl;
        value_type val_ = 0;
    protected:
        constexpr StaticModint32_impl() noexcept {}
        constexpr value_type val() const noexcept { return val_; }
        static constexpr value_type mod() noexcept { return mod_; }
        constexpr void assign(gsh::itype::u32 x) noexcept { val_ = x % mod_; }
        constexpr void assign(gsh::itype::u64 x) noexcept { val_ = x % mod_; }
        constexpr void rawassign(value_type x) noexcept { val_ = x; }
        constexpr void neg() noexcept { val_ = (val_ == 0 ? 0 : mod_ - val_); }
        constexpr void inc() noexcept { val_ = (val_ == mod_ - 1 ? 0 : val_ + 1); }
        constexpr void dec() noexcept { val_ = (val_ == 0 ? mod_ - 1 : val_ - 1); }
        constexpr void add(modint_type x) noexcept {
            if (mod_ - val_ > x.val_) val_ += x.val_;
            else val_ = x.val_ - (mod_ - val_);
        }
        constexpr void sub(modint_type x) noexcept {
            if (val_ >= x.val_) val_ -= x.val_;
            else val_ = mod_ - (x.val_ - val_);
        }
        constexpr void mul(modint_type x) noexcept { val_ = static_cast<gsh::itype::u64>(val_) * x.val_ % mod_; }
    };

    template<gsh::itype::u64 mod_> class StaticModint64_impl {
        using value_type = gsh::itype::u64;
        using modint_type = StaticModint64_impl;
        value_type val_ = 0;
    protected:
        constexpr StaticModint64_impl() noexcept {}
        constexpr value_type val() const noexcept { return val_; }
        static constexpr value_type mod() noexcept { return mod_; }
        constexpr void assign(gsh::itype::u32 x) noexcept {
            if constexpr (mod_ < (1ull << 32)) val_ = x % mod_;
            else val_ = x;
        }
        constexpr void assign(gsh::itype::u64 x) noexcept { val_ = x % mod_; }
        constexpr void rawassign(value_type x) noexcept { val_ = x; }
        constexpr void neg() noexcept { val_ = (val_ == 0 ? 0 : mod_ - val_); }
        constexpr void inc() noexcept { val_ = (val_ == mod_ - 1 ? 0 : val_ + 1); }
        constexpr void dec() noexcept { val_ = (val_ == 0 ? mod_ - 1 : val_ - 1); }
        constexpr void add(modint_type x) noexcept {
            if (mod_ - val_ > x.val_) val_ += x.val_;
            else val_ = x.val_ - (mod_ - val_);
        }
        constexpr void sub(modint_type x) noexcept {
            if (val_ >= x.val_) val_ -= x.val_;
            else val_ = mod_ - (x.val_ - val_);
        }
        constexpr void mul(modint_type x) noexcept { val_ = static_cast<gsh::itype::u128>(val_) * x.val_ % mod_; }
    };

    template<int id> class DynamicModint32_impl {
        using value_type = gsh::itype::u32;
        using modint_type = DynamicModint32_impl;
        static inline value_type mod_ = 0;
        static inline gsh::itype::u128 L_ = 0;
        value_type val_ = 0;
        value_type reduce(gsh::itype::u32 c) const noexcept {
            gsh::itype::u32 q = (c * L_) >> 96;
            return c - q * mod_;
        }
        value_type reduce(gsh::itype::u64 c) const noexcept {
            gsh::itype::u64 q = (c * L_) >> 96;
            return c - q * mod_;
        }
    protected:
        DynamicModint32_impl() noexcept {}
        static void set_mod(value_type newmod) noexcept {
            mod_ = newmod;
            L_ = ((gsh::itype::u128(1) << 96) - 1) / mod_ + 1;
        }
        value_type val() const noexcept { return val_; }
        static value_type mod() noexcept { return mod_; }
        void assign(gsh::itype::u32 x) noexcept { val_ = reduce(x); }
        void assign(gsh::itype::u64 x) noexcept { val_ = reduce(x); }
        void rawassign(value_type x) noexcept { val_ = x; }
        void neg() noexcept { val_ = (val_ == 0 ? 0 : mod_ - val_); }
        void inc() noexcept { val_ = (val_ == mod_ - 1 ? 0 : val_ + 1); }
        void dec() noexcept { val_ = (val_ == 0 ? mod_ - 1 : val_ - 1); }
        void add(modint_type x) noexcept {
            const auto tmp = val_ + x.val_;
            const auto tmp2 = mod_ - val_;
            val_ = tmp2 > x.val_ ? tmp : x.val_ - tmp2;
        }
        void sub(modint_type x) noexcept {
            if (val_ >= x.val_) val_ -= x.val_;
            else val_ = mod_ - (x.val_ - val_);
        }
        void mul(modint_type x) noexcept { val_ = reduce(static_cast<gsh::itype::u64>(val_) * x.val_); }
    };

    template<int id> class DynamicModint64_impl {
        using value_type = gsh::itype::u64;
        using modint_type = DynamicModint64_impl;
        static inline value_type mod_ = 0;
        static inline gsh::itype::u128 M_ = 0;
        value_type val_ = 0;
    protected:
        DynamicModint64_impl() noexcept {}
        static void set_mod(value_type newmod) noexcept {
            mod_ = newmod;
            M_ = std::numeric_limits<gsh::itype::u128>::max() / mod_ + std::has_single_bit(mod_);
        }
        value_type val() const noexcept { return val_; }
        static value_type mod() noexcept { return mod_; }
        void assign(gsh::itype::u64 x) noexcept { val_ = x % mod_; }
        void rawassign(value_type x) noexcept { val_ = x; }
        void neg() noexcept { val_ = (val_ == 0 ? 0 : mod_ - val_); }
        void inc() noexcept { val_ = (val_ == mod_ - 1 ? 0 : val_ + 1); }
        void dec() noexcept { val_ = (val_ == 0 ? mod_ - 1 : val_ - 1); }
        void add(modint_type x) noexcept {
            const auto tmp = val_ + x.val_;
            const auto tmp2 = mod_ - val_;
            val_ = tmp2 > x.val_ ? tmp : x.val_ - tmp2;
        }
        void sub(modint_type x) noexcept {
            if (val_ >= x.val_) val_ -= x.val_;
            else val_ = mod_ - (x.val_ - val_);
        }
        void mul(modint_type x) noexcept {
            const gsh::itype::u64 a = (((M_ * val_) >> 64) * x.val_) >> 64;
            const gsh::itype::u64 b = val_ * x.val_;
            const gsh::itype::u64 c = a * mod_;
            const gsh::itype::u64 d = b - c;
            const bool e = d < mod_;
            const gsh::itype::u64 f = d - mod_;
            val_ = e ? d : f;
        }
    };

    template<int id> class MontgomeryModint32_impl {
        using value_type = gsh::itype::u32;
        using modint_type = MontgomeryModint32_impl;
        static inline value_type mod_ = 0;
        static inline value_type R2 = 0;
        static inline value_type Ni = 0;
        value_type val_ = 0;
        static value_type MR(gsh::itype::u64 x) noexcept {
            value_type tmp = (x + (gsh::itype::u64)((value_type) x * Ni) * mod_) >> 32;
            return tmp >= mod_ ? tmp - mod_ : tmp;
        }
    protected:
        MontgomeryModint32_impl() noexcept {}
        static void set_mod(value_type newmod) {
            if (newmod % 2 == 0) throw gsh::Exception("MontgomeryModint32 / Mod must be odd.");
            mod_ = newmod;
            Ni = mod_ * (2u - mod_ * mod_);
            Ni *= (2u - mod_ * Ni);
            Ni *= (2u - mod_ * Ni);
            Ni = -(Ni * (2u - mod_ * Ni));
            R2 = -(gsh::itype::u64) mod_ % mod_;
        }
        value_type val() const noexcept { return MR(val_); }
        static value_type mod() noexcept { return mod_; }
        void assign(gsh::itype::u32 x) noexcept { val_ = MR((gsh::itype::u64)(x % mod_) * R2); }
        void assign(gsh::itype::u64 x) noexcept { val_ = MR((x % mod_) * R2); }
        void rawassign(value_type x) noexcept { val_ = MR((gsh::itype::u64) x * R2); }
        void neg() noexcept { val_ = (val_ == 0 ? 0 : mod_ - val_); }
        void inc() noexcept { val_ = (val_ == mod_ - 1 ? 0 : val_ + 1); }
        void dec() noexcept { val_ = (val_ == 0 ? mod_ - 1 : val_ - 1); }
        void add(modint_type x) noexcept {
            const auto tmp = val_ + x.val_;
            const auto tmp2 = mod_ - val_;
            val_ = tmp2 > x.val_ ? tmp : x.val_ - tmp2;
        }
        void sub(modint_type x) noexcept {
            if (val_ >= x.val_) val_ -= x.val_;
            else val_ = mod_ - (x.val_ - val_);
        }
        void mul(modint_type x) noexcept { val_ = MR((gsh::itype::u64) val_ * x.val_); }
    };

}  // namespace internal

template<gsh::itype::u32 mod_ = 998244353> using StaticModint32 = gsh::internal::ModintTraits<gsh::internal::StaticModint32_impl<mod_>>;
template<gsh::itype::u64 mod_ = 998244353> using StaticModint64 = gsh::internal::ModintTraits<gsh::internal::StaticModint64_impl<mod_>>;
template<gsh::itype::u64 mod_ = 998244353> using StaticModint = std::conditional_t<(mod_ < (1ull << 32)), StaticModint32<mod_>, StaticModint64<mod_>>;
template<int id = 0> using DynamicModint32 = gsh::internal::ModintTraits<gsh::internal::DynamicModint32_impl<id>>;
template<int id = 0> using DynamicModint64 = gsh::internal::ModintTraits<gsh::internal::DynamicModint64_impl<id>>;
template<int id = 0> using MontgomeryModint32 = gsh::internal::ModintTraits<gsh::internal::MontgomeryModint32_impl<id>>;

}  // namespace gsh
