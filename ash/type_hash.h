#ifndef ASH_TYPE_HASH_H_
#define ASH_TYPE_HASH_H_

#include <array>
#include <cstddef>
#include <string>
#include <tuple>
#include <type_traits>
#include <utility>

#include "ash/serializable_base.h"
#include "ash/mpt.h"
#include "ash/traits.h"

namespace ash {

/// Trait classes for type hashing, based on 32-bit FNV-1.
namespace traits {

template<typename T, typename S, typename Seen = mpt::pack<>,
		std::uint32_t base = 2166136261, typename Enable = void>
struct type_hash;

namespace detail {

enum class type_family
	: std::uint8_t {
		INTEGER = 1,
	FLOAT,
	ENUM,
	ARRAY,
	TUPLE,
	SEQUENCE,
	SET,
	MAP,
	UNIQUE_PTR,
	SHARED_PTR,
	WEAK_PTR,
	CLASS,
	BASE_CLASS,
	FIELD,
	CUSTOM_SERIALIZATION,
	SEEN_TYPE_BACKREFERENCE
};

constexpr std::size_t FAMILY_OFFSET = 0;
constexpr std::size_t SIGN_OFFSET = 7;
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

template<std::uint32_t base, typename S, typename Seen, typename ... T>
struct compose_with_types;

template<std::uint32_t base, typename S, typename Seen, typename ... T>
struct compose_with_types<base, S, Seen, mpt::pack<T...>> {
	static constexpr std::uint32_t value = compose_with_types<base, S, Seen,
			T...>::value;
};

template<std::uint32_t base, typename S, typename Seen>
struct compose_with_types<base, S, Seen> {
	static constexpr std::uint32_t value = base;
};

template<std::uint32_t base, typename S, typename Seen, typename T,
		typename ... TN>
struct compose_with_types<base, S, Seen, T, TN...> {
	static constexpr std::uint32_t value = compose_with_types<
			type_hash<T, S, Seen, base>::value, S, Seen, TN...>::value;
};

}

// If we see a type that we already saw before there's a cyclic reference somewhere,
// so insert a back-reference.
template<typename T, typename S, typename Seen, std::uint32_t base>
struct type_hash<T, S, Seen, base,
		typename std::enable_if<mpt::is_in<T, Seen>::value>::type> {
	static constexpr std::uint32_t value = detail::type_hash_add(base,
			detail::type_family::SEEN_TYPE_BACKREFERENCE, false,
			mpt::head(mpt::find_if(Seen { }, mpt::is<T> { })));
};

template<typename T, typename S, typename Seen, std::uint32_t base>
struct type_hash<T, S, Seen, base,
		typename std::enable_if<
				std::is_integral<T>::value && !mpt::is_in<T, Seen>::value>::type> {
	static constexpr std::uint32_t value = detail::type_hash_add(base,
			detail::type_family::INTEGER, std::is_signed < T > ::value,
			sizeof(T));
};

template<typename T, typename S, typename Seen, std::uint32_t base>
struct type_hash<T, S, Seen, base,
		typename std::enable_if<
				std::is_floating_point<T>::value && !mpt::is_in<T, Seen>::value>::type> {
	static constexpr std::uint32_t value = detail::type_hash_add(base,
			detail::type_family::FLOAT, std::is_signed < T > ::value,
			sizeof(T));
};

template<typename T, typename S, typename Seen, std::uint32_t base>
struct type_hash<T, S, Seen, base,
		typename std::enable_if<
				std::is_enum<T>::value && !mpt::is_in<T, Seen>::value>::type> {
	static constexpr std::uint32_t value = detail::type_hash_add(base,
			detail::type_family::ENUM, std::is_signed < T > ::value, sizeof(T));
};

template<typename T, typename S, typename Seen, std::uint32_t base>
struct type_hash<T, S, Seen, base,
		typename std::enable_if<
				std::is_array<T>::value && !mpt::is_in<T, Seen>::value>::type> {
	static constexpr std::uint32_t value = detail::compose_with_types<
			detail::type_hash_add(base, detail::type_family::ARRAY, false,
					std::extent < T > ::value), S, mpt::insert_into_t<T, Seen>,
			typename std::remove_extent<T>::type>::value;
};

template<typename U, typename V, typename S, typename Seen, std::uint32_t base>
struct type_hash<std::pair<U, V>, S, Seen, base,
		typename std::enable_if<!mpt::is_in<std::pair<U, V>, Seen>::value>::type> {
	static constexpr std::uint32_t value = detail::compose_with_types<
			detail::type_hash_add(base, detail::type_family::TUPLE, false, 2),
			S, mpt::insert_into_t<std::pair<U, V>, Seen>, U, V>::value;
};

template<typename ... T, typename S, typename Seen, std::uint32_t base>
struct type_hash<std::tuple<T...>, S, Seen, base,
		typename std::enable_if<!mpt::is_in<std::tuple<T...>, Seen>::value>::type> {
	static constexpr std::uint32_t value = detail::compose_with_types<
			detail::type_hash_add(base, detail::type_family::TUPLE, false,
					sizeof...(T)), S, mpt::insert_into_t<std::tuple<T...>, Seen>,
	T...>::value;
};

template<typename T, typename S, typename Seen, std::uint32_t base>
struct type_hash<T, S, Seen, base,
		typename std::enable_if<
				is_iterable<T>::value && has_static_size<T>::value
						&& !mpt::is_in<T, Seen>::value>::type> {
	static constexpr std::uint32_t value = detail::compose_with_types<
			detail::type_hash_add(base, detail::type_family::ARRAY, false,
					std::tuple_size < T > ::value), S,
			mpt::insert_into_t<T, Seen>, typename T::value_type>::value;
};

template<typename T, typename S, typename Seen, std::uint32_t base>
struct type_hash<T, S, Seen, base,
		typename std::enable_if<
				is_iterable<T>::value && !has_static_size<T>::value
						&& !is_associative<T>::value
						&& !mpt::is_in<T, Seen>::value>::type> {
	static constexpr std::uint32_t value =
			detail::compose_with_types<
					detail::type_hash_add(base, detail::type_family::SEQUENCE,
							false, 0), S, mpt::insert_into_t<T, Seen>,
					typename deserializable_value_type<typename T::value_type>::type>::value;
};

template<typename T, typename S, typename Seen, std::uint32_t base>
struct type_hash<T, S, Seen, base,
		typename std::enable_if<
				is_iterable<T>::value && !has_static_size<T>::value
						&& is_associative<T>::value
						&& !std::is_same<typename T::key_type,
								typename T::value_type>::value
						&& !mpt::is_in<T, Seen>::value>::type> {
	static constexpr std::uint32_t value =
			detail::compose_with_types<
					detail::type_hash_add(base, detail::type_family::MAP, false,
							0), S, mpt::insert_into_t<T, Seen>,
					typename deserializable_value_type<typename T::value_type>::type>::value;
};

template<typename T, typename S, typename Seen, std::uint32_t base>
struct type_hash<T, S, Seen, base,
		typename std::enable_if<
				is_iterable<T>::value && !has_static_size<T>::value
						&& is_associative<T>::value
						&& std::is_same<typename T::key_type,
								typename T::value_type>::value
						&& !mpt::is_in<T, Seen>::value>::type> {
	static constexpr std::uint32_t value =
			detail::compose_with_types<
					detail::type_hash_add(base, detail::type_family::SET, false,
							0), S, mpt::insert_into_t<T, Seen>,
					typename deserializable_value_type<typename T::value_type>::type>::value;
};

template<typename T, typename S, typename Seen, typename Deleter,
		std::uint32_t base>
struct type_hash<std::unique_ptr<T, Deleter>, S, Seen, base,
		typename std::enable_if<
				!mpt::is_in<std::unique_ptr<T, Deleter>, Seen>::value>::type> {
	static constexpr std::uint32_t value = detail::compose_with_types<
			detail::type_hash_add(base, detail::type_family::UNIQUE_PTR, false,
					0), S,
			mpt::insert_into_t<std::unique_ptr<T, Deleter>, Seen>, T>::value;
};

template<typename T, typename S, typename Seen, std::uint32_t base>
struct type_hash<std::shared_ptr<T>, S, Seen, base,
		typename std::enable_if<!mpt::is_in<std::shared_ptr<T>, Seen>::value>::type> {
	static constexpr std::uint32_t value =
			detail::compose_with_types<
					detail::type_hash_add(base, detail::type_family::SHARED_PTR,
							false, 0), S,
					mpt::insert_into_t<std::shared_ptr<T>, Seen>, T>::value;
};

template<typename T, typename S, typename Seen, std::uint32_t base>
struct type_hash<std::weak_ptr<T>, S, Seen, base,
		typename std::enable_if<!mpt::is_in<std::weak_ptr<T>, Seen>::value>::type> {
	static constexpr std::uint32_t value = detail::compose_with_types<
			detail::type_hash_add(base, detail::type_family::WEAK_PTR, false,
					0), S, mpt::insert_into_t<std::weak_ptr<T>, Seen>, T>::value;
};

template<typename C, typename T, T C::*m_ptr, typename S, typename Seen,
		std::uint32_t base>
struct type_hash<::ash::field_descriptor<T C::*, m_ptr>, S, Seen, base, void> {
	static constexpr std::uint32_t value = type_hash<T, S, Seen, base>::value;
};

template<typename T, typename S, typename Seen, std::uint32_t base>
struct type_hash<T, S, Seen, base,
		typename std::enable_if<
				(can_be_loaded<T, S>::value || can_be_saved<T, S>::value)
						&& !mpt::is_in<T, Seen>::value>::type> {
private:
	static constexpr std::uint32_t class_header_value = detail::type_hash_add(
			base, detail::type_family::CLASS, false,
			mpt::size<typename get_base_classes<T>::type>::value
					+ mpt::size<typename get_field_descriptors<T>::type>::value
					+ get_custom_serialization_version<T, S>::value);

	static constexpr std::uint32_t with_base_classes =
			detail::compose_with_types<
					detail::type_hash_add(class_header_value,
							detail::type_family::BASE_CLASS, false,
							mpt::size<typename get_base_classes<T>::type>::value),
					S, mpt::insert_into_t<T, Seen>,
					typename get_base_classes<T>::type>::value;

	static constexpr std::uint32_t with_fields = detail::compose_with_types<
			detail::type_hash_add(with_base_classes, detail::type_family::FIELD,
					false,
					mpt::size<typename get_field_descriptors<T>::type>::value),
			S, mpt::insert_into_t<T, Seen>,
			typename get_field_descriptors<T>::type>::value;

public:
	static constexpr std::uint32_t value = detail::type_hash_add(with_fields,
			detail::type_family::CUSTOM_SERIALIZATION, false,
			get_custom_serialization_version<T, S>::value);
};

}
// namespace traits

}// namespace ash

#endif /* ASH_TYPE_HASH_H_ */
