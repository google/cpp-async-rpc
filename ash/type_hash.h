#ifndef ASH_TYPE_HASH_H_
#define ASH_TYPE_HASH_H_

#include <array>
#include <cstddef>
#include <string>
#include <tuple>
#include <type_traits>
#include <utility>

#include "ash/mpt.h"
#include "ash/traits.h"

namespace ash {

/// Trait classes for type hashing, based on 32-bit FNV-1.
namespace traits {

template<typename T, std::uint32_t base = 2166136261, typename Enable = void>
struct type_hash;

namespace detail {

enum class type_family
	: std::uint8_t {
		INTEGER = 1,
	FLOAT,
	ENUM,
	ARRAY,
	TUPLE,
	CLASS,
	SEQUENCE,
	SET,
	MAP,
	UNIQUE_PTR,
	SHARED_PTR,
	WEAK_PTR
};

constexpr std::size_t FAMILY_OFFSET = 0;
constexpr std::size_t SIGN_OFFSET = 4;
constexpr std::size_t SIZE_OFFSET = 8;

constexpr std::uint32_t type_hash_compose(std::uint32_t base,
		std::uint32_t more) {
	return (base * 16777619) ^ more;
}

constexpr std::uint32_t type_hash_add(std::uint32_t base, type_family type,
		bool is_signed, std::size_t size) {
	return type_hash_compose(base,
			(static_cast<std::uint32_t>(type) << detail::FAMILY_OFFSET)
					| (static_cast<std::uint32_t>(is_signed)
							<< detail::SIGN_OFFSET)
					| (static_cast<std::uint32_t>(size) << detail::SIZE_OFFSET));
}

template<std::uint32_t base, typename ... T>
struct compose_with_types;

template<std::uint32_t base>
struct compose_with_types<base> {
	static constexpr std::uint32_t value = base;
};

template<std::uint32_t base, typename T, typename ... TN>
struct compose_with_types<base, T, TN...> {
	static constexpr std::uint32_t value = compose_with_types<
			type_hash<T, base>::value, TN...>::value;
};

}

template<typename T, std::uint32_t base>
struct type_hash<T, base,
		typename std::enable_if<std::is_integral<T>::value>::type> {
	static constexpr std::uint32_t value = detail::type_hash_add(base,
			detail::type_family::INTEGER, std::is_signed < T > ::value,
			sizeof(T));
};

template<typename T, std::uint32_t base>
struct type_hash<T, base,
		typename std::enable_if<std::is_floating_point<T>::value>::type> {
	static constexpr std::uint32_t value = detail::type_hash_add(base,
			detail::type_family::FLOAT, std::is_signed < T > ::value,
			sizeof(T));
};

template<typename T, std::uint32_t base>
struct type_hash<T, base, typename std::enable_if<std::is_enum<T>::value>::type> {
	static constexpr std::uint32_t value = detail::type_hash_add(base,
			detail::type_family::ENUM, std::is_signed < T > ::value, sizeof(T));
};

template<typename T, std::uint32_t base>
struct type_hash<T, base, typename std::enable_if<std::is_array<T>::value>::type> {
	static constexpr std::uint32_t value = detail::compose_with_types<
			detail::type_hash_add(base, detail::type_family::ARRAY, false,
					std::extent < T > ::value),
			typename std::remove_extent<T>::type>::value;
};

template<typename U, typename V, std::uint32_t base>
struct type_hash<std::pair<U, V>, base, void> {
	static constexpr std::uint32_t value = detail::compose_with_types<
			detail::type_hash_add(base, detail::type_family::TUPLE, false, 2),
			U, V>::value;
};

template<typename ... T, std::uint32_t base>
struct type_hash<std::tuple<T...>, base, void> {
	static constexpr std::uint32_t value = detail::compose_with_types<
			detail::type_hash_add(base, detail::type_family::TUPLE, false,
					sizeof...(T)),
	T...>::value;
};

template<typename T, std::uint32_t base>
struct type_hash<T, base,
		typename std::enable_if<
				is_iterable<T>::value && has_static_size<T>::value>::type> {
	static constexpr std::uint32_t value =
			detail::compose_with_types<
					detail::type_hash_add(base, detail::type_family::ARRAY,
							false, std::tuple_size < T > ::value),
					typename T::value_type>::value;
};

template<typename T, std::uint32_t base>
struct type_hash<T, base,
		typename std::enable_if<
				is_iterable<T>::value && !has_static_size<T>::value && !is_associative<T>::value>::type> {
	static constexpr std::uint32_t value =
			detail::compose_with_types<
					detail::type_hash_add(base, detail::type_family::SEQUENCE,
							false, 0),
					typename deserializable_value_type<typename T::value_type>::type>::value;
};

template<typename T, std::uint32_t base>
struct type_hash<T, base,
		typename std::enable_if<
				is_iterable<T>::value && !has_static_size<T>::value && is_associative<T>::value && !std::is_same<typename T::key_type, typename T::value_type>::value>::type> {
	static constexpr std::uint32_t value =
			detail::compose_with_types<
					detail::type_hash_add(base, detail::type_family::MAP,
							false, 0),
					typename deserializable_value_type<typename T::value_type>::type>::value;
};

template<typename T, std::uint32_t base>
struct type_hash<T, base,
		typename std::enable_if<
				is_iterable<T>::value && !has_static_size<T>::value && is_associative<T>::value && std::is_same<typename T::key_type, typename T::value_type>::value>::type> {
	static constexpr std::uint32_t value =
			detail::compose_with_types<
					detail::type_hash_add(base, detail::type_family::SET,
							false, 0),
					typename deserializable_value_type<typename T::value_type>::type>::value;
};

template<typename T, typename Deleter, std::uint32_t base>
struct type_hash<std::unique_ptr<T, Deleter>, base, void> {
	static constexpr std::uint32_t value = detail::compose_with_types<
			detail::type_hash_add(base, detail::type_family::UNIQUE_PTR, false,
					0), T>::value;
};

template<typename T, std::uint32_t base>
struct type_hash<std::shared_ptr<T>, base, void> {
	static constexpr std::uint32_t value = detail::compose_with_types<
			detail::type_hash_add(base, detail::type_family::SHARED_PTR, false,
					0), T>::value;
};

template<typename T, std::uint32_t base>
struct type_hash<std::weak_ptr<T>, base, void> {
	static constexpr std::uint32_t value = detail::compose_with_types<
			detail::type_hash_add(base, detail::type_family::WEAK_PTR, false,
					0), T>::value;
};

}  // namespace traits

}  // namespace ash

#endif /* ASH_TYPE_HASH_H_ */
