#pragma once
#include "gsh/TypeDef.hpp"
#include <type_traits>
#include <utility>
namespace gsh {
template<class Key, class Value> struct KeyValuePair;
}
namespace std {
template<class Key, class Value> class tuple_size<gsh::KeyValuePair<Key, Value>> {
public:
  constexpr static size_t value = !std::is_void_v<Key> + !std::is_void_v<Value>;
};
template<class Key, class Value> class tuple_element<0, gsh::KeyValuePair<Key, Value>> {
public:
  using type = Key;
};
template<class Key, class Value> class tuple_element<1, gsh::KeyValuePair<Key, Value>> {
public:
  using type = Value;
};
template<class Key> class tuple_element<0, gsh::KeyValuePair<Key, void>> {
public:
  using type = Key;
};
template<class Value> class tuple_element<0, gsh::KeyValuePair<void, Value>> {
public:
  using type = Value;
};
} // namespace std
namespace gsh {
template<size_t I, class Key, class Value> constexpr auto& get(gsh::KeyValuePair<Key, Value>& p) noexcept {
  static_assert(I < (!std::is_void_v<Key> + !std::is_void_v<Value>));
  if constexpr(std::is_void_v<Key> || I == 1) return p.value;
  else return p.key;
}
template<size_t I, class Key, class Value> constexpr auto& get(const gsh::KeyValuePair<Key, Value>& p) noexcept {
  static_assert(I < (!std::is_void_v<Key> + !std::is_void_v<Value>));
  if constexpr(std::is_void_v<Key> || I == 1) return p.value;
  else return p.key;
}
template<size_t I, class Key, class Value> constexpr decltype(auto) get(gsh::KeyValuePair<Key, Value>&& p) noexcept {
  static_assert(I < (!std::is_void_v<Key> + !std::is_void_v<Value>));
  if constexpr(std::is_void_v<Key> || I == 1) return static_cast<std::decay_t<decltype(p)>::value_type&&>(p.value);
  else return static_cast<std::decay_t<decltype(p)>::key_type&&>(p.key);
}
template<size_t I, class Key, class Value> constexpr decltype(auto) get(const gsh::KeyValuePair<Key, Value>&& p) noexcept {
  static_assert(I < (!std::is_void_v<Key> + !std::is_void_v<Value>));
  if constexpr(std::is_void_v<Key> || I == 1) return static_cast<const std::decay_t<decltype(p)>::value_type&&>(p.value);
  else return static_cast<const std::decay_t<decltype(p)>::key_type&&>(p.key);
}
template<class Key, class Value> constexpr auto& get(gsh::KeyValuePair<Key, Value>& p) {
  return p.key;
}
template<class Key, class Value> constexpr auto& get(const gsh::KeyValuePair<Key, Value>& p) {
  return p.key;
}
template<class Key, class Value> constexpr decltype(auto) get(gsh::KeyValuePair<Key, Value>&& p) {
  return static_cast<std::decay_t<decltype(p)>::key_type&&>(p.key);
}
template<class Key, class Value> constexpr decltype(auto) get(const gsh::KeyValuePair<Key, Value>&& p) {
  return static_cast<const std::decay_t<decltype(p)>::key_type&&>(p.key);
}
template<class Value, class Key> constexpr auto& get(gsh::KeyValuePair<Key, Value>& p) {
  return p.value;
}
template<class Value, class Key> constexpr auto& get(const gsh::KeyValuePair<Key, Value>& p) {
  return p.value;
}
template<class Value, class Key> constexpr decltype(auto) get(gsh::KeyValuePair<Key, Value>&& p) {
  return static_cast<typename std::decay_t<decltype(p)>::value_type&&>(p.value);
}
template<class Value, class Key> constexpr decltype(auto) get(const gsh::KeyValuePair<Key, Value>&& p) {
  return static_cast<const std::decay_t<decltype(p)>::value_type&&>(p.value);
}
namespace internal {
template<class T> void TestImplicitConstructor(T);
template<class T> concept IsImplicitlyDefaultConstructible = requires { TestImplicitConstructor<const T&>({}); };
template<class T, class U> constexpr auto SynthThreeWay(const T& t, const U& u)
requires requires {
  static_cast<bool>(t < u);
  static_cast<bool>(u < t);
}
{
  if constexpr(std::three_way_comparable_with<T, U>) {
    return t <=> u;
  } else {
    if(t < u) return std::weak_ordering::less;
    if(u < t) return std::weak_ordering::greater;
    return std::weak_ordering::equivalent;
  }
};
} // namespace internal
template<class Key, class Value> struct KeyValuePair {
  using key_type = Key;
  using value_type = Value;
  using first_type = Key;
  using second_type = Value;
  Key key;
  Value value;
  explicit(!internal::IsImplicitlyDefaultConstructible<Key> || !internal::IsImplicitlyDefaultConstructible<Value>) constexpr KeyValuePair() : key(), value() {}
  explicit(!std::is_convertible_v<const Key&, Key> || !std::is_convertible_v<const Value&, Value>) constexpr KeyValuePair(const Key& k, const Value& v) : key(k), value(v) {}
  template<class K, class V> explicit(!std::is_convertible_v<K, Key> || !std::is_convertible_v<V, Value>) constexpr KeyValuePair(K&& k, V&& v) : key(std::forward<K>(k)), value(std::forward<V>(v)) {}
private:
  template<class T> constexpr static bool is_explicit_constructor = !std::is_convertible_v<decltype(std::get<0>(std::declval<T>())), Key> || !std::is_convertible_v<decltype(std::get<1>(std::declval<T>())), Value>;
public:
  template<class K, class V> explicit(is_explicit_constructor<KeyValuePair<K, V>&>) constexpr KeyValuePair(KeyValuePair<K, V>& p) : key(std::get<0>(std::forward<decltype(p)>(p))), value(std::get<1>(std::forward<decltype(p)>(p))) {}
  template<class K, class V> explicit(is_explicit_constructor<const KeyValuePair<K, V>&>) constexpr KeyValuePair(const KeyValuePair<K, V>& p) : key(std::get<0>(std::forward<decltype(p)>(p))), value(std::get<1>(std::forward<decltype(p)>(p))) {}
  template<class K, class V> explicit(is_explicit_constructor<KeyValuePair<K, V>&&>) constexpr KeyValuePair(KeyValuePair<K, V>&& p) : key(std::get<0>(std::forward<decltype(p)>(p))), value(std::get<1>(std::forward<decltype(p)>(p))) {}
  template<class K, class V> explicit(is_explicit_constructor<const KeyValuePair<K, V>&&>) constexpr KeyValuePair(const KeyValuePair<K, V>&& p) : key(std::get<0>(std::forward<decltype(p)>(p))), value(std::get<1>(std::forward<decltype(p)>(p))) {}
private:
  template<size_t... I1, size_t... I2, class... Args1, class... Args2> constexpr KeyValuePair(std::index_sequence<I1...>, std::index_sequence<I2...>, std::tuple<Args1...> k, std::tuple<Args2...> v) : key(std::get<I1>(k)...), value(std::get<I2>(v)...) {}
public:
  template<class... Args1, class... Args2> constexpr KeyValuePair(std::piecewise_construct_t, std::tuple<Args1...> key_args, std::tuple<Args2...> value_args) : KeyValuePair(std::make_index_sequence<sizeof...(Args1)>(), std::make_index_sequence<sizeof...(Args2)>(), std::forward<decltype(key_args)>(key_args), std::forward<decltype(value_args)>(value_args)) {}
  constexpr void swap(KeyValuePair& p) noexcept(noexcept([]() {
    using std::swap;
    return noexcept(swap(key, p.key)) && noexcept(swap(value, p.value));
  }())) {
    using std::swap;
    swap(key, p.key);
    swap(value, p.value);
  }
  constexpr KeyValuePair& operator=(const KeyValuePair&) = default;
  constexpr const KeyValuePair& operator=(const KeyValuePair& p) const { return key = p.key, value = p.value, *this; }
  template<class U, class V> constexpr KeyValuePair& operator=(const KeyValuePair<U, V>& p) { return key = p.key, value = p.value, *this; }
  template<class U, class V> constexpr const KeyValuePair& operator=(const KeyValuePair<U, V>& p) const { return key = p.key, value = p.value, *this; }
  constexpr KeyValuePair& operator=(KeyValuePair&& p) noexcept(std::is_nothrow_move_assignable_v<Key> && std::is_nothrow_move_assignable_v<Value>) = default;
  constexpr const KeyValuePair& operator=(KeyValuePair&& p) const { return key = std::move(p.key), value = std::move(p.value), *this; }
  template<class U, class V> constexpr KeyValuePair& operator=(KeyValuePair<U, V>&& p) { return key = std::move(p.key), value = std::move(p.value), *this; }
  template<class U, class V> constexpr const KeyValuePair& operator=(KeyValuePair<U, V>&& p) const { return key = std::move(p.key), value = std::move(p.value), *this; }
  friend constexpr bool operator==(const KeyValuePair& x, const KeyValuePair& y) { return x.key == y.key; }
  friend constexpr auto operator<=>(const KeyValuePair& x, const KeyValuePair& y) { return SynthThreeWay(x.key, y.key); }
};
template<> struct KeyValuePair<void, void> {
  using key_type = void;
  using value_type = void;
  KeyValuePair() = default;
  KeyValuePair(const KeyValuePair&) = default;
  KeyValuePair(KeyValuePair&&) = default;
  constexpr KeyValuePair(std::piecewise_construct_t) noexcept {}
  constexpr void swap(KeyValuePair&) noexcept {}
  KeyValuePair& operator=(const KeyValuePair&) = default;
  KeyValuePair& operator=(KeyValuePair&&) = default;
};
template<class Key> struct KeyValuePair<Key, void> {
  using key_type = Key;
  using value_type = void;
  using first_type = Key;
  Key key;
  explicit(!internal::IsImplicitlyDefaultConstructible<Key>) constexpr KeyValuePair() : key() {}
  explicit(!std::is_convertible_v<const Key&, Key>) constexpr KeyValuePair(const Key& k) : key(k) {}
  template<class K> explicit(!std::is_convertible_v<K, Key>) constexpr KeyValuePair(K&& k) : key(std::forward<K>(k)) {}
private:
  template<class T> constexpr static bool is_explicit_constructor = !std::is_convertible_v<decltype(std::get<0>(std::declval<T>())), Key>;
public:
  template<class K> explicit(is_explicit_constructor<KeyValuePair<K, void>&>) constexpr KeyValuePair(KeyValuePair<K, void>& p) : key(std::get<0>(std::forward<decltype(p)>(p))) {}
  template<class K> explicit(is_explicit_constructor<const KeyValuePair<K, void>&>) constexpr KeyValuePair(const KeyValuePair<K, void>& p) : key(std::get<0>(std::forward<decltype(p)>(p))) {}
  template<class K> explicit(is_explicit_constructor<KeyValuePair<K, void>&&>) constexpr KeyValuePair(KeyValuePair<K, void>&& p) : key(std::get<0>(std::forward<decltype(p)>(p))) {}
  template<class K> explicit(is_explicit_constructor<const KeyValuePair<K, void>&&>) constexpr KeyValuePair(const KeyValuePair<K, void>&& p) : key(std::get<0>(std::forward<decltype(p)>(p))) {}
private:
  template<std::size_t... I, class... Args> constexpr KeyValuePair(std::index_sequence<I...>, std::tuple<Args...> args) : key(std::get<I>(args)...) {}
public:
  template<class... Args> constexpr KeyValuePair(std::piecewise_construct_t, std::tuple<Args...> args) : KeyValuePair(std::make_index_sequence<sizeof...(Args)>(), std::forward<decltype(args)>(args)) {}
  constexpr void swap(KeyValuePair& p) noexcept(noexcept([]() {
    using std::swap;
    return noexcept(swap(key, p.key));
  }())) {
    using std::swap;
    swap(key, p.key);
  }
  constexpr KeyValuePair& operator=(const KeyValuePair&) = default;
  constexpr const KeyValuePair& operator=(const KeyValuePair& p) const { return key = p.key, *this; }
  template<class U> constexpr KeyValuePair& operator=(const KeyValuePair<U, void>& p) { return key = p.key, *this; }
  template<class U> constexpr const KeyValuePair& operator=(const KeyValuePair<U, void>& p) const { return key = p.key, *this; }
  constexpr KeyValuePair& operator=(KeyValuePair&& p) noexcept(std::is_nothrow_move_assignable_v<Key>) = default;
  constexpr const KeyValuePair& operator=(KeyValuePair&& p) const { return key = std::move(p.key), *this; }
  template<class U> constexpr KeyValuePair& operator=(KeyValuePair<U, void>&& p) { return key = std::move(p.key), *this; }
  template<class U> constexpr const KeyValuePair& operator=(KeyValuePair<U, void>&& p) const { return key = std::move(p.key), *this; }
  friend constexpr bool operator==(const KeyValuePair& x, const KeyValuePair& y) { return x.key == y.key; }
  friend constexpr auto operator<=>(const KeyValuePair& x, const KeyValuePair& y) { return SynthThreeWay(x.key, y.key); }
};
template<class Value> struct KeyValuePair<void, Value> {
  using key_type = void;
  using value_type = Value;
  using first_type = Value;
  Value value;
  explicit(!internal::IsImplicitlyDefaultConstructible<Value>) constexpr KeyValuePair() : value() {}
  explicit(!std::is_convertible_v<const Value&, Value>) constexpr KeyValuePair(const Value& v) : value(v) {}
  template<class V> explicit(!std::is_convertible_v<V, Value>) constexpr KeyValuePair(V&& v) : value(std::forward<V>(v)) {}
private:
  template<class T> constexpr static bool is_explicit_constructor = !std::is_convertible_v<decltype(std::get<0>(std::declval<T>())), Value>;
public:
  template<class V> explicit(is_explicit_constructor<KeyValuePair<void, V>&>) constexpr KeyValuePair(KeyValuePair<void, V>& p) : value(std::get<0>(std::forward<decltype(p)>(p))) {}
  template<class V> explicit(is_explicit_constructor<const KeyValuePair<void, V>&>) constexpr KeyValuePair(const KeyValuePair<void, V>& p) : value(std::get<0>(std::forward<decltype(p)>(p))) {}
  template<class V> explicit(is_explicit_constructor<KeyValuePair<void, V>&&>) constexpr KeyValuePair(KeyValuePair<void, V>&& p) : value(std::get<0>(std::forward<decltype(p)>(p))) {}
  template<class V> explicit(is_explicit_constructor<const KeyValuePair<void, V>&&>) constexpr KeyValuePair(const KeyValuePair<void, V>&& p) : value(std::get<0>(std::forward<decltype(p)>(p))) {}
private:
  template<std::size_t... I, class... Args> constexpr KeyValuePair(std::index_sequence<I...>, std::tuple<Args...> args) : value(std::get<I>(args)...) {}
public:
  template<class... Args> constexpr KeyValuePair(std::piecewise_construct_t, std::tuple<Args...> args) : KeyValuePair(std::make_index_sequence<sizeof...(Args)>(), std::forward<decltype(args)>(args)) {}
  constexpr void swap(KeyValuePair& p) noexcept(noexcept([]() {
    using std::swap;
    return noexcept(swap(value, p.value));
  }())) {
    using std::swap;
    swap(value, p.value);
  }
  constexpr KeyValuePair& operator=(const KeyValuePair&) = default;
  constexpr const KeyValuePair& operator=(const KeyValuePair& p) const { return value = p.value, *this; }
  template<class V> constexpr KeyValuePair& operator=(const KeyValuePair<void, V>& p) { return value = p.value, *this; }
  template<class V> constexpr const KeyValuePair& operator=(const KeyValuePair<void, V>& p) const { return value = p.value, *this; }
  constexpr KeyValuePair& operator=(KeyValuePair&& p) noexcept(std::is_nothrow_move_assignable_v<Value>) = default;
  constexpr const KeyValuePair& operator=(KeyValuePair&& p) const { return value = std::move(p.value), *this; }
  template<class V> constexpr KeyValuePair& operator=(KeyValuePair<void, V>&& p) { return value = std::move(p.value), *this; }
  template<class V> constexpr const KeyValuePair& operator=(KeyValuePair<void, V>&& p) const { return value = std::move(p.value), *this; }
};
} // namespace gsh
namespace std {
template<class Key, class Value> constexpr void swap(gsh::KeyValuePair<Key, Value>& p, gsh::KeyValuePair<Key, Value>& q) noexcept(noexcept(p.swap(q))) {
  p.swap(q);
}
} // namespace std
