#pragma once
#include <type_traits>        // std::conditional_t, std::decay_t, std::is_same_v, std::is_integral_v, std::is_unsigned_v
#include <limits>             // std::numeric_limits
#include <bit>                // std::countr_zero, std::has_single_bit
#include <gsh/TypeDef.hpp>    // gsh::itype
#include <gsh/Exception.hpp>  // gsh::Exception
#include <gsh/Option.hpp>     // gsh::Option

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
            if (m != n) __builtin_unreachable();
            const T s = y < x ? b : a;
            const T t = x < y ? x : y;
            x = s >> m;
            y = t;
        }
        return x << l;
    }

    template<class T> class ModintImpl : public T {
        using base_type = T;
    public:
        using value_type = std::decay_t<decltype(base_type::mod())>;
        using modint_type = ModintImpl;
        constexpr static bool is_static_mod = !requires { base_type::set_mod(0); };
        constexpr ModintImpl() noexcept : T() {}
        template<class U> constexpr ModintImpl(U x) noexcept { operator=(x); }
        constexpr explicit operator value_type() const noexcept { return val(); }
        constexpr static void set_mod(value_type x) {
            static_assert(!is_static_mod, "gsh::internal::ModintImpl::set_mod / Mod must be dynamic.");
            if (x <= 1) throw Exception("gsh::internal::ModintImpl::set_mod / Mod must be at least 2.");
            if (x == mod()) return;
            base_type::set_mod(x);
        }
        constexpr value_type val() const noexcept { return base_type::val(); }
        constexpr static value_type mod() noexcept { return base_type::mod(); }
        template<class U> constexpr modint_type& operator=(U x) noexcept {
            static_assert(std::is_integral_v<U>, "gsh::internal::ModintImpl::operator= / Only integer types can be assigned.");
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
        constexpr Option<modint_type> inv() const noexcept {
            value_type a = 1, b = 0, x = val(), y = mod();
            if (x == 0) return Null;
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
            return Null;
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
            auto iv = x.inv();
            if (!iv) throw gsh::Exception("gsh::internal::ModintImpl::operator/= / Cannot calculate inverse.");
            operator*=(*iv);
            return *this;
        }
        friend constexpr modint_type operator+(modint_type l, modint_type r) noexcept { return modint_type(l) += r; }
        friend constexpr modint_type operator-(modint_type l, modint_type r) noexcept { return modint_type(l) -= r; }
        friend constexpr modint_type operator*(modint_type l, modint_type r) noexcept { return modint_type(l) *= r; }
        friend constexpr modint_type operator/(modint_type l, modint_type r) { return modint_type(l) /= r; }
        friend constexpr bool operator==(modint_type l, modint_type r) noexcept { return l.val() == r.val(); }
        friend constexpr bool operator!=(modint_type l, modint_type r) noexcept { return l.val() != r.val(); }
    };

    template<class T> class StaticModintImpl {
        [[no_unique_address]] T mint{};
        typename T::value_type val_ = 0;
    protected:
        using value_type = typename T::value_type;
        constexpr StaticModintImpl() noexcept {}
        constexpr value_type val() const noexcept { return mint.val(val_); }
        constexpr static value_type mod() noexcept { return T::get_mod(); }
        constexpr void assign(itype::u32 x) noexcept { val_ = mint.build(x); }
        constexpr void assign(itype::u64 x) noexcept { val_ = mint.build(x); }
        constexpr void rawassign(value_type x) noexcept { val_ = mint.raw(x); }
        constexpr void neg() noexcept { val_ = mint.neg(val_); }
        constexpr void inc() noexcept { val_ = mint.inc(val_); }
        constexpr void dec() noexcept { val_ = mint.dec(val_); }
        constexpr void add(StaticModintImpl x) noexcept { val_ = mint.add(val_, x.val_); }
        constexpr void sub(StaticModintImpl x) noexcept { val_ = mint.sub(val_, x.val_); }
        constexpr void mul(StaticModintImpl x) noexcept { val_ = mint.mul(val_, x.val_); }
    };

    template<itype::u32 mod_> class StaticModint32Impl {
    public:
        using value_type = itype::u32;
        constexpr StaticModint32Impl() noexcept {}
        constexpr value_type val(value_type x) const noexcept { return x; }
        constexpr value_type mod() const noexcept { return mod_; }
        constexpr static value_type get_mod() noexcept { return mod_; }
        constexpr value_type build(itype::u32 x) const noexcept { return x % mod_; }
        constexpr value_type build(itype::u64 x) const noexcept { return x % mod_; }
        constexpr value_type raw(value_type x) const noexcept { return x; }
        constexpr value_type neg(value_type x) const noexcept { return x == 0 ? 0 : mod_ - x; }
        constexpr value_type inc(value_type x) const noexcept { return x == mod_ - 1 ? 0 : x + 1; }
        constexpr value_type dec(value_type x) const noexcept { return x == 0 ? mod_ - 1 : x - 1; }
        constexpr value_type add(value_type x, value_type y) const noexcept { return mod_ - x > y ? x + y : y - (mod_ - x); }
        constexpr value_type sub(value_type x, value_type y) const noexcept { return x >= y ? x - y : mod_ - (y - x); }
        constexpr value_type mul(value_type x, value_type y) noexcept { return static_cast<itype::u64>(x) * y % mod_; }
    };

    template<itype::u64 mod_> class StaticModint64Impl {
    public:
        using value_type = itype::u64;
        constexpr StaticModint64Impl() noexcept {}
        constexpr value_type val(value_type x) const noexcept { return x; }
        constexpr value_type mod() const noexcept { return mod_; }
        constexpr static value_type get_mod() noexcept { return mod_; }
        constexpr value_type build(itype::u32 x) const noexcept { return x % mod_; }
        constexpr value_type build(itype::u64 x) const noexcept { return x % mod_; }
        constexpr value_type raw(value_type x) const noexcept { return x; }
        constexpr value_type neg(value_type x) const noexcept { return x == 0 ? 0 : mod_ - x; }
        constexpr value_type inc(value_type x) const noexcept { return x == mod_ - 1 ? 0 : x + 1; }
        constexpr value_type dec(value_type x) const noexcept { return x == 0 ? mod_ - 1 : x - 1; }
        constexpr value_type add(value_type x, value_type y) const noexcept { return mod_ - x > y ? x + y : y - (mod_ - x); }
        constexpr value_type sub(value_type x, value_type y) const noexcept { return x >= y ? x - y : mod_ - (y - x); }
        constexpr value_type mul(value_type x, value_type y) const noexcept {
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

    template<class T, itype::u32 id> class DynamicModintImpl {
        static inline T mint{};
        typename T::value_type val_;
    public:
        using value_type = typename T::value_type;
        DynamicModintImpl() noexcept {}
        static void set_mod(value_type newmod) noexcept { mint.set_mod(newmod); }
        value_type val() const noexcept { return mint.val(val_); }
        static value_type mod() noexcept { return mint.mod(); }
        void assign(itype::u32 x) noexcept { val_ = mint.build(x); }
        void assign(itype::u64 x) noexcept { val_ = mint.build(x); }
        void rawassign(value_type x) noexcept { val_ = mint.raw(x); }
        void neg() noexcept { val_ = mint.neg(val_); }
        void inc() noexcept { val_ = mint.inc(val_); }
        void dec() noexcept { val_ = mint.dec(val_); }
        void add(DynamicModintImpl x) noexcept { val_ = mint.add(val_, x.val_); }
        void sub(DynamicModintImpl x) noexcept { val_ = mint.sub(val_, x.val_); }
        void mul(DynamicModintImpl x) noexcept { val_ = mint.mul(val_, x.val_); }
    };

    class DynamicModint32Impl {
        itype::u32 mod_ = 0;
        itype::u64 M_ = 0;
    public:
        using value_type = itype::u32;
        constexpr DynamicModint32Impl() noexcept {}
        constexpr void set_mod(value_type newmod) noexcept {
            mod_ = newmod;
            M_ = std::numeric_limits<itype::u64>::max() / mod_ + std::has_single_bit(mod_);
        }
        constexpr value_type val(value_type x) const noexcept { return x; }
        constexpr value_type mod() const noexcept { return mod_; }
        constexpr value_type build(itype::u32 x) const noexcept { return x % mod_; }
        constexpr value_type build(itype::u64 x) const noexcept { return x % mod_; }
        constexpr value_type raw(value_type x) const noexcept { return x; }
        constexpr value_type neg(value_type x) const noexcept { return x == 0 ? 0 : mod_ - x; }
        constexpr value_type inc(value_type x) const noexcept { return x == mod_ - 1 ? 0 : x + 1; }
        constexpr value_type dec(value_type x) const noexcept { return x == 0 ? mod_ - 1 : x - 1; }
        constexpr value_type add(value_type x, value_type y) const noexcept { return mod_ - x > y ? x + y : y - (mod_ - x); }
        constexpr value_type sub(value_type x, value_type y) const noexcept { return x >= y ? x - y : mod_ - (y - x); }
        constexpr value_type mul(value_type x, value_type y) const noexcept {
            const itype::u64 a = static_cast<itype::u64>(x) * y;
            const itype::u64 b = (static_cast<itype::u128>(M_) * a) >> 64;
            const itype::u64 c = a - b * mod_;
            return c < mod_ ? c : c - mod_;
        }
    };

    class DynamicModint64Impl {
        itype::u64 mod_ = 0;
        itype::u128 M_ = 0;
    public:
        using value_type = itype::u64;
        constexpr DynamicModint64Impl() noexcept {}
        constexpr void set_mod(value_type newmod) noexcept {
            mod_ = newmod;
            M_ = std::numeric_limits<itype::u128>::max() / mod_ + std::has_single_bit(mod_);
        }
        constexpr value_type val(value_type x) const noexcept { return x; }
        constexpr value_type mod() const noexcept { return mod_; }
        constexpr value_type build(itype::u32 x) const noexcept { return x % mod_; }
        constexpr value_type build(itype::u64 x) const noexcept { return x % mod_; }
        constexpr value_type raw(value_type x) const noexcept { return x; }
        constexpr value_type neg(value_type x) const noexcept { return x == 0 ? 0 : mod_ - x; }
        constexpr value_type inc(value_type x) const noexcept { return x == mod_ - 1 ? 0 : x + 1; }
        constexpr value_type dec(value_type x) const noexcept { return x == 0 ? mod_ - 1 : x - 1; }
        constexpr value_type add(value_type x, value_type y) const noexcept { return mod_ - x > y ? x + y : y - (mod_ - x); }
        constexpr value_type sub(value_type x, value_type y) const noexcept { return x >= y ? x - y : mod_ - (y - x); }
        constexpr value_type mul(value_type x, value_type y) const noexcept {
            const itype::u64 a = (((M_ * x) >> 64) * y) >> 64;
            const itype::u64 b = x * y;
            const itype::u64 c = a * mod_;
            const itype::u64 d = b - c;
            const bool e = d < mod_;
            const itype::u64 f = d - mod_;
            return e ? d : f;
        }
    };

    class MontgomeryModint64Impl {
        itype::u64 mod_ = 0, R2 = 0, ninv = 0;
        __attribute__((always_inline)) constexpr itype::u64 reduce(const itype::u64 t) const noexcept {
            const itype::u64 res = (t + static_cast<itype::u128>(t * ninv) * mod_) >> 64;
            return res < mod_ ? res : res - mod_;
        }
        __attribute__((always_inline)) constexpr itype::u64 reduce(const itype::u128 t) const noexcept {
            //const itype::u64 res = (static_cast<itype::u128>(static_cast<itype::u64>(t) * ninv) * mod_ + t) >> 64;
            //return res < mod_ ? res : res - mod_;
            const itype::u64 a = t, b = t >> 64;
            const itype::u64 c = a * ninv;
            const itype::u64 d = (static_cast<itype::u128>(c) * mod_) >> 64;
            return b + d + (a != 0);
        }
    public:
        using value_type = itype::u64;
        constexpr MontgomeryModint64Impl() noexcept {}
        constexpr void set_mod(value_type newmod) {
            if (newmod % 2 == 0) [[unlikely]]
                throw Exception("gsh::internal::MontgomeryModint64Impl::set_mod / It is not allowed to set the modulo to an even number.");
            mod_ = newmod;
            R2 = -static_cast<itype::u128>(mod_) % mod_;
            ninv = mod_;
            for (itype::u32 i = 0; i != 5; ++i) ninv *= 2 - mod_ * ninv;
            ninv = -ninv;
        }
        constexpr value_type val(value_type x) const noexcept { return reduce(x); }
        constexpr value_type mod() const noexcept { return mod_; }
        constexpr value_type build(itype::u32 x) const noexcept { return reduce(static_cast<itype::u128>(x % mod_) * R2); }
        constexpr value_type build(itype::u64 x) const noexcept { return reduce(static_cast<itype::u128>(x % mod_) * R2); }
        constexpr value_type raw(value_type x) const noexcept { return reduce(static_cast<itype::u128>(x) * R2); }
        constexpr value_type neg(value_type x) const noexcept { return x == 0 ? 0 : 2 * mod_ - x; }
        constexpr value_type inc(value_type x) const noexcept { return x + 1 == 2 * mod_ ? 0 : x + 1; }
        constexpr value_type dec(value_type x) const noexcept { return x == 0 ? 2 * mod_ - 1 : x - 1; }
        constexpr value_type add(value_type x, value_type y) const noexcept { return 2 * mod_ - x > y ? x + y : y - (2 * mod_ - x); }
        constexpr value_type sub(value_type x, value_type y) const noexcept { return x >= y ? x - y : 2 * mod_ - (y - x); }
        constexpr value_type mul(value_type x, value_type y) const noexcept { return reduce(static_cast<itype::u128>(x) * y); }
    };

}  // namespace internal

template<itype::u32 mod_ = 998244353> using StaticModint32 = internal::ModintImpl<internal::StaticModintImpl<internal::StaticModint32Impl<mod_>>>;
template<itype::u64 mod_ = 998244353> using StaticModint64 = internal::ModintImpl<internal::StaticModintImpl<internal::StaticModint64Impl<mod_>>>;
template<itype::u64 mod_ = 998244353> using StaticModint = std::conditional_t<(mod_ < (1ull << 32)), StaticModint32<mod_>, StaticModint64<mod_>>;
template<itype::u32 id = 0> using DynamicModint32 = internal::ModintImpl<internal::DynamicModintImpl<internal::DynamicModint32Impl, id>>;
template<itype::u32 id = 0> using DynamicModint64 = internal::ModintImpl<internal::DynamicModintImpl<internal::DynamicModint64Impl, id>>;
template<itype::u32 id = 0> using MontgomeryModint64 = internal::ModintImpl<internal::DynamicModintImpl<internal::MontgomeryModint64Impl, id>>;

}  // namespace gsh
