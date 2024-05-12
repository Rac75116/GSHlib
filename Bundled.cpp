#pragma GCC optimize("Ofast")
#pragma GCC optimize("unroll-loops")

#include <ctime>
#include <limits>
#include <cstdio>

namespace gsh {

class ClockTimer {
    std::clock_t start_time;
public:
    using value_type = std::clock_t;
    ClockTimer() { start_time = std::clock(); }
    void restart() { start_time = std::clock(); }
    value_type elapsed() const { return (std::clock() - start_time) * 1000 / CLOCKS_PER_SEC; }
    void print() const { std::printf("gsh::ClockTimer::print / %llu ms\n", static_cast<unsigned long long>(elapsed())); }
};

}  // namespace gsh

#include <bit>
#include <ctime>

namespace gsh {

namespace itype {
    using i8 = char;
    using u8 = unsigned char;
    using i16 = short;
    using u16 = unsigned short;
    using i32 = int;
    using u32 = unsigned;
    using ilong = long;
    using ulong = unsigned long;
    using i64 = long long;
    using u64 = unsigned long long;
    using i128 = __int128_t;
    using u128 = __uint128_t;
    using isize = i32;
    using usize = u32;
}  // namespace itype

namespace ftype {
    using f32 = float;
    using f64 = double;
    using flong = long double;
}  // namespace ftype

class Byte {
    itype::u8 b = 0;
public:
    friend constexpr Byte operator&(Byte l, Byte r) noexcept { return Byte::from_integer(l.b & r.b); }
    constexpr Byte& operator&=(Byte r) noexcept {
        b &= r.b;
        return *this;
    }
    friend constexpr Byte operator|(Byte l, Byte r) noexcept { return Byte::from_integer(l.b | r.b); }
    constexpr Byte& operator|=(Byte r) noexcept {
        b |= r.b;
        return *this;
    }
    friend constexpr Byte operator^(Byte l, Byte r) noexcept { return Byte::from_integer(l.b ^ r.b); }
    constexpr Byte& operator^=(Byte r) noexcept {
        b ^= r.b;
        return *this;
    }
    template<class IntType> friend constexpr Byte operator<<(Byte l, IntType r) noexcept { return Byte::from_integer(l.b << r); }
    template<class IntType> constexpr Byte& operator<<=(IntType r) noexcept {
        b <<= r;
        return *this;
    }
    template<class IntType> friend constexpr Byte operator>>(Byte l, IntType r) noexcept { return Byte::from_integer(l.b >> r); }
    template<class IntType> constexpr Byte& operator>>=(IntType r) noexcept {
        b >>= r;
        return *this;
    }
    friend constexpr Byte operator~(Byte l) noexcept { return Byte::from_integer(~l.b); }
    template<class IntType> constexpr IntType to_integer() noexcept { return static_cast<IntType>(b); }
    template<class IntType> static constexpr Byte from_integer(IntType l) noexcept {
        Byte res;
        res.b = static_cast<itype::u8>(l);
        return res;
    }
};

}  // namespace gsh

namespace gsh {

namespace internal {
    constexpr itype::u64 splitmix(itype::u64 x) {
        itype::u64 z = (x + 0x9e3779b97f4a7c15);
        z = (z ^ (z >> 30)) * 0xbf58476d1ce4e5b9;
        z = (z ^ (z >> 27)) * 0x94d049bb133111eb;
        return z ^ (z >> 31);
    }
}  // namespace internal


class Rand64 {
    itype::u64 s0, s1;
public:
    using result_type = itype::u64;
    static constexpr itype::usize word_size = sizeof(result_type) * 8;
    static constexpr result_type default_seed = 0xcafef00dd15ea5e5;
    constexpr Rand64() : Rand64(default_seed) {}
    constexpr explicit Rand64(result_type value) : s0(value), s1(internal::splitmix(value)) {}
    constexpr result_type operator()() {
        itype::u64 t0 = s0, t1 = s1;
        const itype::u64 res = t0 + t1;
        t1 ^= t0;
        s0 = std::rotr(t0, 9) ^ t1 ^ (t1 << 14);
        s1 = std::rotr(t1, 28);
        return res;
    };
    constexpr void discard(unsigned long long z) {
        for (unsigned long long i = 0; i < z; ++i) operator()();
    }
    static constexpr result_type max() { return 18446744073709551615u; }
    static constexpr result_type min() { return 0; }
    constexpr void seed(result_type value = default_seed) { s0 = value, s1 = internal::splitmix(value); }
    friend constexpr bool operator==(Rand64 x, Rand64 y) { return x.s0 == y.s0 && x.s1 == y.s1; }
};


class Rand32 {
    itype::u64 val;
public:
    using result_type = itype::u32;
    static constexpr itype::usize word_size = sizeof(result_type) * 8;
    static constexpr result_type default_seed = 0xcafef00d;
    constexpr Rand32() : Rand32(default_seed) {}
    constexpr explicit Rand32(result_type value) : val(internal::splitmix((itype::u64) value << 32 | value)) {}
    constexpr result_type operator()() {
        itype::u64 x = val;
        const itype::i32 count = x >> 61;
        val = x * 0xcafef00dd15ea5e5;
        x ^= x >> 22;
        return x >> (22 + count);
    };
    constexpr void discard(unsigned long long z) {
        for (unsigned long long i = 0; i < z; ++i) val *= 0xcafef00dd15ea5e5;
    }
    static constexpr result_type max() { return 4294967295u; }
    static constexpr result_type min() { return 0; }
    constexpr void seed(result_type value = default_seed) { val = internal::splitmix((itype::u64) value << 32 | value); }
    friend constexpr bool operator==(Rand32 x, Rand32 y) { return x.val == y.val; }
};


class RandomDevice {
    static inline Rand64 engine{ static_cast<itype::u64>(std::time(nullptr)) };
public:
    using result_type = itype::u32;
    RandomDevice() {}
    RandomDevice(const RandomDevice&) = delete;
    ~RandomDevice() = default;
    void operator=(const RandomDevice&) = delete;
    ftype::f64 entropy() const noexcept { return 0.0; }
    static constexpr result_type max() { return 4294967295u; }
    static constexpr result_type min() { return 0; }
    result_type operator()() {
        itype::u64 a = internal::splitmix(static_cast<itype::u64>(std::time(nullptr)));
        itype::u64 b = internal::splitmix(static_cast<itype::u64>(std::clock()));
        return static_cast<result_type>(engine() ^ a ^ b);
    }
};


template<class URBG> constexpr itype::u32 Uniform32(URBG& g, itype::u32 max) {
    return (static_cast<itype::u64>(g() & 4294967295u) * max) >> 32;
}

template<class URBG> constexpr itype::u32 Uniform32(URBG& g, itype::u32 min, itype::u32 max) {
    return static_cast<itype::u32>((static_cast<itype::u64>(g() & 4294967295u) * (max - min)) >> 32) + min;
}

template<class URBG> constexpr itype::u64 Uniform64(URBG& g, itype::u64 max) {
    return (static_cast<itype::u128>(g()) * max) >> 64;
}

template<class URBG> constexpr itype::u64 Uniform64(URBG& g, itype::u64 min, itype::u64 max) {
    return static_cast<itype::u64>((static_cast<itype::u128>(g()) * (max - min)) >> 64) + min;
}


template<class URBG> constexpr ftype::f32 Canocicaled32(URBG& g) {
    return std::bit_cast<ftype::f32>((127u << 23) | (static_cast<itype::u32>(g()) & 0x7fffff)) - 1.0f;
}
template<class URBG> constexpr ftype::f32 Uniformf32(URBG& g, ftype::f32 max) {
    return canocicaled32(g) * max;
}
template<class URBG> constexpr ftype::f32 Uniformf32(URBG& g, ftype::f32 min, ftype::f32 max) {
    return canocicaled32(g) * (max - min) + min;
}
template<class URBG> constexpr ftype::f64 Canocicaled64(URBG& g) {
    return std::bit_cast<ftype::f64>((1023ull << 52) | (g() & 0xfffffffffffffull)) - 1.0;
}
template<class URBG> constexpr ftype::f64 Uniformf64(URBG& g, ftype::f64 max) {
    return canocicaled64(g) * max;
}
template<class URBG> constexpr ftype::f64 Uniformf64(URBG& g, ftype::f64 min, ftype::f64 max) {
    return canocicaled64(g) * (max - min) + min;
}

}  // namespace gsh

#include <type_traits>
#include <bit>
#include <initializer_list>

#include <type_traits>
#include <limits>
#include <bit>
#include <optional>


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

}  // namespace gsh


namespace gsh {

namespace internal {


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
            const value_type a = (((M_ * val_) >> 32) * x.val_) >> 32;
            const value_type b = val_ * x.val_;
            const value_type c = a * mod_;
            const value_type d = b - c;
            const bool e = d < mod_;
            const value_type f = d - mod_;
            val_ = e ? d : f;
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


namespace gsh {


constexpr itype::u32 IntSqrt(const itype::u32 x) noexcept {
    if (x < 2) return x;
    const itype::u32 log2x = std::bit_width(x) - 1;
    const itype::u32 log2y = log2x / 2u;
    itype::u32 y = 1 << log2y;
    itype::u32 y_squared = 1 << (2u * log2y);
    itype::i32 sqr_diff = x - y_squared;
    y += (sqr_diff / 3u) >> log2y;
    y_squared = y * y;
    sqr_diff = x - y_squared;
    y += sqr_diff / (2 * y);
    y_squared = y * y;
    sqr_diff = x - y_squared;
    if (sqr_diff >= 0) return y;
    y -= (-sqr_diff / (2 * y)) + 1;
    y_squared = y * y;
    sqr_diff = x - y_squared;
    y -= (sqr_diff < 0);
    return y;
}


template<class T, class U> constexpr auto GCD(T x, U y) {
    static_assert(!std::is_same_v<T, bool> && !std::is_same_v<U, bool> && std::is_integral_v<T> && std::is_integral_v<U>, "GCD / The input must be an integral type.");
    if constexpr (std::is_same_v<T, U>) {
        if constexpr (std::is_unsigned_v<T>) {
            return internal::calc_gcd(x, y);
        } else {
            return static_cast<T>(GCD<std::make_unsigned_t<T>, std::make_unsigned<T>>((x < 0 ? -x : x), (y < 0 ? -y : y)));
        }
    } else {
        return GCD<std::common_type_t<T, U>, std::common_type_t<T, U>>(x, y);
    }
}

template<class T, class... Args> auto GCD(T x, Args... y) {
    return GCD(x, GCD(y...));
}

template<class T, class U> auto LCM(T x, U y) {
    return static_cast<std::common_type_t<T, U>>(x < 0 ? -x : x) / GCD(x, y) * static_cast<std::common_type_t<T, U>>(y < 0 ? -y : y);
}

template<class T, class... Args> auto LCM(T x, Args... y) {
    return LCM(x, LCM(y...));
}

namespace internal {

    bool isPrime32(const itype::u32 x) {
        if (x % 2 == 0 || x % 3 == 0 || x % 5 == 0 || x % 7 == 0 || x % 11 == 0 || x % 13 == 0 || x % 17 == 0 || x % 19 == 0 || x % 23 == 0 || x % 29 == 0 || x % 31 == 0 || x % 37 == 0 || x % 41 == 0 || x % 43 == 0) return x <= 43 && (x == 2 || x == 3 || x == 5 || x == 7 || x == 11 || x == 13 || x == 17 || x == 19 || x == 23 || x == 29 || x == 31 || x == 37 || x == 41 || x == 43);
        if (x < 47 * 47) return (x > 1);
        const static itype::u16 bases[] = { 1216, 1836,  8885,  4564, 10978, 5228, 15613, 13941, 1553, 173,   3615, 3144, 10065, 9259,  233,  2362, 6244,  6431, 10863, 5920, 6408, 6841, 22124, 2290,  45597, 6935,  4835, 7652, 1051, 445,  5807, 842,  1534, 22140, 1282, 1733, 347,   6311,  14081, 11157, 186,  703,  9862,  15490, 1720, 17816, 10433, 49185, 2535, 9158,  2143,  2840,  664,  29074, 24924, 1035, 41482, 1065,  10189, 8417,  130,  4551,  5159,  48886,
                                            786,  1938,  1013,  2139, 7171,  2143, 16873, 188,   5555, 42007, 1045, 3891, 2853,  23642, 148,  3585, 3027,  280,  3101,  9918, 6452, 2716, 855,   990,   1925,  13557, 1063, 6916, 4965, 4380, 587,  3214, 1808, 1036,  6356, 8191, 6783,  14424, 6929,  1002,  840,  422,  44215, 7753,  5799, 3415,  231,   2013,  8895, 2081,  883,   3855,  5577, 876,   3574,  1925, 1192,  865,   7376,  12254, 5952, 2516,  20463, 186,
                                            5411, 35353, 50898, 1084, 2127,  4305, 115,   7821,  1265, 16169, 1705, 1857, 24938, 220,   3650, 1057, 482,   1690, 2718,  4309, 7496, 1515, 7972,  3763,  10954, 2817,  3430, 1423, 714,  6734, 328,  2581, 2580, 10047, 2797, 155,  5951,  3817,  54850, 2173,  1318, 246,  1807,  2958,  2697, 337,   4871,  2439,  736,  37112, 1226,  527,   7531, 5418,  7242,  2421, 16135, 7015,  8432,  2605,  5638, 5161,  11515, 14949,
                                            748,  5003,  9048,  4679, 1915,  7652, 9657,  660,   3054, 15469, 2910, 775,  14106, 1749,  136,  2673, 61814, 5633, 1244,  2567, 4989, 1637, 1273,  11423, 7974,  7509,  6061, 531,  6608, 1088, 1627, 160,  6416, 11350, 921,  306,  18117, 1238,  463,   1722,  996,  3866, 6576,  6055,  130,  24080, 7331,  3922,  8632, 2706,  24108, 32374, 4237, 15302, 287,   2296, 1220,  20922, 3350,  2089,  562,  11745, 163,   11951 };
        using mint = DynamicModint32<-1>;
        mint::set_mod(x);
        const itype::u32 h = x * 0xad625b89;
        itype::u32 d = x - 1;
        mint cur = bases[h >> 24];
        itype::i32 s = std::countr_zero(d);
        d >>= s;
        cur = cur.pow(d);
        if (cur.val() == 1) return true;
        while (--s) {
            if (cur.val() == x - 1) return true;
            cur *= cur;
        }
        return cur.val() == x - 1;
    }

    bool isPrime64(const itype::u64 x) {
        if (x % 2 == 0 || x % 3 == 0 || x % 5 == 0 || x % 7 == 0 || x % 11 == 0 || x % 13 == 0 || x % 17 == 0 || x % 19 == 0 || x % 23 == 0 || x % 29 == 0 || x % 31 == 0 || x % 37 == 0 || x % 41 == 0 || x % 43 == 0) return false;
        using mint = DynamicModint64<-1>;
        mint::set_mod(x);
        itype::u64 d = x - 1;
        const itype::i32 s = std::countr_zero(d);
        d >>= s;
        auto test = [&](itype::u64 a) -> bool {
            mint cur = mint(a).pow(d);
            if (cur.val() <= 1) return true;
            itype::i32 i = s;
            while (--i) {
                if (cur.val() == x - 1) return true;
                cur *= cur;
            }
            return cur.val() == x - 1;
        };
        if (x < 585226005592931977ull) {
            if (x < 7999252175582851ull) {
                if (x < 350269456337ull) return test(4230279247111683200ull) && test(14694767155120705706ull) && test(16641139526367750375ull);
                else if (x < 55245642489451ull) return test(2ull) && test(141889084524735ull) && test(1199124725622454117ull) && test(11096072698276303650ull);
                else return test(2ull) && test(4130806001517ull) && test(149795463772692060ull) && test(186635894390467037ull) && test(3967304179347715805ull);
            } else return test(2ull) && test(123635709730000ull) && test(9233062284813009ull) && test(43835965440333360ull) && test(761179012939631437ull) && test(1263739024124850375ull);
        } else return test(2ull) && test(325ull) && test(9375ull) && test(28178ull) && test(450775ull) && test(9780504ull) && test(1795265022ull);
    }

}  // namespace internal


bool isPrime(const itype::u64 x) {
    if (x < 2147483648) return internal::isPrime32(x);
    else return internal::isPrime64(x);
}

}  // namespace gsh

#include <iostream>
#include <cassert>
#include <numeric>
#include <vector>

using namespace gsh::itype;
using namespace gsh::ftype;
int main() {
    gsh::Rand32 engine;
    u32 r = 0;
    gsh::ClockTimer t;
    for (u32 i = 0; i != 10000000; ++i) {
        u32 a = engine();
        u32 b = engine();
        r += gsh::GCD(a, b);
    }
    t.print();
    std::cout << r << std::endl;
}
