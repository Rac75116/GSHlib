#pragma once
#include "Functional.hpp"
#include "Memory.hpp"
#include "Range.hpp"
#include "TypeDef.hpp"
#include "Util.hpp"
#include "Vec.hpp"
#include <immintrin.h>
#include <utility>


namespace gsh {

template<class K, class V> using KeyValuePair = std::pair<const K, V>;

namespace internal {
    template<class T> struct DefMappedType {
        using mapped_type = T;
    };
    template<> struct DefMappedType<void> {};
    template<class Key, class Value, class Hasher, class Pred, class Alloc, bool Multi> class HashTable : public ViewInterface<HashTable<Key, Value, Hasher, Pred, Alloc, Multi>, std::conditional_t<std::is_void_v<Value>, Key, KeyValuePair<Key, Value>>>, public DefMappedType<Value> {
        using traits = std::allocator_traits<Alloc>;
        constexpr static bool is_set = std::is_void_v<Value>;
    public:
        using key_type = Key;
        using value_type = std::conditional_t<is_set, Key, KeyValuePair<Key, Value>>;
        using hasher = Hasher;
        using key_equal = Pred;
        using allocator_type = Alloc;
        using pointer = typename traits::pointer;
        using const_pointer = typename traits::const_pointer;
        using reference = value_type&;
        using const_reference = const value_type&;
        using size_type = u32;
        using difference_type = i32;
        using iterator = std::conditional_t<is_set, const value_type*, value_type*>;
        using const_iterator = const value_type*;
        static_assert(std::is_same_v<typename traits::value_type, value_type>);
    private:
        constexpr static bool is_hs_tp = requires { typename hasher::is_transparent; };
        constexpr static bool is_eq_tp = requires { typename key_equal::is_transparent; };
        constexpr static u32 default_bucket_size = 4;
        using hashed_type = std::invoke_result_t<hasher, key_type>;
        static_assert(std::is_integral_v<hashed_type> && std::is_unsigned_v<hashed_type> && sizeof(hashed_type) <= sizeof(u64));
        using value_allocator_type = allocator_type;
        using value_allocator_traits = std::allocator_traits<value_allocator_type>;
        using hash_allocator_type = typename value_allocator_traits::template rebind_alloc<hashed_type>;
        using hash_allocator_traits = std::allocator_traits<hash_allocator_type>;
        using sig_allocator_type = typename value_allocator_traits::template rebind_alloc<u8>;
        using sig_allocator_traits = std::allocator_traits<sig_allocator_type>;
        using index_allocator_type = typename value_allocator_traits::template rebind_alloc<size_type>;
        using index_allocator_traits = std::allocator_traits<index_allocator_type>;
        [[no_unique_address]] mutable hasher hash_f;
        [[no_unique_address]] mutable key_equal key_eq_f;
        [[no_unique_address]] hash_allocator_type hash_alloc;
        [[no_unique_address]] value_allocator_type value_alloc;
        [[no_unique_address]] sig_allocator_type sig_alloc;
        [[no_unique_address]] index_allocator_type index_alloc;
        value_type* elem;
        hashed_type* hash;
        u8* sig;
        size_type* index;
        size_type elem_size;
        size_type elem_capacity;
        size_type bucket_size;
        size_type bucket_capacity;

        GSH_INTERNAL_INLINE constexpr auto get_key(const value_type& v) const {
            if constexpr (is_set) return v;
            else return v.first;
        }
        GSH_INTERNAL_INLINE constexpr hashed_type calc_hash(const value_type& k) const {
            if constexpr (is_set) return hash_f(k);
            else return hash_f(k.first);
        }
        constexpr void construct_elem_hash(size_type new_elem_cap) {
            value_type* new_elem = value_allocator_traits::allocate(value_alloc, new_elem_cap);
            hashed_type* new_hash = hash_allocator_traits::allocate(hash_alloc, new_elem_cap);
            for (u32 i = 0; i < elem_size; i++) {
                value_allocator_traits::construct(value_alloc, new_elem + i, std::move_if_noexcept(elem[i]));
                hash_allocator_traits::construct(hash_alloc, new_hash + i, std::move_if_noexcept(hash[i]));
            }
            if constexpr (!std::is_trivially_destructible_v<value_type>) {
                for (u32 i = 0; i < elem_size; i++) {
                    value_allocator_traits::destroy(value_alloc, elem + i);
                }
            }
            value_allocator_traits::deallocate(value_alloc, elem, elem_capacity);
            hash_allocator_traits::deallocate(hash_alloc, hash, elem_capacity);
            elem = new_elem;
            hash = new_hash;
            elem_capacity = new_elem_cap;
        }
        constexpr bool construct_sig_index() {
            for (u32 i = 0; i < elem_size; i++) {
                const hashed_type h = hash[i];
                const u8 s = static_cast<u8>(h & 0x7f) | 0x80;
                const size_type b = (h >> 7) & (bucket_size - 1);
                auto mask = _mm256_movemask_epi8(_mm256_loadu_si256(reinterpret_cast<const __m256i_u*>(sig + b)));
                if (mask == 0xffff) {
                    return false;
                }
                const u32 inv = ~mask;
                Assume(inv != 0);
                const auto bit = std::countr_zero(inv);
                sig[b + bit] = s;
                index[b + bit] = i;
            }
            return true;
        }
        constexpr bool rehash_impl() {
            if (bucket_size == bucket_capacity) {
                sig_allocator_traits::deallocate(sig_alloc, sig, bucket_capacity + 31);
                index_allocator_traits::deallocate(index_alloc, index, bucket_capacity + 31);
                const size_type new_bucket_cap = bucket_capacity * 2;
                sig = sig_allocator_traits::allocate(sig_alloc, new_bucket_cap + 31);
                index = index_allocator_traits::allocate(index_alloc, new_bucket_cap + 31);
                for (u32 i = 0; i < new_bucket_cap + 31; i++) {
                    sig[i] = 0;
                }
                bucket_capacity = new_bucket_cap;
                bucket_size = new_bucket_cap;
            } else {
                bucket_size *= 2;
                for (u32 i = 0; i < bucket_size + 31; i++) {
                    sig[i] = 0;
                }
            }
            return construct_sig_index();
        }
        constexpr bool rehash_impl(size_type new_bucket_size) {
            if (new_bucket_size <= bucket_size) return true;
            if (new_bucket_size > bucket_capacity) {
                sig_allocator_traits::deallocate(sig_alloc, sig, bucket_capacity + 31);
                index_allocator_traits::deallocate(index_alloc, index, bucket_capacity + 31);
                const size_type new_bucket_cap = std::bit_ceil(new_bucket_size);
                sig = sig_allocator_traits::allocate(sig_alloc, new_bucket_cap + 31);
                index = index_allocator_traits::allocate(index_alloc, new_bucket_cap + 31);
                for (u32 i = 0; i < new_bucket_cap + 31; i++) {
                    sig[i] = 0;
                }
                bucket_capacity = new_bucket_cap;
                bucket_size = new_bucket_cap;
            } else {
                bucket_size = std::bit_ceil(new_bucket_size);
                for (u32 i = 0; i < bucket_size + 31; i++) {
                    sig[i] = 0;
                }
            }
            return construct_sig_index();
        }
        constexpr void initialize_buckets(size_type n) {
            if (n == 0) return;
            size_type elem_cap = std::bit_ceil(n);
            size_type bucket_cap = elem_cap * 4;
            elem_size = 0;
            elem_capacity = elem_cap;
            bucket_capacity = bucket_cap;
            bucket_size = bucket_cap;
            elem = value_allocator_traits::allocate(value_alloc, elem_cap);
            hash = hash_allocator_traits::allocate(hash_alloc, elem_cap);
            sig = sig_allocator_traits::allocate(sig_alloc, bucket_cap + 31);
            index = index_allocator_traits::allocate(index_alloc, bucket_cap + 31);
            for (u32 i = 0; i < bucket_cap + 31; i++) {
                sig[i] = 0;
            }
        }
        template<class K> constexpr size_type find_impl(const K& k) const {
            const hashed_type h = hash_f(k);
            const size_type b = (h >> 7) & (bucket_size - 1);
            const u8 s = static_cast<u8>(h & 0x7f) | 0x80;
            auto mask = _mm256_movemask_epi8(_mm256_cmpeq_epi8(_mm256_set1_epi8(s), _mm256_loadu_si256(reinterpret_cast<const __m256i_u*>(sig + b))));
            while (mask != 0) {
                const auto bit = std::countr_zero(static_cast<u32>(mask));
                const size_type i = index[b + bit];
                if (key_eq_f(k, get_key(elem[i]))) return i;
                mask &= mask - 1;
            }
            return 0xffffffff;
        }
        template<class K> constexpr std::pair<size_type, bool> insert_impl(const K& v, const hashed_type h) {
            const u8 s = static_cast<u8>(h & 0x7f) | 0x80;
            while (true) {
                const size_type b = (h >> 7) & (bucket_size - 1);
                const auto sigs = _mm256_loadu_si256(reinterpret_cast<const __m256i_u*>(sig + b));
                u32 mask1 = _mm256_movemask_epi8(_mm256_cmpeq_epi8(_mm256_set1_epi8(s), sigs));
                while (mask1 != 0) {
                    const auto bit = std::countr_zero(mask1);
                    const size_type i = index[b + bit];
                    if (key_eq_f(v, get_key(elem[i]))) return { i, false };
                    mask1 &= mask1 - 1;
                }
                u32 mask2 = _mm256_movemask_epi8(sigs);
                if (mask2 == 0xffffffff) {
                    while (!rehash_impl());
                    continue;
                }
                const auto bit = std::countr_zero(~mask2);
                return { b + bit, true };
            }
        }
    public:
        constexpr HashTable() : HashTable(default_bucket_size, hasher(), key_equal(), allocator_type()) {}
        constexpr explicit HashTable(size_type n, const hasher& hf = hasher(), const key_equal& eql = key_equal(), const allocator_type& a = allocator_type()) : hash_f(hf), key_eq_f(eql), hash_alloc(a), value_alloc(a), sig_alloc(a), index_alloc(a) { initialize_buckets(n); }
        template<class InputIterator> constexpr HashTable(InputIterator first, InputIterator last, size_type n = default_bucket_size, const hasher& hf = hasher(), const key_equal& eql = key_equal(), const allocator_type& a = allocator_type()) : HashTable(std::max(n, std::ranges::distance(first, last)), hf, eql, a) { insert(first, last); }
        constexpr HashTable(const HashTable& v) : HashTable(v, allocator_type()) {}
        constexpr HashTable(HashTable&& rv) noexcept : hash_f(std::move(rv.hash_f)), key_eq_f(std::move(rv.key_eq_f)), hash_alloc(std::move(rv.hash_alloc)), value_alloc(std::move(rv.value_alloc)), sig_alloc(std::move(rv.sig_alloc)), index_alloc(std::move(rv.index_alloc)), elem(rv.elem), hash(rv.hash), sig(rv.sig), index(rv.index), elem_size(rv.elem_size), elem_capacity(rv.elem_capacity), bucket_size(rv.bucket_size), bucket_capacity(rv.bucket_capacity) {
            rv.elem = nullptr;
            rv.hash = nullptr;
            rv.sig = nullptr;
            rv.index = nullptr;
            rv.elem_size = 0;
            rv.elem_capacity = 0;
            rv.bucket_size = 0;
            rv.bucket_capacity = 0;
        }
        constexpr explicit HashTable(const allocator_type& a) : HashTable(default_bucket_size, hasher(), key_equal(), a) {}
        constexpr HashTable(const HashTable& v, const allocator_type& a) : HashTable(v.elem_size, v.hash_f, v.key_eq_f, a) { insert(v.begin(), v.end()); }
        constexpr HashTable(HashTable&& v, const allocator_type& a) noexcept;  // TODO
        constexpr HashTable(std::initializer_list<value_type> il, size_type n = default_bucket_size, const hasher& hf = hasher(), const key_equal& eql = key_equal(), const allocator_type& a = allocator_type()) : HashTable(il.begin(), il.end(), n, hf, eql, a) {}
        constexpr HashTable(size_type n, const allocator_type& a) : HashTable(n, hasher(), key_equal(), a) {}
        constexpr HashTable(size_type n, const hasher& hf, const allocator_type& a) : HashTable(n, hf, key_equal(), a) {}
        template<class InputIterator> constexpr HashTable(InputIterator f, InputIterator l, size_type n, const allocator_type& a) : HashTable(f, l, n, hasher(), key_equal(), a) {}
        template<class InputIterator> constexpr HashTable(InputIterator f, InputIterator l, size_type n, const hasher& hf, const allocator_type& a) : HashTable(f, l, n, hf, key_equal(), a) {}
        constexpr HashTable(std::initializer_list<value_type> il, size_type n, const allocator_type& a) : HashTable(il, n, hasher(), key_equal(), a) {}
        constexpr HashTable(std::initializer_list<value_type> il, size_type n, const hasher& hf, const allocator_type& a) : HashTable(il, n, hf, key_equal(), a) {}
        constexpr ~HashTable() noexcept {
            if (elem_capacity != 0) {
                if constexpr (!std::is_trivially_destructible_v<value_type>) {
                    for (u32 i = 0; i < elem_size; i++) {
                        value_allocator_traits::destroy(value_alloc, elem + i);
                    }
                }
                value_allocator_traits::deallocate(value_alloc, elem, elem_capacity);
                hash_allocator_traits::deallocate(hash_alloc, hash, elem_capacity);
            }
            if (bucket_capacity != 0) {
                sig_allocator_traits::deallocate(sig_alloc, sig, bucket_capacity + 31);
                index_allocator_traits::deallocate(index_alloc, index, bucket_capacity + 31);
            }
            elem = nullptr;
            hash = nullptr;
            sig = nullptr;
            index = nullptr;
            elem_size = 0;
            elem_capacity = 0;
            bucket_size = 0;
            bucket_capacity = 0;
        }
        constexpr HashTable& operator=(const HashTable& v);
        constexpr HashTable& operator=(HashTable&& x);
        constexpr HashTable& operator=(std::initializer_list<value_type> il);
        [[nodiscard]] constexpr bool empty() const noexcept { return elem_size == 0; }
        constexpr size_type size() const noexcept { return elem_size; }
        constexpr size_type max_size() const noexcept { return traits::max_size(value_alloc); }
        constexpr iterator begin() noexcept { return elem; }
        constexpr const_iterator begin() const noexcept { return elem; }
        constexpr iterator end() noexcept { return elem + elem_size; }
        constexpr const_iterator end() const noexcept { return elem + elem_size; }
        constexpr const_iterator cbegin() const noexcept { return elem; }
        constexpr const_iterator cend() const noexcept { return elem + elem_size; }
        constexpr allocator_type get_allocator() const noexcept;
        template<class... Args> constexpr std::pair<iterator, bool> emplace(Args&&... args) {
            if (elem_size == elem_capacity) {
                construct_elem_hash(elem_capacity * 2);
            }
            value_allocator_traits::construct(value_alloc, elem + elem_size, std::forward<Args>(args)...);
            const auto& k = get_key(elem[elem_size]);
            const hashed_type h = calc_hash(k);
            const auto [pos, f] = insert_impl(k, h);
            if (f) {
                hash_allocator_traits::construct(hash_alloc, hash + elem_size, h);
                sig[pos] = static_cast<u8>(h & 0x7f) | 0x80;
                index[pos] = elem_size;
                elem_size++;
                return { elem + (elem_size - 1), true };
            } else {
                if constexpr (!std::is_trivially_destructible_v<value_type>) {
                    value_allocator_traits::destroy(value_alloc, elem + elem_size);
                }
                return { elem + pos, false };
            }
        }
        template<class... Args> constexpr iterator emplace_hint([[maybe_unused]] const_iterator position, Args&&... args) { return emplace(std::forward<Args>(args)...).first; }
        constexpr std::pair<iterator, bool> insert(const value_type& v) {
            const auto& k = get_key(v);
            const hashed_type h = calc_hash(k);
            const auto [pos, f] = insert_impl(k, h);
            if (f) {
                if (elem_size == elem_capacity) {
                    construct_elem_hash(elem_capacity * 2);
                }
                value_allocator_traits::construct(value_alloc, elem + elem_size, v);
                hash_allocator_traits::construct(hash_alloc, hash + elem_size, h);
                sig[pos] = static_cast<u8>(h & 0x7f) | 0x80;
                index[pos] = elem_size;
                elem_size++;
                return { elem + (elem_size - 1), true };
            } else {
                return { elem + pos, false };
            }
        }
        constexpr std::pair<iterator, bool> insert(value_type&& v) {
            const auto& k = get_key(v);
            const hashed_type h = calc_hash(k);
            const auto [pos, f] = insert_impl(k, h);
            if (f) {
                if (elem_size == elem_capacity) {
                    construct_elem_hash(elem_capacity * 2);
                }
                value_allocator_traits::construct(value_alloc, elem + elem_size, std::move(v));
                hash_allocator_traits::construct(hash_alloc, hash + elem_size, h);
                sig[pos] = static_cast<u8>(h & 0x7f) | 0x80;
                index[pos] = elem_size;
                elem_size++;
                return { elem + (elem_size - 1), true };
            } else {
                return { elem + pos, false };
            }
        }
        constexpr iterator insert([[maybe_unused]] const_iterator position, const value_type& v) { return insert(v).first; }
        constexpr iterator insert([[maybe_unused]] const_iterator position, value_type&& v) { return insert(std::move(v)).first; }
        template<class InputIterator> constexpr void insert(InputIterator first, InputIterator last);
        constexpr void insert(std::initializer_list<value_type> il) { insert(il.begin(), il.end()); }
        template<class M>
            requires(!is_set && !Multi)
        constexpr std::pair<iterator, bool> insert_or_assign(const key_type& key, M&& obj);
        template<class M>
            requires(!is_set && !Multi)
        constexpr std::pair<iterator, bool> insert_or_assign(key_type&& key, M&& obj);
        template<class M>
            requires(!is_set && !Multi)
        constexpr iterator insert_or_assign([[maybe_unused]] const_iterator hint, const key_type& k, M&& obj) {
            return insert_or_assign(k, std::forward<M>(obj)).first;
        }
        template<class M>
            requires(!is_set && !Multi)
        constexpr iterator insert_or_assign([[maybe_unused]] const_iterator hint, key_type&& k, M&& obj) {
            return insert_or_assign(std::move(k), std::forward<M>(obj)).first;
        }
        constexpr iterator erase(iterator position);
        constexpr iterator erase(const_iterator position)
            requires(!is_set)
        {
            return erase(const_cast<iterator>(position));
        }
        constexpr size_type erase(const key_type& k);
        constexpr iterator erase(const_iterator first, const_iterator last);
        constexpr void clear() noexcept;
        constexpr void swap(HashTable& x) noexcept(traits::is_always_equal::value && noexcept(swap(std::declval<Hash&>(), std::declval<Hash&>())) && noexcept(swap(std::declval<Pred&>(), std::declval<Pred&>())));
        constexpr hasher hash_function() const;
        constexpr key_equal key_eq() const;
        constexpr iterator find(const key_type& x) {
            const size_type i = find_impl(x);
            if (i == 0xffffffff) return end();
            return elem + i;
        }
        constexpr const_iterator find(const key_type& x) const {
            const size_type i = find_impl(x);
            if (i == 0xffffffff) return end();
            return elem + i;
        }
        template<class K>
            requires is_hs_tp && is_eq_tp
        constexpr iterator find(const K& k) {
            const size_type i = find_impl(k);
            if (i == 0xffffffff) return end();
            return elem + i;
        }
        template<class K>
            requires is_hs_tp && is_eq_tp
        constexpr const_iterator find(const K& k) const {
            const size_type i = find_impl(k);
            if (i == 0xffffffff) return end();
            return elem + i;
        }
        constexpr size_type count(const key_type& x) const;
        template<class K>
            requires is_hs_tp && is_eq_tp
        constexpr size_type count(const K& k) const;
        constexpr bool contains(const key_type& x) const {
            const size_type i = find_impl(x);
            return i != 0xffffffff;
        }
        template<class K>
            requires is_hs_tp && is_eq_tp
        constexpr bool contains(const K& k) const {
            const size_type i = find_impl(k);
            return i != 0xffffffff;
        }
        constexpr void rehash(size_type n) {
            if (!rehash_impl(n)) {
                while (!rehash_impl());
            }
        }
        constexpr void reserve(size_type n) {
            if (n > elem_capacity) {
                construct_elem_hash(std::bit_ceil(n));
            }
            const size_type required_bucket_size = n * 4;
            if (!rehash_impl(required_bucket_size)) {
                while (!rehash_impl());
            }
        }
        constexpr auto& operator[](const key_type& key)
            requires(!is_set && !Multi);
        constexpr auto& operator[](key_type&& key)
            requires(!is_set && !Multi);
    };
}  // namespace internal

template<class Key, class Hasher = Hash, class Pred = EqualTo, class Alloc = Allocator<Key>> using HashSet = internal::HashTable<Key, void, Hasher, Pred, Alloc, false>;
template<class Key, class Hasher = Hash, class Pred = EqualTo, class Alloc = Allocator<Key>> using HashMultiset = internal::HashTable<Key, void, Hasher, Pred, Alloc, true>;
template<class Key, class Value, class Hasher = Hash, class Pred = EqualTo, class Alloc = Allocator<KeyValuePair<Key, Value>>> using HashMap = internal::HashTable<Key, Value, Hasher, Pred, Alloc, false>;
template<class Key, class Value, class Hasher = Hash, class Pred = EqualTo, class Alloc = Allocator<KeyValuePair<Key, Value>>> using HashMultimap = internal::HashTable<Key, Value, Hasher, Pred, Alloc, true>;

}  // namespace gsh
