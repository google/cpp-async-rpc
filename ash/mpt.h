#ifndef ASH_MPT_H_
#define ASH_MPT_H_

#include <cstddef>
#include <tuple>
#include <utility>
#include <type_traits>

namespace ash {

/// Type-based meta-programming toolkit.
namespace mpt {

/// Backport of `std::conditional_t` to c++11.
template< bool B, class T, class F >
using conditional_t = typename std::conditional<B,T,F>::type;

/// Backport of `std::conjunction` to c++11.
template<class...> struct conjunction : std::true_type { };
/// Backport of `std::conjunction` to c++11.
template<class B1> struct conjunction<B1> : B1 { };
/// Backport of `std::conjunction` to c++11.
template<class B1, class... Bn>
struct conjunction<B1, Bn...>
    : conditional_t<bool(B1::value), conjunction<Bn...>, B1> {};

/// Backport of `std::disjunction` to c++11.
template<class...> struct disjunction : std::false_type { };
/// Backport of `std::disjunction` to c++11.
template<class B1> struct disjunction<B1> : B1 { };
/// Backport of `std::disjunction` to c++11.
template<class B1, class... Bn>
struct disjunction<B1, Bn...>
    : conditional_t<bool(B1::value), B1, disjunction<Bn...>>  { };

/// \brief Define a whole set of arithmetic operators.
/// Macro to define the whole set of arithmetic operations given a pair of macros
/// for unary and binary operations.
/// \param CREATE_UNARY_OPERATOR Macro to be used for defining a unary operator in terms of the `NAME` and the operation (`OP`).
/// \param CREATE_BINARY_OPERATOR Macro to be used for defining a binary operator in terms of the `NAME` and the operation (`OP`).
#define ASH_CREATE_OPERATOR_HIERARCHY(CREATE_UNARY_OPERATOR, CREATE_BINARY_OPERATOR) \
	CREATE_UNARY_OPERATOR(negate, -); \
	CREATE_UNARY_OPERATOR(logical_not, !); \
	CREATE_UNARY_OPERATOR(bit_not, ~); \
	CREATE_BINARY_OPERATOR(plus, +); \
	CREATE_BINARY_OPERATOR(minus, -); \
	CREATE_BINARY_OPERATOR(multiplies, *); \
	CREATE_BINARY_OPERATOR(divides, /); \
	CREATE_BINARY_OPERATOR(modulus, %); \
	CREATE_BINARY_OPERATOR(equal_to, ==); \
	CREATE_BINARY_OPERATOR(not_equal_to, !=); \
	CREATE_BINARY_OPERATOR(greater, >); \
	CREATE_BINARY_OPERATOR(less, <); \
	CREATE_BINARY_OPERATOR(greater_equal, >=); \
	CREATE_BINARY_OPERATOR(less_equal, <=); \
	CREATE_BINARY_OPERATOR(logical_and, &&); \
	CREATE_BINARY_OPERATOR(logical_or, ||); \
	CREATE_BINARY_OPERATOR(bit_and, &); \
	CREATE_BINARY_OPERATOR(bit_or, |); \
	CREATE_BINARY_OPERATOR(bit_xor, ^);

/// \brief Define a whole set of arithmetic operators plus the identity.
/// Macro to define the whole set of arithmetic operations (plus the identitiy operation)
/// given a pair of macros for unary and binary operations.
///
/// Notice that the identity operation is special in the sense that the value of `OP` for it
/// is the empty string, and thus it's not suitable to define a `operator OP ()` function.
///
/// \param CREATE_UNARY_OPERATOR Macro to be used for defining a unary operator in terms of the `NAME` and the operation (`OP`).
/// \param CREATE_BINARY_OPERATOR Macro to be used for defining a binary operator in terms of the `NAME` and the operation (`OP`).
#define ASH_CREATE_OPERATOR_HIERARCHY_WITH_IDENTITY(CREATE_UNARY_OPERATOR, CREATE_BINARY_OPERATOR) \
	ASH_CREATE_OPERATOR_HIERARCHY(CREATE_UNARY_OPERATOR, CREATE_BINARY_OPERATOR) \
	CREATE_UNARY_OPERATOR(identity, );

/// Define unary arithmetic operation functors on arbitrary types.
#define ASH_CREATE_UNARY_OPERATOR(NAME, OP) \
struct NAME { \
	template <typename T> \
	constexpr auto operator()(T v) \
	-> decltype(OP v) \
	{ return OP v; } \
};
/// Define binary arithmetic operation functors on arbitrary types.
#define ASH_CREATE_BINARY_OPERATOR(NAME, OP) \
struct NAME { \
	template <typename T, typename U> \
	constexpr auto operator()(T v, U w) \
	-> decltype(v OP w) \
	{ return v OP w; } \
};
/// Create a complete set of arithmetic operation functors on arbitrary types.
ASH_CREATE_OPERATOR_HIERARCHY_WITH_IDENTITY(ASH_CREATE_UNARY_OPERATOR, ASH_CREATE_BINARY_OPERATOR);

/// \brief Types representing sequences of homogeneously typed integers.
/// An `integer_sequence<T, Ints...>` type represents a compile-time sequence of
/// a variable number of integers (given by `Ints...`), all of the same type `T`.
/// \param T The type of all the integers.
/// \param Ints... The list of `T`-typed integers to encode in the type.
template<typename T, T ...Ints>
struct integer_sequence {
	/// The type's own type.
	using type = integer_sequence<T, Ints...>;

	/// The type of the integers in the sequence.
	using value_type = T;

	/// The number of integers in the sequence.
	static constexpr std::size_t size = sizeof...(Ints);

private:
	template <typename O>
	struct unary_op{
		using result_type = typename std::remove_const<decltype(O()(T()))>::type;
		using type = integer_sequence<result_type, (O()(Ints))...>;
	};

	template <typename U, typename O>
	struct binary_op;

	template <typename U, U ...IntsU, typename O>
	struct binary_op<integer_sequence<U, IntsU...>, O> {
		using result_type = typename std::remove_const<decltype(O()(T(), U()))>::type;
		using type = integer_sequence<result_type, (O()(Ints, IntsU))...>;
	};

public:
	/// Type-based unary operator generator macro.
	# define ASH_INT_SEQ_TYPE_UNARY_OP(NAME, OP) \
	struct NAME : unary_op<::ash::mpt:: NAME > {};
	/// Type-based binary operator generator macro.
	# define ASH_INT_SEQ_TYPE_BINARY_OP(NAME, OP) \
	template <typename U> \
	struct NAME : binary_op<U, ::ash::mpt:: NAME > {};
	/// Type-based operator hierarchy.
	ASH_CREATE_OPERATOR_HIERARCHY_WITH_IDENTITY(ASH_INT_SEQ_TYPE_UNARY_OP, ASH_INT_SEQ_TYPE_BINARY_OP);

	/// Value-based unary operator generator macro.
	# define ASH_INT_SEQ_VALUE_UNARY_OP(NAME, OP) \
	constexpr typename NAME ::type operator OP () { return {}; }
	/// Value-based binary operator generator macro.
	# define ASH_INT_SEQ_VALUE_BINARY_OP(NAME, OP) \
	template <typename U> \
	constexpr typename NAME <U>::type operator OP (U) { return {}; }
	// Value-based operator hierarchy.
	ASH_CREATE_OPERATOR_HIERARCHY(ASH_INT_SEQ_VALUE_UNARY_OP, ASH_INT_SEQ_VALUE_BINARY_OP);
};

namespace detail {

template<typename Is, typename Js, bool adjust_values = false>
struct join_integer_sequences;

template<typename T, T ...Is, T ...Js, bool adjust_values>
struct join_integer_sequences<integer_sequence<T, Is...>,
		integer_sequence<T, Js...>, adjust_values> {
	using type = integer_sequence<T, Is..., ((adjust_values ? sizeof...(Is) : 0) + Js)...>;
};

template<typename T, std::size_t N>
struct make_integer_sequence: join_integer_sequences<
		typename make_integer_sequence<T, N / 2>::type,
		typename make_integer_sequence<T, N - N / 2>::type,
		true> {
};

template<typename T>
struct make_integer_sequence<T, 1> {
	using type = integer_sequence<T, 0>;
};

template<typename T>
struct make_integer_sequence<T, 0> {
	using type = integer_sequence<T>;
};

template<typename T, std::size_t N, T v>
struct make_constant_integer_sequence: join_integer_sequences<
		typename make_constant_integer_sequence<T, N / 2, v>::type,
		typename make_constant_integer_sequence<T, N - N / 2, v>::type> {
};

template<typename T, T v>
struct make_constant_integer_sequence<T, 1, v> {
	using type = integer_sequence<T, v>;
};

template<typename T, T v>
struct make_constant_integer_sequence<T, 0, v> {
	using type = integer_sequence<T>;
};

}  // namespace detail

/// \brief Generate a sequence of integers.
/// The integers are consecutive, in the range `0 .. N-1`.
/// \param T The type of the integers.
/// \param N The total number of integers the sequence will have.
template<typename T, std::size_t N>
using make_integer_sequence = typename detail::make_integer_sequence<T, N>::type;

/// \brief Generate a repeated sequence of the same integer.
/// This will generate `N` integers with the value `v` of type `T`.
/// \param T The type of the integers.
/// \param N The total number of integers the sequence will have.
/// \param v The value of the integers.
template<typename T, std::size_t N, T v>
using make_constant_integer_sequence = typename detail::make_constant_integer_sequence<T, N, v>::type;

/// \brief Types representing sequences of `std::size_t` which can be used as indices.
/// An `index_sequence<Ints...>` type represents a compile-time sequence of
/// a variable number of `std::size_t` values (given by `Ints...`).
/// \param Ints... The list of `std::size_t`-typed integers to encode in the type.
template<std::size_t ...Ints>
using index_sequence = integer_sequence<std::size_t, Ints...>;
/// \brief Generate a sequence of indices.
/// The integers are consecutive, in the range `0 .. N-1`, and of `std::size_t` type.
/// \param N The total number of integers the sequence will have.
template<std::size_t N>
using make_index_sequence = make_integer_sequence<std::size_t, N>;
/// \brief Generate a repeated sequence of the same index.
/// This will generate `N` indices with the value `v` of type `std::size_t`.
/// \param N The total number of indices the sequence will have.
/// \param v The value of the indices.
template<std::size_t N, std::size_t v>
using make_constant_index_sequence = make_constant_integer_sequence<std::size_t, N, v>;

/// \brief Template class to represent sequences of types.
/// `pack` structs contain no data; all the information they contain is the list
/// of types represented by `T...`.
/// \param T... List of types to pack.
template<typename ...T>
struct pack {
};

/// \brief Wrap one type so that it can be passed around without constructing any instance of it.
/// `wrap_type<T>` encodes a single type in a class whose objects can be passed around at runtime
/// without having to copy any data.
/// \param T The type to wrap.
template <typename T>
struct wrap_type {
	/// Actual type wrapped by the `wrap_type` specialization.
	using type = T;
};

namespace detail {
// Get the number of types.
template <typename T>
struct size;

template <typename ...T>
struct size<pack<T...>> {
	static constexpr std::size_t value = sizeof...(T);
};

template <typename ...T>
struct size<std::tuple<T...>> {
	static constexpr std::size_t value = sizeof...(T);
};

template <typename T, T ...Nums>
struct size<integer_sequence<T, Nums...>> {
	static constexpr std::size_t value = sizeof...(Nums);
};
}  // namespace detail

/// \brief Get the size of a sequence-like type.
/// This template struct provides a standard way to look at the size (number of elements)
/// in any of the sequence types supported by `ash::mpt`.
/// \param T The type from which to obtain the size.
template <typename T>
struct size {
	/// Number of elements of the sequence-like type `T`.
	static constexpr std::size_t value = detail::size<typename std::remove_cv<typename std::remove_reference<T>::type>::type>::value;
};

namespace detail {

template <std::size_t n, typename = make_index_sequence<n>>
struct nth_element_impl;

template <std::size_t n, std::size_t ...ignore>
struct nth_element_impl<n, index_sequence<ignore...>> {
    template <typename Tn>
    static Tn f(decltype((void*)ignore)..., Tn*, ...);
};

}  // namespace detail

/// \brief Get the type of the `i`th element of a sequence-like type.
/// This template struct provides a standard way to look at the type
/// of a particular element in any of the sequence types supported by `ash::mpt`.
/// \param T The type for which to inspect the element type.
/// \param i The index of the element for which we want to retrieve the type.
template<std::size_t i, typename T>
struct element_type;
/// \brief Get the type of the `i`th element of a sequence-like type.
/// This template struct provides a standard way to look at the type
/// of a particular element in any of the sequence types supported by `ash::mpt`.
/// \param T The type for which to inspect the element type.
/// \param i The index of the element for which we want to retrieve the type.
template<std::size_t i, typename ...T>
struct element_type<i, pack<T...>> {
	/// Type of the `i`th element.
	using type = typename decltype(detail::nth_element_impl<i>::f(
			static_cast<wrap_type<T>*>(0)...))::type;
};
/// \brief Get the type of the `i`th element of a sequence-like type.
/// This template struct provides a standard way to look at the type
/// of a particular element in any of the sequence types supported by `ash::mpt`.
/// \param T The type for which to inspect the element type.
/// \param i The index of the element for which we want to retrieve the type.
template<std::size_t i, typename ...T>
struct element_type<i, std::tuple<T...>> {
	/// Type of the `i`th element.
	using type = typename std::tuple_element<i, std::tuple<T...>>::type;
};
/// \brief Get the type of the `i`th element of a sequence-like type.
/// This template struct provides a standard way to look at the type
/// of a particular element in any of the sequence types supported by `ash::mpt`.
/// \param T The type for which to inspect the element type.
/// \param i The index of the element for which we want to retrieve the type.
template<std::size_t i, typename T, T ...Nums>
struct element_type<i, integer_sequence<T, Nums...>> {
	/// Type of the `i`th element.
	using type = T;
};
/// \brief Get the type of the `i`th element of a sequence-like type.
/// This template struct provides a standard shortcut way to look at the type
/// of a particular element in any of the sequence types supported by `ash::mpt`.
/// \param T The type for which to inspect the element type.
/// \param i The index of the element for which we want to retrieve the type.
template<std::size_t i, typename T>
using element_type_t = typename element_type<i, T>::type;

/// \brief Convert an integer sequence value into a tuple value.
/// The result is a `std::tuple` with as many elements as the input sequence,
/// all of them of type `T`, set to the values of `Ints...`.
/// \return An appropriate tuple type, containing the values of `Ints...`.
template <typename T, T ...Ints>
constexpr auto as_tuple(integer_sequence<T, Ints...>)
-> decltype(std::make_tuple(Ints...)) {
	return std::make_tuple(Ints...);
}
/// \brief Convert an `pack` value into a tuple value.
/// The result is a `std::tuple` with as many elements as the input sequence,
/// every one of the same type as the same-index element of the `pack`.
///
/// Beware that some of the types embedded in a `pack` could be impossible to
/// instantiate (like abstract base classes, for instance), so generating values
/// of such a `std::tuple` type would be impossible. Consider using `wrap` and
/// `wrap_type` if needed in such a situation.
/// \return An appropriate tuple type, default-initialized.
template <typename ...T>
constexpr std::tuple<T...>
as_tuple(pack<T...>) {
	return {};
};
/// \brief Generate a tuple type with the same element types as a `pack`.
/// The result is a `std::tuple` with as many elements as the input sequence,
/// every one of the same type as the same-index element of the `pack`.
template <typename T>
struct as_tuple_type;
/// \brief Generate a tuple type with the same element types as a `pack`.
/// The result is a `std::tuple` with as many elements as the input sequence,
/// every one of the same type as the same-index element of the `pack`.
template <typename ...T>
struct as_tuple_type<pack<T...>> {
	using type = std::tuple<T...>;
};

/// \brief Convert a tuple value into a `pack` value.
/// The result is a `pack` with as many elements as the input `std::tuple`,
/// every one of the same type as the same-index element of the `std::tuple`.
/// \return An appropriate `pack` type with the types in the tuple.
template <typename ...T>
constexpr pack<T...>
as_pack(std::tuple<T...>) {
	return {};
};
/// \brief Convert an `integer_sequence` value into a `pack` value.
/// The result is a `pack` with as many elements as the input `integer_sequence`,
/// every one of the same type, which is the common type of the `integer_sequence`
/// elements.
/// \return An appropriate `pack` type with all elements of the integer type in the sequence.
template <typename T, T ...Ints>
constexpr auto as_pack(integer_sequence<T, Ints...>)
-> decltype(as_pack(std::make_tuple(Ints...))) {
	return as_pack(std::make_tuple(Ints...));
}

/// \brief Get the `i`th *value* from a `pack`.
/// This returns a `wrap_type` object wrapping the type
/// at index `i` in the `pack` type.
/// \param i The index to retrieve from the `pack` type sequence.
/// \param t The sequence from which to extract an element.
/// \return A `wrap_type` object for the type at index `i`.
template <std::size_t i, typename ...T>
constexpr wrap_type<element_type_t<i, pack<T...>>>
at(pack<T...>) {
	return {};
}
/// \brief Get the `i`th *value* from a `std::tuple`.
/// This returns a value of the `i`th element of the
/// tuple, of the corresponding type.
/// \param i The index to retrieve within the `std::tuple` object.
/// \param t The sequence from which to extract an element.
/// \return The element at index `i`.
template <std::size_t i, typename ...T>
constexpr element_type_t<i, std::tuple<T...>>&
at(std::tuple<T...>& t) {
	return std::get<i>(t);
}
/// \brief Get the `i`th *value* from a `std::tuple`.
/// This returns a value of the `i`th element of the
/// tuple, of the corresponding type.
/// \param i The index to retrieve within the `std::tuple` object.
/// \param t The sequence from which to extract an element.
/// \return The element at index `i`.
template <std::size_t i, typename ...T>
constexpr element_type_t<i, std::tuple<T...>> const&
at(const std::tuple<T...>& t) {
	return std::get<i>(t);
}
/// \brief Get the `i`th *value* from a `std::tuple`.
/// This returns a value of the `i`th element of the
/// tuple, of the corresponding type.
/// \param i The index to retrieve within the `std::tuple` object.
/// \param t The sequence from which to extract an element.
/// \return The element at index `i`.
template <std::size_t i, typename ...T>
constexpr element_type_t<i, std::tuple<T...>>&&
at(std::tuple<T...>&& t) {
	return std::get<i>(std::forward<std::tuple<T...>>(t));
}
/// \brief Get the `i`th *value* from an `integer_sequence` value.
/// This returns a value of the `i`th element of the
/// `integer_sequence`, of the sequence's integer type.
/// \param i The index to retrieve within the `integer_sequence` object.
/// \param is The sequence from which to extract an element.
/// \return The element at index `i`.
template <std::size_t i, typename T, T ...Nums>
constexpr T
at(integer_sequence<T, Nums...> is) {
	return at<i>(as_tuple(is));
}

namespace detail {

template <typename Val, typename F, typename ...Args, std::size_t ...Ints>
static constexpr auto
transform_one(Val&& v, F f, std::tuple<Args...>&& t, index_sequence<Ints...>)
-> decltype(f(std::forward<Val>(v), (std::forward<Args>(at<Ints>(t)))...)) {
	return f(std::forward<Val>(v), (std::forward<Args>(at<Ints>(t)))...);
}

template <typename T, typename F, std::size_t ...Ints, typename ...Args>
static void for_each(T&& v, index_sequence<Ints...>, F f, std::tuple<Args...>&& a) {
	using swallow = int[];
	(void)swallow{
			0,
			(void(transform_one(
					at<Ints>(std::forward<T>(v)),
					f,
					std::forward<std::tuple<Args...>>(a),
					make_index_sequence<sizeof...(Args)>{})), 0)...
	};
}

template <typename T, typename F, std::size_t ...Ints, typename ...Args>
static constexpr auto transform(T&& v, index_sequence<Ints...>, F f, std::tuple<Args...>&& a)
-> decltype(std::make_tuple(
		transform_one(
				at<Ints>(std::forward<T>(v)),
				f,
				std::forward<std::tuple<Args...>>(a),
				make_index_sequence<sizeof...(Args)>{})...
)) {
	return {
		transform_one(
				at<Ints>(std::forward<T>(v)),
				f,
				std::forward<std::tuple<Args...>>(a),
				make_index_sequence<sizeof...(Args)>{})...
	};
}
}  // namespace detail

/// \brief Run a functor for each element in a sequence type.
///
/// `for_each` performs compile-time iteration on sequences.
///
/// When calling:
///  `for_each(sequence_value, functor, args...);`
///
/// For each element in the sequence, functor will be called in this way
/// (for every `i` in `0 ... size`):
///
///  `functor(at<i>(sequence_value), args...);`
///
/// This function doesn't return any value.
/// \param v The sequence over which to iterate.
/// \param f The functor to call on every element.
/// \param args... Further arguments to forward to the functor call.
//
// For this other call:
//  map(pack<Types...>{}, functor, args...);
//
// A std::tuple with the return values is returned, but that
// requires the functor to have a non-void return type in each
// case.
//
// Variants taking tuples work the same way, but the tuple element
// value is passed to the functor instead of a tagging pack type.
template <typename T, typename F, typename ...Args>
void for_each(T&& v, F f, Args&& ...args) {
	detail::for_each(
			std::forward<T>(v),
			make_index_sequence<size<T>::value>{},
			f,
			std::forward_as_tuple(std::forward<Args>(args)...));
}

/// \brief Run a functor for each element in a sequence type and return the results.
///
/// `transform` performs compile-time iteration on sequences returning the
/// resulting values.
///
/// When calling:
///  `transform(sequence_value, functor, args...);`
///
/// For each element in the sequence, functor will be called in this way
/// (for every `i` in `0 ... size`):
///
///  `functor(at<i>(sequence_value), args...);`
///
/// The returned result will be a `std::tuple` composed of all the values returned
/// by the successive calls of the functor (each one can have a distinct type).
/// \param v The sequence over which to iterate.
/// \param f The functor to call on every element.
/// \param args... Further arguments to forward to the functor call.
/// \return A `std::tuple` of appropriate type to contain the results of every call.
template <typename T, typename F, typename ...Args>
constexpr auto
transform(T&& v, F f, Args&& ...args)
-> decltype(detail::transform(
		std::forward<T>(v),
		make_index_sequence<size<T>::value>{},
		f,
		std::forward_as_tuple(std::forward<Args>(args)...))) {
	return detail::transform(
			std::forward<T>(v),
			make_index_sequence<size<T>::value>{},
			f,
			std::forward_as_tuple(std::forward<Args>(args)...));
}

/// Return a new tuple containing a subset of the fields as determined by the passed index sequence.
/// \param t The sequence to subset.
/// \param ...Idx The indices to extract.
/// \return A sliced sequence containing just the elements specified by the indices.
template <typename ...T, std::size_t ...Idx>
constexpr std::tuple<typename std::tuple_element<Idx, std::tuple<T...>>::type...>
subset(std::tuple<T...>&t, index_sequence<Idx...>) {
	return std::make_tuple(at<Idx>(t)...);
}
/// Return a new tuple containing a subset of the fields as determined by the passed index sequence.
/// \param t The sequence to subset.
/// \param ...Idx The indices to extract.
/// \return A sliced sequence containing just the elements specified by the indices.
template <typename ...T, std::size_t ...Idx>
constexpr std::tuple<typename std::tuple_element<Idx, const std::tuple<T...>>::type...>
subset(const std::tuple<T...>&t, index_sequence<Idx...>) {
	return std::make_tuple(at<Idx>(t)...);
}
/// Return a new tuple containing a subset of the fields as determined by the passed index sequence.
/// \param t The sequence to subset.
/// \param ...Idx The indices to extract.
/// \return A sliced sequence containing just the elements specified by the indices.
template <typename ...T, std::size_t ...Idx>
constexpr std::tuple<typename std::tuple_element<Idx, const std::tuple<T...>>::type...>
subset(std::tuple<T...>&&t, index_sequence<Idx...>) {
	return std::make_tuple(at<Idx>(t)...);
}
/// Return a new pack containing a subset of the types as determined by the passed index sequence.
/// \param t The sequence to subset.
/// \param ...Idx The indices to extract.
/// \return A sliced sequence containing just the elements specified by the indices.
template <typename ...T, std::size_t ...Idx>
constexpr pack<element_type_t<Idx, pack<T...>>...>
subset(pack<T...>, index_sequence<Idx...>) {
	return {};
}
/// Return a new integer sequence containing a subset of the integers determined by the passed index sequence.
/// \param t The sequence to subset.
/// \param ...Idx The indices to extract.
/// \return A sliced sequence containing just the elements specified by the indices.
template <typename T, T ...Nums, std::size_t ...Idx>
constexpr integer_sequence<T, at<Idx>(integer_sequence<T, Nums...>{})...>
subset(integer_sequence<T, Nums...>, index_sequence<Idx...>) {
	return {};
}

/// Return a subsequence based on a semi-open index range.
/// \param t The sequence from which to extract a range.
/// \param Begin The first index to extract.
/// \param End The index following the last one to extract.
/// \return A sliced sequence containing just the elements in the range `Begin .. End`.
template <std::size_t Begin, std::size_t End, typename T>
constexpr auto range(T&& t)
-> decltype(subset(std::forward<T>(t), make_index_sequence<(End - Begin)>{} + make_constant_index_sequence<(End - Begin), Begin>{})) {
	return subset(std::forward<T>(t), make_index_sequence<(End - Begin)>{} + make_constant_index_sequence<(End - Begin), Begin>{});
}

/// Return the head element of a sequence.
/// \param t The sequence from which to extract the head element.
/// \return The result of calling `at<0>` over the sequence.
template <typename T>
constexpr auto head(T&& t)
-> decltype(at<0>(std::forward<T>(t))) {
	return at<0>(std::forward<T>(t));
}

/// Return the tail subsequence of a sequence.
/// \param t The sequence from which to remove the head element.
/// \return The original sequence with the head removed.
template <typename T>
constexpr auto tail(T&& t)
-> decltype(range<1, size<T>::value>(t)) {
	return range<1, size<T>::value>(t);
}

namespace detail {
template <std::size_t N>
struct accumulate_helper {
	template <typename I, typename T, typename O>
	static constexpr auto accumulate_internal(I&& a, T&& t, O o)
	-> decltype(accumulate(o(std::forward<I>(a), head(std::forward<T>(t))), tail(std::forward<T>(t)), o)) {
		return accumulate(o(std::forward<I>(a), head(std::forward<T>(t))), tail(std::forward<T>(t)), o);
	}
};
template <>
struct accumulate_helper<0> {
	template <typename I, typename T, typename O>
	static constexpr auto	accumulate_internal(I&& a, T&& t, O o)
	-> I {
		return a;
	}
};
}  // namespace detail

// Accumulate elements with an initial value using the given operator.
template <typename I, typename T, typename O>
constexpr auto accumulate(I&& a, T&& t, O o)
-> decltype(detail::accumulate_helper<size<T>::value>::accumulate_internal(std::forward<I>(a), std::forward<T>(t), o)) {
	return detail::accumulate_helper<size<T>::value>::accumulate_internal(std::forward<I>(a), std::forward<T>(t), o);
}

// Wrap and unwrap pack types. Wrapped types can be used in contexts where we
// are worried that type instantiation can happen. The wrapped types are inert.
template <typename ...T>
constexpr pack<wrap_type<T>...>
wrap(pack<T...>) {
	return {};
}

template <typename ...T>
constexpr pack<T...>
unwrap(pack<wrap_type<T>...>) {
	return {};
};


// Helpers to determine if a type is a pack / tuple / integer_sequence.
template <typename T>
struct is_tuple: public std::false_type {};

template <typename ...T>
struct is_tuple<std::tuple<T...>>: public std::true_type {};

template <typename T>
struct is_pack: public std::false_type {};

template <typename ...T>
struct is_pack<pack<T...>>: public std::true_type {};

template <typename T>
struct is_integer_sequence: public std::false_type {};

template <typename T, T ...Nums>
struct is_integer_sequence<integer_sequence<T, Nums...>>: public std::true_type {};

// Forward cat for tuples to tuple_cat.
template <typename ...Args, typename Enable = typename std::enable_if<conjunction<is_tuple<typename std::remove_cv<typename std::remove_reference<Args>::type>::type>...>::value>::type>
constexpr auto cat(Args&&... args)
-> decltype(std::tuple_cat(std::forward<Args>(args)...)) {
	return std::tuple_cat(std::forward<Args>(args)...);
}

// Handle cat pack by wrapping the types and transforming into a tuple.
template <typename ...Args, typename Enable = typename std::enable_if<conjunction<is_pack<typename std::remove_cv<typename std::remove_reference<Args>::type>::type>...>::value>::type>
constexpr auto cat(Args... args)
-> decltype(unwrap(as_pack(std::tuple_cat(as_tuple(wrap(args))...)))) {
	return unwrap(as_pack(std::tuple_cat(as_tuple(wrap(args))...)));
}

namespace detail {
template <typename ...TN>
struct integer_sequence_cat_helper;

template <typename T1, typename T2, typename ...TN>
struct integer_sequence_cat_helper<T1, T2, TN...> {
	using rest_type = typename integer_sequence_cat_helper<T2, TN...>::type;
	using type = typename join_integer_sequences<T1, rest_type, false>::type;
};

template <typename T1>
struct integer_sequence_cat_helper<T1> {
	using type = T1;
};
}  // namespace detail

// Handle cat for integer sequences.
// We need at least one to determine the integer type, and all of them must match.
// So a workaround for the empty case is to provide a zero-length "start" as the
// first argument.
template <typename ...Args, typename Enable = typename std::enable_if<conjunction<is_integer_sequence<typename std::remove_cv<typename std::remove_reference<Args>::type>::type>...>::value>::type>
constexpr auto cat(Args... args)
-> 	typename detail::integer_sequence_cat_helper<Args...>::type {
	return {};
}

// Check whether the passed wrapped type is T.
template <typename T>
struct is_wrapped {
	constexpr bool operator()(wrap_type<T>) {
		return true;
	}
	constexpr bool operator()(...) {
		return false;
	}
};

}  // namespace mpt

}
// namespace ash

#endif /* ASH_MPT_H_ */
