#pragma once
#include <utility>  // std::swap
#include "TypeDef.hpp"
#include "Functional.hpp"
#include "Memory.hpp"
#include "Range.hpp"
#include "Vec.hpp"
#include <immintrin.h>

namespace gsh {

template<class K, class V> using KeyValuePair = std::pair<const K, V>;

namespace internal {
    template<class T> struct DefMappedType {
        using mapped_type = T;
    };
    template<> struct DefMappedType<void> {};
    template<class Key, class Value, class Hasher, class Pred, class Alloc, bool Multi> class HashTable : public ViewInterface<HashTable<Key, Value, Hasher, Pred, Alloc, Multi>, std::conditional_t<std::is_void_v<Value>, Key, KeyValuePair<Key, Value>>>, public DefMappedType<Value> {
        using traits = AllocatorTraits<Alloc>;
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
        using size_type = itype::u32;
        using difference_type = itype::i32;
        using iterator = std::conditional_t<is_set, const value_type*, value_type*>;
        using const_iterator = const value_type*;
        //using insert_return_type = void;
    private:
        constexpr static bool is_hs_tp = requires { typename hasher::is_transparent; };
        constexpr static bool is_eq_tp = requires { typename key_equal::is_transparent; };
        constexpr static itype::u32 min_bucket_size = sizeof(Key) > 64 ? 1 : std::bit_ceil<itype::u32>(64 / sizeof(Key));
        [[no_unique_address]] mutable hasher hash_func;
        [[no_unique_address]] mutable key_equal equal_func;
        template<class K> GSH_INTERNAL_INLINE constexpr itype::u64 calc_hash(K&& k) const noexcept(std::is_nothrow_invocable_v<hasher, std::conditional_t<is_hs_tp, K, key_type>>) {
            if constexpr (is_hs_tp) return Invoke(hash_func, std::forward<K>(k));
            else return Invoke(hash_func, static_cast<key_type>(std::forward<K>(k)));
        }
        template<class T, class U> GSH_INTERNAL_INLINE constexpr bool is_equal(T&& a, U&& b) const noexcept(std::is_nothrow_invocable_v<key_equal, std::conditional_t<is_eq_tp, T, key_type>, std::conditional_t<is_eq_tp, U, key_type>>) {
            if constexpr (is_eq_tp) return Invoke(equal_func, std::forward<T>(a), std::forward<U>(b));
            else return Invoke(equal_func, static_cast<key_type>(std::forward<T>(a)), static_cast<key_type>(std::forward<U>(b)));
        }
        GSH_INTERNAL_INLINE constexpr const key_type& get_key(const value_type& v) const noexcept {
            if constexpr (is_set) return v;
            else return v.first;
        }

        struct node1 {
            itype::u32 index = 0xffffffff;
        };
        struct node2 {
            itype::u32 index = 0xffffffff;
            itype::u32 next = 0xffffffff;
            itype::u32 prev = 0xffffffff;
        };
        using node = std::conditional_t<Multi, node2, node1>;
        using node_allocator = traits::template rebind_alloc<node>;
        using node_alloc_traits = AllocatorTraits<node>;
        [[no_unique_address]] allocator_type data_alloc;
        [[no_unique_address]] node_allocator node_alloc;
        value_type* data;
        node* nodes;
        itype::u32 nodes_cap;
        itype::u32 nodes_size;
        template<class... Args> GSH_INTERNAL_INLINE constexpr void push_node(Args&&... args) {
            if (nodes_size == nodes_cap) [[unlikely]] {
                itype::u32 new_cap = (nodes_cap << 1) + 1;
                value_type* new_data = traits::allocate(data_alloc, new_cap);
                node* new_nodes = node_alloc_traits::allocate(node_alloc, new_cap);
                if (data != nullptr) [[likely]] {
                    for (u32 i = 0; i != nodes_size; ++i) {
                        traits::construct(data_alloc, std::move(data[i]));
                        traits::destroy(data_alloc, data[i]);
                    }
                    for (u32 i = 0; i != nodes_size; ++i) {
                        traits::construct(node_alloc, nodes[i]);
                        traits::destroy(node_alloc, nodes[i]);
                    }
                    traits::deallocate(data_alloc, data);
                    traits::deallocate(data_alloc, nodes);
                }
                data = new_data;
                nodes = new_nodes;
                nodes_cap = new_cap;
            }
            traits::construct(data_alloc, data + nodes_size, std::forward<Args>(args)...);
            node_alloc_traits::construct(node_alloc, nodes + nodes_size);
            ++nodes_size;
        }
        GSH_INTERNAL_INLINE constexpr void pop_node() {
            traits::destroy(data_alloc, data + nodes_size - 1);
            node_alloc_traits::destroy(node_alloc, nodes + nodes_size - 1);
            --nodes_size;
        }

        using signature_allocator = traits::template rebind_alloc<ctype::c8>;
        using index_allocator = traits::template rebind_alloc<itype::u32>;
        using signature_alloc_traits = AllocatorTraits<signature_allocator>;
        using index_alloc_traits = AllocatorTraits<index_allocator>;
        [[no_unique_address]] signature_allocator signature_alloc;
        [[no_unique_address]] index_allocator index_alloc;
        ctype::c8* signatures;
        itype::u32* indexes;
        itype::u32 signatures_size;

        GSH_INTERNAL_INLINE constexpr itype::u32 calc_index(itype::u64 h) const noexcept { return (h >> 32) & (buckets_size - 1); }
        template<class K> constexpr itype::u32 find_loc(const K& k, itype::u64 h, itype::u32 idx) const noexcept(noexcept(is_equal(std::declval<const key_type&>(), k))) {
            itype::u32 mask = 0;
            mask = _mm256_movemask_epi8(_mm256_cmpeq_epi8(_mm256_set1_epi8(static_cast<ctype::c8>(h | (1 << 7))), _mm256_loadu_si256(reinterpret_cast<const __m256i_u*>(signatures + idx))));
            const itype::u32* ptr = indexes + idx;
            while (mask != 0) {
                itype::u32 tmp = std::countr_zero(mask);
                if (is_equal(get_key(data[ptr[tmp]]), k)) [[likely]] {
                    return idx + tmp;
                }
                mask &= mask - 1;
            }
            return 0xffffffff;
        }
        template<class K> constexpr static bool nothrow_loc_computable = noexcept(calc_hash(std::declval<const K&>())) && noexcept(find_loc(std::declval<const K&>(), itype::u64(), itype::u32()));
        template<class K> GSH_INTERNAL_INLINE constexpr iterator find_impl(const K& k) noexcept(nothrow_loc_computable<K>) {
            const itype::u64 h = calc_hash(k);
            const itype::u32 loc = find_loc(k, h, calc_index(h));
            const iterator res = begin() + indexes[(loc == 0xffffffff ? 0 : loc)];
            return loc == 0xffffffff ? end() : res;
        }
        template<class K> GSH_INTERNAL_INLINE constexpr const_iterator find_impl(const K& k) const noexcept(nothrow_loc_computable<K>) {
            const itype::u64 h = calc_hash(k);
            const itype::u32 loc = find_loc(k, h, calc_index(h));
            const const_iterator res = cbegin() + indexes[(loc == 0xffffffff ? 0 : loc)];
            return loc == 0xffffffff ? cend() : res;
        }
        template<class K> GSH_INTERNAL_INLINE constexpr itype::u32 count_impl(const K& k) const noexcept(nothrow_loc_computable<K>) {
            const itype::u64 h = calc_hash(k);
            const itype::u32 loc = find_loc(k, h, calc_index(h));
            if constexpr (!Multi) return loc != 0xffffffff;
            else {
                if (loc == 0xffffffff) return 0;
                itype::u32 res = 0;
                for (itype::u32 i = indexes[loc]; i != 0xffffffff; i = nodes[i].next) ++res;
                return res;
            }
        }
        template<class K> GSH_INTERNAL_INLINE constexpr bool contains_impl(const K& k) const noexcept(nothrow_loc_computable<K>) {
            const itype::u64 h = calc_hash(k);
            return find_loc(k, h, calc_index(h)) != 0xffffffff;
        }
        constexpr bool rehash_impl(itype::u32 n) {
            /*
            buckets.clear();
            buckets.resize(n);
            Vec<itype::u32> cnt(n);
            itype::u32 i = 0, j = data.size();
            while (i + 8 < j) {
                itype::u32 f = 0;
                for (itype::u32 k = 0; k != 8; ++k) {
                    const itype::u64 h = calc_hash(get_key(data[i + k]));
                    const itype::u32 idx = calc_index(h);
                    const itype::u32 tmp = cnt[idx]++;
                    const itype::u32 loc = tmp % bsize;
                    f += tmp == 32;
                    buckets[idx].exist |= 1u << loc;
                    buckets[idx].signature[loc] = h;
                    buckets[idx].index[loc] = i + k;
                    nodes[i + k].index = idx * bsize + loc;
                }
                i += 8;
                if (f != 0) return false;
            }
            itype::u32 f = 0;
            while (i < j) {
                const itype::u64 h = calc_hash(get_key(data[i]));
                const itype::u32 idx = calc_index(h);
                const itype::u32 tmp = cnt[idx]++;
                const itype::u32 loc = tmp % bsize;
                f += tmp == 32;
                buckets[idx].exist |= 1u << loc;
                buckets[idx].signature[loc] = h;
                buckets[idx].index[loc] = i;
                nodes[i].index = idx * bsize + loc;
                ++i;
            }
            return f == 0;
            */
        }
        template<class K> GSH_INTERNAL_INLINE constexpr void insert_impl(K&& key) {
            const itype::u64 h = calc_hash(key);
            itype::u32 idx = calc_index(h);
            itype::u32 loc = find_loc(key, h, idx);
            if (loc == 0xffffffff) {
                itype::u32 exist = 0;
                while (true) {
                    exist = _mm256_movemask_epi8(_mm256_loadu_si256(reinterpret_cast<const __m256i_u*>(signatures + idx)));
                    if (exist != 0xffffffff) [[likely]]
                        break;
                    while (!rehash_impl(signatures_size * 2)) {}
                    idx = calc_index(h);
                    exist = calc_mask();
                }
                loc = idx + std::countr_zero(exist);
                push_node(std::forward<K>(key));
                const itype::u32 n = nodes_size - 1;
                signatures[loc] = h;
                indexes[loc] = n;
                nodes[n].index = loc;
            } else {
                if constexpr (Multi) {
                    push_node(std::forward<K>(key));
                    const itype::u32 n = nodes_size - 1, m = indexes[loc];
                    nodes[n].index = loc;
                    nodes[n].next = m;
                    nodes[m].prev = n;
                    indexes[loc] = n;
                }
            }
        }
    public:
        constexpr HashTable() : hash_func(), equal_func(), data(), nodes(), buckets(min_bucket_size) {}
        constexpr explicit HashTable(size_type n, const hasher& hf = hasher(), const key_equal& eql = key_equal(), const allocator_type& a = allocator_type()) : hash_func(hf), equal_func(eql), data(a), nodes(static_cast<node_alloc>(a)), buckets((n < min_bucket_size ? min_bucket_size : n), static_cast<bucket_alloc>(a)) {}
        template<class InputIterator> constexpr HashTable(InputIterator first, InputIterator last, size_type n = min_bucket_size, const hasher& hf = hasher(), const key_equal& eql = key_equal(), const allocator_type& a = allocator_type()) : hash_func(hf), equal_func(eql), data(a), nodes(static_cast<node_alloc>(a)), buckets((n < min_bucket_size ? min_bucket_size : n), static_cast<bucket_alloc>(a)) { insert(first, last); }
        constexpr HashTable(const HashTable& v) = default;
        constexpr HashTable(HashTable&& rv) = default;
        constexpr explicit HashTable(const allocator_type& a) : hash_func(), equal_func(), data(a), nodes(static_cast<node_alloc>(a)), buckets(min_bucket_size, static_cast<bucket_alloc>(a)) {}
        constexpr HashTable(const HashTable& v, const allocator_type& a) : hash_func(v.hash_func), equal_func(v.equal_func), data(v.data, a), nodes(v.nodes, static_cast<node_alloc>(a)), buckets(v.buckets, static_cast<bucket_alloc>(a)) {}
        constexpr HashTable(HashTable&& v, const allocator_type& a) : hash_func(std::move(v.hash_func)), equal_func(std::move(v.equal_func)), data(std::move(v.data), a), nodes(std::move(v.nodes), static_cast<node_alloc>(a)), buckets(std::move(v.buckets), static_cast<bucket_alloc>(a)) {}
        constexpr HashTable(std::initializer_list<value_type> il, size_type n = min_bucket_size, const hasher& hf = hasher(), const key_equal& eql = key_equal(), const allocator_type& a = allocator_type()) : HashTable(il.begin(), il.end(), n, hf, eql, a) {}
        constexpr HashTable(size_type n, const allocator_type& a) : HashTable(n, hasher(), key_equal(), a) {}
        constexpr HashTable(size_type n, const hasher& hf, const allocator_type& a) : HashTable(n, hf, key_equal(), a) {}
        template<class InputIterator> constexpr HashTable(InputIterator f, InputIterator l, size_type n, const allocator_type& a) : HashTable(f, l, n, hasher(), key_equal(), a) {}
        template<class InputIterator> constexpr HashTable(InputIterator f, InputIterator l, size_type n, const hasher& hf, const allocator_type& a) : HashTable(f, l, hf, key_equal(), a) {}
        constexpr HashTable(std::initializer_list<value_type> il, size_type n, const allocator_type& a) : HashTable(il, n, hasher(), key_equal(), a) {}
        constexpr HashTable(std::initializer_list<value_type> il, size_type n, const hasher& hf, const allocator_type& a) : HashTable(il, n, hf, key_equal(), a) {}
        constexpr ~HashTable() = default;
        constexpr HashTable& operator=(const HashTable& v);
        constexpr HashTable& operator=(HashTable&& x) noexcept(traits::is_always_equal::value && std::is_nothrow_move_assignable<Hasher>::value);
        constexpr HashTable& operator=(std::initializer_list<value_type> il);
        [[nodiscard]] constexpr bool empty() const noexcept { return nodes.empty(); }
        constexpr size_type size() const noexcept { return nodes.size(); }
        constexpr size_type max_size() const noexcept { return nodes.max_size() < data.max_size() ? nodes.max_size() : data.max_size(); }
        constexpr iterator begin() noexcept { return data.begin(); }
        constexpr const_iterator begin() const noexcept { return data.cbegin(); }
        constexpr iterator end() noexcept { return data.end(); }
        constexpr const_iterator end() const noexcept { return data.cend(); }
        constexpr const_iterator cbegin() const noexcept { return data.cbegin(); }
        constexpr const_iterator cend() const noexcept { return data.cend(); }
        constexpr allocator_type get_allocator() const noexcept { return data.get_allocator(); }
        template<class... Args> constexpr auto emplace(Args&&... args) {
            const key_type& key = get_key(data.emplace_back(std::forward<Args>(args)...));
            const itype::u32 n = data.size() - 1;
            if constexpr (!Multi) {
                itype::u32 off = insert_impl(key, n);
                if (off == n) return std::pair{ begin() + off, true };
                else {
                    data.pop_back();
                    return std::pair{ begin() + off, false };
                }
            } else {
                insert_impl(key, n);
                return begin() + n;
            }
        }
        template<class... Args> constexpr iterator emplace_hint([[maybe_unused]] const_iterator position, Args&&... args) { return emplace(std::forward<Args>(args)...).first; }
        constexpr auto insert(const value_type& v) {
            const key_type& key = get_key(v);
            const itype::u32 n = data.size();
            if constexpr (!Multi) {
                itype::u32 off = insert_impl(key, n);
                if (off == n) {
                    data.push_back(v);
                    return std::pair{ begin() + off, true };
                } else return std::pair{ begin() + off, false };
            } else {
                insert_impl(key, n);
                data.push_back(v);
                return begin() + n;
            }
        }
        constexpr auto insert(value_type&& v) {
            const key_type& key = get_key(v);
            const itype::u32 n = data.size();
            if constexpr (!Multi) {
                itype::u32 off = insert_impl(key, n);
                if (off == n) {
                    data.push_back(std::move(v));
                    return std::pair{ begin() + off, true };
                } else return std::pair{ begin() + off, false };
            } else {
                insert_impl(key, n);
                data.push_back(std::move(v));
                return begin() + n;
            }
        }
        constexpr iterator insert([[maybe_unused]] const_iterator position, const value_type& v) {
            if constexpr (Multi) return insert(v);
            else return insert(v).first;
        }
        constexpr iterator insert([[maybe_unused]] const_iterator position, value_type&& v) {
            if constexpr (Multi) return insert(std::move(v));
            else return insert(std::move(v)).first;
        }
        template<class InputIterator> constexpr void insert(InputIterator first, InputIterator last) {
            reserve(data.size() + std::distance(first, last));
            for (; first != last; ++first) insert(*first);
        }
        constexpr void insert(std::initializer_list<value_type> il) { insert(il.begin(), il.end()); }
        //constexpr insert_return_type insert(node_type&& nh);
        //constexpr iterator insert(const_iterator hint, node_type&& nh);
        template<class M>
            requires(!is_set && !Multi)
        constexpr std::pair<iterator, bool> insert_or_assign(const key_type& key, M&& obj) {
            const itype::u32 n = data.size();
            itype::u32 off = insert_impl(key, n);
            if (off == n) {
                data.emplace_back(key, std::forward<M>(obj));
                return std::pair{ begin() + off, true };
            } else {
                data[off].second = std::forward<M>(obj);
                return std::pair{ begin() + off, false };
            }
        }
        template<class M>
            requires(!is_set && !Multi)
        constexpr std::pair<iterator, bool> insert_or_assign(key_type&& key, M&& obj) {
            const itype::u32 n = data.size();
            itype::u32 off = insert_impl(key, n);
            if (off == n) {
                data.emplace_back(std::move(key), std::forward<M>(obj));
                return std::pair{ begin() + off, true };
            } else {
                data[off].second = std::forward<M>(obj);
                return std::pair{ begin() + off, false };
            }
        }
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
        constexpr iterator erase(const_iterator position);
        constexpr size_type erase(const key_type& k);
        constexpr iterator erase(const_iterator first, const_iterator last);
        constexpr void clear() noexcept {
            data.clear();
            nodes.clear();
            buckets.clear();
        }
        constexpr void swap(HashTable& x) noexcept(traits::is_always_equal::value && noexcept(swap(std::declval<Hash&>(), std::declval<Hash&>())) && noexcept(swap(std::declval<Pred&>(), std::declval<Pred&>())));
        //constexpr node_type extract(const_iterator position);
        //constexpr node_type extract(const key_type& x);
        //template<class H2, class P2, bool U2> constexpr void merge(HashTable<Key, H2, P2, Allocator, U2>& source);
        //template<class H2, class P2, bool U2> constexpr void merge(HashTable<Key, H2, P2, Allocator, U2>&& source);
        constexpr hasher hash_function() const { return hash_func; }
        constexpr key_equal key_eq() const { return equal_func; }
        constexpr iterator find(const key_type& x) { return find_impl(x); }
        constexpr const_iterator find(const key_type& x) const { return find_impl(x); }
        template<class K>
            requires is_hs_tp && is_eq_tp
        constexpr iterator find(const K& k) {
            return find_impl(k);
        }
        template<class K>
            requires is_hs_tp && is_eq_tp
        constexpr const_iterator find(const K& k) const {
            return find_impl(k);
        }
        constexpr size_type count(const key_type& x) const { return count_impl(x); }
        template<class K>
            requires is_hs_tp && is_eq_tp
        constexpr size_type count(const K& k) const {
            return count_impl(k);
        }
        constexpr bool contains(const key_type& x) const { return contains_impl(x); }
        template<class K>
            requires is_hs_tp && is_eq_tp
        constexpr bool contains(const K& k) const {
            return contains_impl(k);
        }
        //constexpr std::pair<iterator, iterator> equal_range(const key_type& x);
        //constexpr std::pair<const_iterator, const_iterator> equal_range(const key_type& x) const;
        //template<class K> constexpr std::pair<iterator, iterator> equal_range(const K& k);
        //template<class K> constexpr std::pair<const_iterator, const_iterator> equal_range(const K& k) const;
        //constexpr size_type bucket_count() const noexcept { return buckets.size(); }
        //constexpr size_type max_bucket_count() const noexcept { return buckets.max_size(); }
        //constexpr size_type min_bucket_count() const noexcept { return min_bucket_size; }
        //constexpr size_type bucket_size(size_type n) const { return std::popcount(buckets[n].exist); }
        //template<class K> constexpr size_type bucket(const K& k) const { return calc_index(calc_hash(k)); }
        //constexpr local_iterator begin(size_type n);
        //constexpr const_local_iterator begin(size_type n) const;
        //constexpr local_iterator end(size_type n);
        //constexpr const_local_iterator end(size_type n) const;
        //constexpr const_local_iterator cbegin(size_type n) const;
        //constexpr const_local_iterator cend(size_type n) const;
        //constexpr ftype::f32 load_factor() const noexcept { return static_cast<ftype::f32>(size()) / bucket_count(); }
        //constexpr ftype::f32 max_load_factor() const noexcept { return static_cast<ftype::f32>(bsize); }
        //constexpr void max_load_factor(ftype::f32 z) { rehash(size() / (z < 0.001f ? 0.001f : z)); }
        constexpr void rehash(size_type n) {
            n = n < size() / bsize ? size() / bsize : n;
            n = std::bit_ceil(n);
            if (n <= buckets.size()) return;
            while (!rehash_impl(n)) n *= 2;
        }
        constexpr void reserve(size_type n) {
            data.reserve(n);
            nodes.reserve(n);
            rehash(n * 2 / bsize < min_bucket_size ? min_bucket_size : n * 2 / bsize);
        }
        constexpr auto& operator[](const key_type& key)
            requires(!is_set && !Multi)
        {
            const itype::u32 n = data.size();
            itype::u32 off = insert_impl(key, n);
            if (off == n) data.emplace_back(std::piecewise_construct, std::tuple{ key }, std::tuple{});
            return data[off].second;
        }
        constexpr auto& operator[](key_type&& key)
            requires(!is_set && !Multi)
        {
            const itype::u32 n = data.size();
            itype::u32 off = insert_impl(key, n);
            if (off == n) data.emplace_back(std::piecewise_construct, std::tuple{ std::move(key) }, std::tuple{});
            return data[off].second;
        }
    };
}  // namespace internal

template<class Key, class Hasher = Hash, class Pred = EqualTo, class Alloc = Allocator<Key>> using HashSet = internal::HashTable<Key, void, Hasher, Pred, Alloc, false>;
template<class Key, class Hasher = Hash, class Pred = EqualTo, class Alloc = Allocator<Key>> using HashMultiset = internal::HashTable<Key, void, Hasher, Pred, Alloc, true>;
template<class Key, class Value, class Hasher = Hash, class Pred = EqualTo, class Alloc = Allocator<KeyValuePair<Key, Value>>> using HashMap = internal::HashTable<Key, Value, Hasher, Pred, Alloc, false>;
template<class Key, class Value, class Hasher = Hash, class Pred = EqualTo, class Alloc = Allocator<KeyValuePair<Key, Value>>> using HashMultimap = internal::HashTable<Key, Value, Hasher, Pred, Alloc, true>;

}  // namespace gsh
