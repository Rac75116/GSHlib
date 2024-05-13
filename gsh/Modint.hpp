#pragma once
#include <type_traits>        // std::conditional_t, std::decay_t, std::is_same_v, std::is_integral_v, std::is_unsigned_v
#include <limits>             // std::numeric_limits
#include <bit>                // std::countr_zero, std::has_single_bit
#include <optional>           // std::optional
#include <gsh/TypeDef.hpp>    // itype
#include <gsh/Exception.hpp>  // Exception

namespace gsh {

namespace internal {

    // https://lpha-z.hatenablog.com/entry/2020/05/24/231500
    template<class T> constexpr T calc_gcd(T x, T y) {
        if (x == 0 || y == 0) [[unlikely]]
            return x | y;
        const itype::i32 n = std::countr_zero(x);
        const itype::i32 m = std::countr_zero(y);
        const itype::i32 l = n < m ? n : m;
        x >>= n;
        y >>= m;
        while (x != y) {
            const T s = y < x ? x - y : y - x;
            const itype::i32 t = std::countr_zero(s);
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
            if (x <= 1) throw Exception("ModintTraits::set_mod / Mod must be at least 2.");
            if (x == mod()) return;
            base_type::set_mod(x);
        }
        constexpr value_type val() const noexcept { return base_type::val(); }
        constexpr static value_type mod() noexcept { return base_type::mod(); }
        template<class U> constexpr modint_type& operator=(U x) noexcept {
            static_assert(std::is_integral_v<U>, "ModintTraits::operator= / Only integer types can be assigned.");
            if constexpr (std::is_unsigned_v<U>) {
                if constexpr (std::is_same_v<U, unsigned long long> || std::is_same_v<U, unsigned long>) base_type::assign(static_cast<itype::u64>(x));
                else base_type::assign(static_cast<itype::u32>(x));
            } else {
                if (x < 0) {
                    if constexpr (std::is_same_v<U, long long> || std::is_same_v<U, long>) base_type::assign(static_cast<itype::u64>(-x));
                    else base_type::assign(static_cast<itype::u32>(-x));
                    base_type::neg();
                } else {
                    if constexpr (std::is_same_v<U, long long> || std::is_same_v<U, long>) base_type::assign(static_cast<itype::u64>(x));
                    else base_type::assign(static_cast<itype::u32>(x));
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
            if (x == 0) throw Exception("ModintTraits::inv / Zero division is not possible.");
            while (true) {
                if (x <= 1) {
                    if (x == 0) [[unlikely]]
                        break;
                    else return modint_type(a);
                }
                b += a * (y / x);
                y %= x;
                if (y <= 1) {
                    if (y == 0) [[unlikely]]
                        break;
                    else return modint_type(mod() - b);
                }
                a += b * (x / y);
                x %= y;
            }
            throw Exception("ModintTraits::inv / Cannot calculate inverse element.");
        }
        constexpr modint_type pow(itype::u64 e) const noexcept {
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
                    res = ((n & 0b111) == 3 || (n & 0b111) == 5) ? -res : res;
                }
                res = ((a & 0b11) == 3 || (n & 0b11) == 3) ? -res : res;
                value_type tmp = n;
                n = a;
                a = tmp;
                a %= n;
            }
            if (n != 1) return 0;
            return res;
        }
        constexpr std::optional<modint_type> sqrt() const noexcept {
            const value_type vl = val(), md = mod();
            if (vl <= 1) return *this;
            auto get_min = [](modint_type x) {
                return x.val() > (mod() >> 1) ? -x : x;
            };
            if ((md & 0b11) == 3) {
                modint_type res = pow((md + 1) >> 2);
                if (res * res != *this) return std::nullopt;
                else return get_min(res);
            } else if ((md & 0b111) == 5) {
                modint_type res = pow((md + 3) >> 3);
                if constexpr (is_staticmod) {
                    constexpr modint_type p = modint_type::raw(2).pow((md - 1) >> 2);
                    res *= p;
                } else if (res * res != *this) res *= modint_type::raw(2).pow((md - 1) >> 2);
                if (res * res != *this) return std::nullopt;
                else return get_min(res);
            } else {
                const itype::u32 S = std::countr_zero(md - 1);
                const value_type Q = (md - 1) >> S;
                if (S < 20) {
                    const modint_type tmp = pow(Q / 2);
                    modint_type R = tmp * (*this), t = R * tmp;
                    if (t.val() == 1) return R;
                    modint_type u = t;
                    for (itype::u32 i = 0; i != S - 1; ++i) u *= u;
                    if (u.val() != 1) return std::nullopt;
                    const modint_type z = [&]() {
                        if (md % 3 == 2) return modint_type::raw(3);
                        if (auto x = md % 5; x == 2 || x == 3) return modint_type::raw(5);
                        if (auto x = md % 7; x == 3 || x == 5 || x == 6) return modint_type::raw(7);
                        if (auto x = md % 11; x == 2 || x == 6 || x == 7 || x == 8 || x == 10) return modint_type(11);
                        if (auto x = md % 13; x == 2 || x == 5 || x == 6 || x == 7 || x == 8 || x == 11) return modint_type(13);
                        for (const itype::u32 x : { 17, 19, 23, 29, 31, 37, 41, 43, 47, 53, 59, 61, 67, 71, 73, 79, 83, 89, 97 }) {
                            if (modint_type(x).legendre() == -1) return modint_type(x);
                        }
                        modint_type z = 101;
                        while (z.legendre() != -1) z += 2;
                        return z;
                    }();
                    itype::u32 M = S;
                    modint_type c = z.pow(Q);
                    do {
                        modint_type U = t * t;
                        itype::u32 i = 1;
                        while (U.val() != 1) U = U * U, ++i;
                        modint_type b = c;
                        for (itype::u32 j = 0, k = M - i - 1; j < k; ++j) b *= b;
                        M = i, c = b * b, t *= c, R *= b;
                    } while (t.val() != 1);
                    return get_min(R);
                } else {
                    if (legendre() != 1) return std::nullopt;
                    modint_type a = 2;
                    while ((a * a - *this).legendre() != -1) ++a;
                    modint_type res1 = modint_type::raw(1), res2, pow1 = a, pow2 = modint_type::raw(1), w = a * a - *this;
                    value_type e = (md + 1) / 2;
                    while (true) {
                        const modint_type tmp2 = pow2 * w;
                        if (e & 1) {
                            const modint_type tmp = res1;
                            res1 = res1 * pow1 + res2 * tmp2;
                            res2 = tmp * pow2 + res2 * pow1;
                        }
                        e >>= 1;
                        if (e == 0) return get_min(res1);
                        const modint_type tmp = pow1;
                        pow1 = pow1 * pow1 + pow2 * tmp2;
                        pow2 *= tmp + tmp;
                    }
                }
            }
        }
    };

    template<itype::u32 mod_> class StaticModint32_impl {
        using value_type = itype::u32;
        using modint_type = StaticModint32_impl;
        value_type val_ = 0;
    protected:
        constexpr StaticModint32_impl() noexcept {}
        constexpr value_type val() const noexcept { return val_; }
        static constexpr value_type mod() noexcept { return mod_; }
        constexpr void assign(itype::u32 x) noexcept { val_ = x % mod_; }
        constexpr void assign(itype::u64 x) noexcept { val_ = x % mod_; }
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
        constexpr void mul(modint_type x) noexcept { val_ = static_cast<itype::u64>(val_) * x.val_ % mod_; }
    };

    template<itype::u64 mod_> class StaticModint64_impl {
        using value_type = itype::u64;
        using modint_type = StaticModint64_impl;
        value_type val_ = 0;
    protected:
        constexpr StaticModint64_impl() noexcept {}
        constexpr value_type val() const noexcept { return val_; }
        static constexpr value_type mod() noexcept { return mod_; }
        constexpr void assign(itype::u32 x) noexcept {
            if constexpr (mod_ < (1ull << 32)) val_ = x % mod_;
            else val_ = x;
        }
        constexpr void assign(itype::u64 x) noexcept { val_ = x % mod_; }
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
        constexpr void mul(modint_type x) noexcept {
            if constexpr (mod_ < (1ull << 63)) {
                constexpr itype::u128 M_ = std::numeric_limits<itype::u128>::max() / mod_ + std::has_single_bit(mod_);
                const value_type a = (((M_ * val_) >> 64) * x.val_) >> 64;
                const value_type b = val_ * x.val_;
                const value_type c = a * mod_;
                const value_type d = b - c;
                const bool e = d < mod_;
                const value_type f = d - mod_;
                val_ = e ? d : f;
            } else {
                constexpr itype::u128 M_ = std::numeric_limits<itype::u128>::max() / mod_ + std::has_single_bit(mod_);
                const value_type a = (((M_ * val_) >> 64) * x.val_) >> 64;
                const itype::u128 b = (itype::u128) val_ * x.val_;
                const itype::u128 c = (itype::u128) a * mod_;
                const itype::u128 d = b - c;
                const bool e = d < mod_;
                const itype::u128 f = d - mod_;
                val_ = e ? d : f;
            }
        }
    };

    template<int id> class DynamicModint32_impl {
        using value_type = itype::u32;
        using modint_type = DynamicModint32_impl;
        static inline value_type mod_ = 0;
        static inline itype::u64 M_ = 0;
        value_type val_ = 0;
    protected:
        DynamicModint32_impl() noexcept {}
        static void set_mod(value_type newmod) noexcept {
            mod_ = newmod;
            M_ = std::numeric_limits<itype::u64>::max() / mod_ + std::has_single_bit(mod_);
        }
        value_type val() const noexcept { return val_; }
        static value_type mod() noexcept { return mod_; }
        void assign(itype::u32 x) noexcept { val_ = x % mod_; }
        void assign(itype::u64 x) noexcept { val_ = x % mod_; }
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
            const itype::u64 a = (itype::u64) val_ * x.val_;
            const itype::u64 b = ((itype::u128) M_ * a) >> 64;
            const itype::u64 c = a - b * mod_;
            val_ = c < mod_ ? c : c - mod_;
            /*
            const value_type a = (((M_ * val_) >> 32) * x.val_) >> 32;
            const value_type b = val_ * x.val_;
            const value_type c = a * mod_;
            const value_type d = b - c;
            const bool e = d < mod_;
            const value_type f = d - mod_;
            val_ = e ? d : f;
            */
        }
    };

    template<int id> class DynamicModint64_impl {
        using value_type = itype::u64;
        using modint_type = DynamicModint64_impl;
        static inline value_type mod_ = 0;
        static inline itype::u128 M_ = 0;
        value_type val_ = 0;
    protected:
        DynamicModint64_impl() noexcept {}
        static void set_mod(value_type newmod) noexcept {
            mod_ = newmod;
            M_ = std::numeric_limits<itype::u128>::max() / mod_ + std::has_single_bit(mod_);
        }
        value_type val() const noexcept { return val_; }
        static value_type mod() noexcept { return mod_; }
        void assign(itype::u32 x) noexcept { val_ = x % mod_; }
        void assign(itype::u64 x) noexcept { val_ = x % mod_; }
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
            const value_type a = (((M_ * val_) >> 64) * x.val_) >> 64;
            const value_type b = val_ * x.val_;
            const value_type c = a * mod_;
            const value_type d = b - c;
            const bool e = d < mod_;
            const value_type f = d - mod_;
            val_ = e ? d : f;
        }
    };

}  // namespace internal

template<itype::u32 mod_ = 998244353> using StaticModint32 = internal::ModintTraits<internal::StaticModint32_impl<mod_>>;
template<itype::u64 mod_ = 998244353> using StaticModint64 = internal::ModintTraits<internal::StaticModint64_impl<mod_>>;
template<itype::u64 mod_ = 998244353> using StaticModint = std::conditional_t<(mod_ < (1ull << 32)), StaticModint32<mod_>, StaticModint64<mod_>>;
template<int id = 0> using DynamicModint32 = internal::ModintTraits<internal::DynamicModint32_impl<id>>;
template<int id = 0> using DynamicModint64 = internal::ModintTraits<internal::DynamicModint64_impl<id>>;
template<int id = 0> using DynamicModint = DynamicModint64<id>;

}  // namespace gsh
