#pragma once
#include <utility>  // std::swap
#include <gsh/Functional.hpp>
#include <gsh/Memory.hpp>
#include <gsh/Range.hpp>

namespace gsh {

template<class Key, class Hasher = Hash, class Pred = EqualTo, class Alloc = Allocator<Key>> class HashSet : public ViewInterface<HashSet<Key, Hasher, Pred, Alloc>, Key> {
    using traits = AllocatorTraits<Alloc>;
    using std::swap;
public:
    using key_type = Key;
    using value_type = Key;
    using hasher = Hasher;
    using key_equal = Pred;
    using allocator_type = Alloc;
    using pointer = typename traits::pointer;
    using const_pointer = typename traits::const_pointer;
    using reference = value_type&;
    using const_reference = const value_type&;
    using size_type = itype::u32;
    using difference_type = itype::i32;
    using iterator = value_type*;  //TODO
    using const_iterator = iterator;
    //using insert_return_type = void;
private:
    constexpr static bool is_tp_hash = requires { typename hasher::is_transparent; };
    constexpr static bool is_tp_eq = requires { typename key_equal::is_transparent; };
    constexpr static itype::u32 min_bucket_size = sizeof(Key) > 64 ? 1 : 64 / sizeof(Key);
    [[no_unique_address]] hasher hash_func;
    [[no_unique_address]] key_equal equal_func;
    using alloc = typename traits::template rebind_alloc<char>;
    using traits = typename traits::template rebind_traits<char>;
public:
    constexpr HashSet() : hash_func(), equal_func() {
        //TODO
    }
    constexpr explicit HashSet(size_type n, const hasher& hf = hasher(), const key_equal& eql = key_equal(), const allocator_type& a = allocator_type()) : hash_func(hf), equal_func(eql) {
        //TODO
    }
    template<class InputIterator> constexpr HashSet(InputIterator first, InputIterator last, size_type n = min_bucket_size, const hasher& hf = hasher(), const key_equal& eql = key_equal(), const allocator_type& a = allocator_type()) : hash_func(hf), equal_func(eql) {
        //TODO
    }
    constexpr HashSet(const HashSet& v);                          //TODO
    constexpr HashSet(HashSet&& rv);                              //TODO
    constexpr explicit HashSet(const allocator_type& a);          //TODO
    constexprHashSet(const HashSet& v, const allocator_type& a);  //TODO
    constexpr HashSet(HashSet&& rv, const allocator_type& a);     //TODO
    constexpr HashSet(std::initializer_list<value_type> il, size_type n = min_bucket_size, const hasher& hf = hasher(), const key_equal& eql = key_equal(), const allocator_type& a = allocator_type()) : HashSet(il.begin(), il.end(), n, hf, eql, a) {}
    constexpr HashSet(size_type n, const allocator_type& a) : HashSet(n, hasher(), key_equal(), a) {}
    constexpr HashSet(size_type n, const hasher& hf, const allocator_type& a) : HashSet(n, hf, key_equal(), a) {}
    template<class InputIterator> constexpr HashSet(InputIterator f, InputIterator l, size_type n, const allocator_type& a) : HashSet(f, l, n, hasher(), key_equal(), a) {}
    template<class InputIterator> constexpr HashSet(InputIterator f, InputIterator l, size_type n, const hasher& hf, const allocator_type& a) : HashSet(f, l, hf, key_equal(), a) {}
    constexpr HashSet(std::initializer_list<value_type> il, size_type n, const allocator_type& a) : HashSet(il, n, hasher(), key_equal(), a) {}
    constexpr HashSet(std::initializer_list<value_type> il, size_type n, const hasher& hf, const allocator_type& a) : HashSet(il, n, hf, key_equal(), a) {}
    constexpr ~HashSet();
    constexpr HashSet& operator=(const HashSet& v);
    constexpr HashSet& operator=(HashSet&& x) noexcept(traits::is_always_equal::value && std::is_nothrow_move_assignable<Hash>::value);
    constexpr HashSet& operator=(std::initializer_list<value_type> il);
    [[nodiscard]] constexpr bool empty() const noexcept;
    constexpr size_type size() const noexcept;
    constexpr size_type max_size() const noexcept;
    constexpr iterator begin() noexcept;
    constexpr const_iterator begin() const noexcept;
    constexpr iterator end() noexcept;
    constexpr const_iterator end() const noexcept;
    constexpr const_iterator cbegin() const noexcept;
    constexpr const_iterator cend() const noexcept;
    constexpr allocator_type get_allocator() const noexcept;
    template<class... Args> constexpr std::pair<iterator, bool> emplace(Args&&... args);
    template<class... Args> constexpr iterator emplace_hint([[maybe_unused]] const_iterator position, Args&&... args) { return emplace(std::forward<Args>(args)...).first; }
    constexpr std::pair<iterator, bool> insert(const value_type& v);
    constexpr std::pair<iterator, bool> insert(value_type&& rv);
    constexpr iterator insert([[maybe_unused]] const_iterator position, const value_type& v) { return insert(v).first; }
    constexpr iterator insert([[maybe_unused]] const_iterator position, value_type&& rv) { return insert(std::move(rv)).first; }
    template<class InputIterator> constexpr void insert(InputIterator first, InputIterator last);
    constexpr void insert(std::initializer_list<value_type> il) { insert(il.begin(), il.end()); }
    //constexpr insert_return_type insert(node_type&& nh);
    //constexpr iterator insert(const_iterator hint, node_type&& nh);
    constexpr iterator erase(iterator position);
    constexpr iterator erase(const_iterator position);
    constexpr size_type erase(const key_type& k);
    constexpr iterator erase(const_iterator first, const_iterator last);
    constexpr void clear() noexcept;
    constexpr void swap(HashSet& x) noexcept(traits::is_always_equal::value && noexcept(swap(std::declval<Hash&>(), std::declval<Hash&>())) && noexcept(swap(std::declval<Pred&>(), std::declval<Pred&>())));
    //constexpr node_type extract(const_iterator position);
    //constexpr node_type extract(const key_type& x);
    template<class H2, class P2, bool U2> constexpr void merge(HashSet<Key, H2, P2, Allocator, U2>& source);
    template<class H2, class P2, bool U2> constexpr void merge(HashSet<Key, H2, P2, Allocator, U2>&& source);
    constexpr hasher hash_function() const { return hash_func; }
    constexpr key_equal key_eq() const { return equal_func; }
    constexpr iterator find(const key_type& x);
    constexpr const_iterator find(const key_type& x) const;
    template<class K> constexpr iterator find(const K& k);
    template<class K> constexpr const_iterator find(const K& k) const;
    constexpr size_type count(const key_type& x) const;
    template<class K> constexpr size_type count(const K& k) const;
    constexpr bool contains(const key_type& x) const;
    template<class K> constexpr bool contains(const K& k) const;
    constexpr std::pair<iterator, iterator> equal_range(const key_type& x);
    constexpr std::pair<const_iterator, const_iterator> equal_range(const key_type& x) const;
    template<class K> constexpr std::pair<iterator, iterator> equal_range(const K& k);
    template<class K> constexpr std::pair<const_iterator, const_iterator> equal_range(const K& k) const;
    constexpr size_type bucket_count() const noexcept;
    constexpr size_type max_bucket_count() const noexcept;
    constexpr size_type min_bucket_count() const noexcept { return min_bucket_size; }
    constexpr size_type bucket_size(size_type n) const;
    constexpr size_type bucket(const key_type& k) const;
    //constexpr local_iterator begin(size_type n);
    //constexpr const_local_iterator begin(size_type n) const;
    //constexpr local_iterator end(size_type n);
    //constexpr const_local_iterator end(size_type n) const;
    //constexpr const_local_iterator cbegin(size_type n) const;
    //constexpr const_local_iterator cend(size_type n) const;
    //constexpr float load_factor() const noexcept;
    //constexpr float max_load_factor() const noexcept;
    //constexpr void max_load_factor(float z);
    constexpr void rehash(size_type n);
    constexpr void reserve(size_type n);
};

}  // namespace gsh