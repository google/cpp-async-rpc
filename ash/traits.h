#ifndef ASH_TRAITS_H_
#define ASH_TRAITS_H_

#include <cstddef>
#include <cstdint>
#include <type_traits>
#include <utility>
#include <array>
#include <string>
#include <vector>

namespace ash {

/// Trait classes for type introspection.
namespace traits {

/// Will have a `true` value if the compilation target is little-endian.
constexpr bool target_is_little_endian = (__BYTE_ORDER__
		== __ORDER_LITTLE_ENDIAN__);
/// Will have a `true` value if the compilation target is big-endian.
constexpr bool target_is_big_endian = (__BYTE_ORDER__ == __ORDER_BIG_ENDIAN__);
static_assert(
		target_is_little_endian ^ target_is_big_endian,
		"Target endianness isn't clearly big or little endian.");

/// \brief Define a checker template struct to verify whether a class has a non-const method with the given name.
///
/// Specific return and argument types are provided at template instantiation.
/// As there's no way to provide a name at template instantiation type, a new
/// instantiation of this macro is required for every distinct name to check.
///
/// The generated checker struct will be named `has_{METHOD}`.
///
/// \param METHOD Name of the (non-const) method to check for.
#define ASH_DEFINE_HAS_METHOD_CHECKER(METHOD) \
	template<typename, typename T> \
	struct has_ ## METHOD { \
		static_assert( \
				std::integral_constant<T, false>::value, \
				"Second template parameter needs to be of function type."); \
	}; \
	template<typename C, typename Ret, typename ... Args> \
	struct has_ ## METHOD <C, Ret(Args...)> { \
	private: \
		template<typename T> \
		static constexpr auto check(T*) \
		-> typename \
		std::is_same< \
		decltype( std::declval<T>(). METHOD ( std::declval<Args>()... ) ), \
		Ret \
		>::type; \
		template<typename > \
		static constexpr std::false_type check(...); \
		typedef decltype(check<C>(0)) type; \
	public: \
		static constexpr bool value = type::value; \
	};

/// \brief Define a checker template struct to verify whether a class has a const method with the given name.
///
/// Specific return and argument types are provided at template instantiation.
/// As there's no way to provide a name at template instantiation type, a new
/// instantiation of this macro is required for every distinct name to check.
///
/// The generated checker struct will be named `has_{METHOD}`.
///
/// \param METHOD Name of the const method to check for.
#define ASH_DEFINE_HAS_CONST_METHOD_CHECKER(METHOD) \
	template<typename, typename T> \
	struct has_ ## METHOD { \
		static_assert( \
				std::integral_constant<T, false>::value, \
				"Second template parameter needs to be of function type."); \
	}; \
	template<typename C, typename Ret, typename ... Args> \
	struct has_ ## METHOD <C, Ret(Args...)> { \
	private: \
		template<typename T> \
		static constexpr auto check(T*) \
		-> typename \
		std::is_same< \
		decltype( std::declval<const T>(). METHOD ( std::declval<Args>()... ) ), \
		Ret \
		>::type; \
		template<typename > \
		static constexpr std::false_type check(...); \
		typedef decltype(check<C>(0)) type; \
	public: \
		static constexpr bool value = type::value; \
	};

/// Check whether a class has a `Ret serialize(Args...)` method.
ASH_DEFINE_HAS_METHOD_CHECKER(serialize);

/// Check whether a class has a `Ret save(Args...) const` method.
ASH_DEFINE_HAS_CONST_METHOD_CHECKER(save);

/// Check whether a class has a `Ret load(Args...)` method.
ASH_DEFINE_HAS_METHOD_CHECKER(load);

/// Enable type `R` if `T` evaluates to a type.
///
/// \param T the expression to check.
template<typename T, typename R = void>
struct enable_if_type {
	/// The resulting replacement type.
	using type = R;
};

/// \brief Define a checker template struct to verify whether a class has a named internal type.
///
/// The generated checker struct will be named `has_{TYPE_NAME}`.
///
/// \param TYPE_NAME Name of the nested type to check for.
#define ASH_DEFINE_HAS_INNER_TYPE_CHECKER(TYPE_NAME) \
		template<typename T, typename Enable = void> \
		struct has_ ## TYPE_NAME : public std::false_type {}; \
		template<typename T> \
		struct has_ ## TYPE_NAME < \
			T, typename ::ash::traits::enable_if_type<typename T:: TYPE_NAME >::type> \
			: public std::true_type {};

/// Check for an inner `class_descriptor` type.
ASH_DEFINE_HAS_INNER_TYPE_CHECKER(class_descriptor);

/// Check for an inner `interface_descriptor` type.
ASH_DEFINE_HAS_INNER_TYPE_CHECKER(interface_descriptor);

/// \brief Check wether `T` is a POD scalar that can be serialized by serializing its memory as-is.
///
/// Whether a type is a scalar type that has a stable meaning when serialized between
/// processes or program runs; i.e. pointers don't qualify so this doesn't match
/// `std::is_scalar`.
///
/// \param T the type to check.
template<typename T>
struct is_serializable_scalar: public std::integral_constant<bool,
		std::is_arithmetic<T>::value || std::is_enum<T>::value> {
};

/// Check for an nested type definition for `const_iterator`.
ASH_DEFINE_HAS_INNER_TYPE_CHECKER(const_iterator);
template<typename T>
struct is_iterable: public std::integral_constant<bool,
		has_const_iterator<T>::value> {
};

/// Check for an nested type definition for `key_type`.
ASH_DEFINE_HAS_INNER_TYPE_CHECKER(key_type);
template<typename T>
/// A trait defining whether a type looks like an associative container by looking for a definition of `key_type`.
struct is_associative: public std::integral_constant<bool,
		has_key_type<T>::value> {
};

/// \brief Create a type derived of `T` suitable to create a temporary onto which we can deserialize data.
/// Used for `std::map` `value_type` elements.
template<typename T>
struct deserializable_value_type {
	using type = typename std::remove_const<T>::type;
};
/// \brief Create a type derived of `T` suitable to create a temporary onto which we can deserialize data.
/// Used for `std::map` `value_type` elements.
template<typename U, typename V>
struct deserializable_value_type<std::pair<U, V>> {
	using type = std::pair<typename std::remove_const<U>::type, typename std::remove_const<V>::type>;
};

/// A trait defining whether a type can be serialized as a contiguous block of memory.
template<typename T>
struct is_contiguous_sequence: public std::false_type {
};
/// A trait defining whether a type can be serialized as a contiguous block of memory.
template<typename T, std::size_t size>
struct is_contiguous_sequence<std::array<T, size>> : public std::true_type {
};
/// A trait defining whether a type can be serialized as a contiguous block of memory.
template<typename T, typename Allocator>
struct is_contiguous_sequence<std::vector<T, Allocator>> : public std::true_type {
};
/// A trait defining whether a type can be serialized as a contiguous block of memory.
template<typename CharT, typename Traits, typename Allocator>
struct is_contiguous_sequence<std::basic_string<CharT, Traits, Allocator>> : public std::true_type {
};

/// Check whether a class has a `Ret reserve(Args...)` method.
ASH_DEFINE_HAS_METHOD_CHECKER(reserve);
template<typename T>
struct has_storage_reservation: public std::integral_constant<bool,
		has_reserve<T, void(std::size_t)>::value> {
};

/// Check whether a class has a `Ret resize(Args...)` method.
ASH_DEFINE_HAS_METHOD_CHECKER(resize);
template<typename T>
struct has_storage_resizing: public std::integral_constant<bool,
		has_resize<T, void(std::size_t)>::value> {
};

/// A trait defining whether a type has a fixed number of elements set at compile time.
template<typename T>
struct has_static_size: public std::false_type {
};
/// A trait defining whether a type has a fixed number of elements set at compile time.
template<typename T, std::size_t size>
struct has_static_size<std::array<T, size>> : public std::true_type {
};

}  // namespace traits

}  // namespace ash

#endif /* ASH_TRAITS_H_ */
