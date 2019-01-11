#ifndef INCLUDE_ASH_MPT_H_
#define INCLUDE_ASH_MPT_H_

#include <algorithm>
#include <cstddef>
#include <tuple>
#include <type_traits>
#include <utility>

namespace ash {

/// Type-based meta-programming toolkit.
namespace mpt {

/// \brief Define a whole set of arithmetic operators.
/// Macro to define the whole set of arithmetic operations given a pair of
/// macros
/// for unary and binary operations.
/// \param CREATE_UNARY_OPERATOR Macro to be used for defining a unary operator
/// in terms of the `NAME` and the operation (`OP`).
/// \param CREATE_BINARY_OPERATOR Macro to be used for defining a binary
/// operator in terms of the `NAME` and the operation (`OP`).
#define ASH_CREATE_OPERATOR_HIERARCHY(CREATE_UNARY_OPERATOR,  \
                                      CREATE_BINARY_OPERATOR) \
  CREATE_UNARY_OPERATOR(negate, -);                           \
  CREATE_UNARY_OPERATOR(logical_not, !);                      \
  CREATE_UNARY_OPERATOR(bit_not, ~);                          \
  CREATE_BINARY_OPERATOR(plus, +);                            \
  CREATE_BINARY_OPERATOR(minus, -);                           \
  CREATE_BINARY_OPERATOR(multiplies, *);                      \
  CREATE_BINARY_OPERATOR(divides, /);                         \
  CREATE_BINARY_OPERATOR(modulus, %);                         \
  CREATE_BINARY_OPERATOR(equal_to, ==);                       \
  CREATE_BINARY_OPERATOR(not_equal_to, !=);                   \
  CREATE_BINARY_OPERATOR(greater, >);                         \
  CREATE_BINARY_OPERATOR(less, <);                            \
  CREATE_BINARY_OPERATOR(greater_equal, >=);                  \
  CREATE_BINARY_OPERATOR(less_equal, <=);                     \
  CREATE_BINARY_OPERATOR(logical_and, &&);                    \
  CREATE_BINARY_OPERATOR(logical_or, ||);                     \
  CREATE_BINARY_OPERATOR(bit_and, &);                         \
  CREATE_BINARY_OPERATOR(bit_or, |);                          \
  CREATE_BINARY_OPERATOR(bit_xor, ^);

/// \brief Define a whole set of arithmetic operators plus the identity.
/// Macro to define the whole set of arithmetic operations (plus the identitiy
/// operation)
/// given a pair of macros for unary and binary operations.
///
/// Notice that the identity operation is special in the sense that the value of
/// `OP` for it
/// is the empty string, and thus it's not suitable to define a `operator OP ()`
/// function.
///
/// \param CREATE_UNARY_OPERATOR Macro to be used for defining a unary operator
/// in terms of the `NAME` and the operation (`OP`).
/// \param CREATE_BINARY_OPERATOR Macro to be used for defining a binary
/// operator in terms of the `NAME` and the operation (`OP`).
#define ASH_CREATE_OPERATOR_HIERARCHY_WITH_IDENTITY(CREATE_UNARY_OPERATOR,     \
                                                    CREATE_BINARY_OPERATOR)    \
  ASH_CREATE_OPERATOR_HIERARCHY(CREATE_UNARY_OPERATOR, CREATE_BINARY_OPERATOR) \
  CREATE_UNARY_OPERATOR(identity, /**/);

/// Define unary arithmetic operation functors on arbitrary types.
#define ASH_CREATE_UNARY_OPERATOR(NAME, OP) \
  struct NAME {                             \
    template <typename T>                   \
    constexpr auto operator()(T v) {        \
      return OP v;                          \
    }                                       \
  };
/// Define binary arithmetic operation functors on arbitrary types.
#define ASH_CREATE_BINARY_OPERATOR(NAME, OP) \
  struct NAME {                              \
    template <typename T, typename U>        \
    constexpr auto operator()(T v, U w) {    \
      return v OP w;                         \
    }                                        \
  };
/// Create a complete set of arithmetic operation functors on arbitrary types.
ASH_CREATE_OPERATOR_HIERARCHY_WITH_IDENTITY(ASH_CREATE_UNARY_OPERATOR,
                                            ASH_CREATE_BINARY_OPERATOR);

/// \brief Types representing sequences of homogeneously typed integers.
/// An `integer_sequence<T, ints...>` type represents a compile-time sequence of
/// a variable number of integers (given by `ints...`), all of the same type
/// `T`.
/// \param T The type of all the integers.
/// \param ints... The list of `T`-typed integers to encode in the type.
template <typename T, T... ints>
struct integer_sequence {
  /// The type's own type.
  using type = integer_sequence<T, ints...>;

  /// The type of the integers in the sequence.
  using value_type = T;

  /// The number of integers in the sequence.
  static constexpr std::size_t size = sizeof...(ints);

 private:
  template <typename O>
  struct unary_op {
    using result_type = std::remove_cv_t<decltype(O()(T()))>;
    using type = integer_sequence<result_type, (O()(ints))...>;
  };

  template <typename U, typename O>
  struct binary_op;

  template <typename U, U... ints_u, typename O>
  struct binary_op<integer_sequence<U, ints_u...>, O> {
    using result_type = std::remove_cv_t<decltype(O()(T(), U()))>;
    using type = integer_sequence<result_type, (O()(ints, ints_u))...>;
  };

 public:
/// Type-based unary operator generator macro.
#define ASH_INT_SEQ_TYPE_UNARY_OP(NAME, OP)    \
  struct NAME : unary_op<::ash::mpt::NAME> {}; \
  using NAME##_t = typename NAME::type;
/// Type-based binary operator generator macro.
#define ASH_INT_SEQ_TYPE_BINARY_OP(NAME, OP)       \
  template <typename U>                            \
  struct NAME : binary_op<U, ::ash::mpt::NAME> {}; \
  template <typename U>                            \
  using NAME##_t = typename NAME<U>::type;
  /// Type-based operator hierarchy.
  ASH_CREATE_OPERATOR_HIERARCHY_WITH_IDENTITY(ASH_INT_SEQ_TYPE_UNARY_OP,
                                              ASH_INT_SEQ_TYPE_BINARY_OP);

/// Value-based unary operator generator macro.
#define ASH_INT_SEQ_VALUE_UNARY_OP(NAME, OP) \
  constexpr NAME##_t operator OP() { return {}; }
/// Value-based binary operator generator macro.
#define ASH_INT_SEQ_VALUE_BINARY_OP(NAME, OP) \
  template <typename U>                       \
  constexpr NAME##_t<U> operator OP(U) {      \
    return {};                                \
  }
  // Value-based operator hierarchy.
  ASH_CREATE_OPERATOR_HIERARCHY(ASH_INT_SEQ_VALUE_UNARY_OP,
                                ASH_INT_SEQ_VALUE_BINARY_OP);
};

namespace detail {

template <typename Is, typename Js, bool adjust_values = false>
struct join_integer_sequences;

template <typename T, T... is, T... js, bool adjust_values>
struct join_integer_sequences<integer_sequence<T, is...>,
                              integer_sequence<T, js...>, adjust_values> {
  using type =
      integer_sequence<T, is..., ((adjust_values ? sizeof...(is) : 0) + js)...>;
};

template <typename T, std::size_t n>
struct make_integer_sequence
    : join_integer_sequences<typename make_integer_sequence<T, n / 2>::type,
                             typename make_integer_sequence<T, n - n / 2>::type,
                             true> {};

template <typename T>
struct make_integer_sequence<T, 1> {
  using type = integer_sequence<T, 0>;
};

template <typename T>
struct make_integer_sequence<T, 0> {
  using type = integer_sequence<T>;
};

template <typename T, std::size_t n, T v>
struct make_constant_integer_sequence
    : join_integer_sequences<
          typename make_constant_integer_sequence<T, n / 2, v>::type,
          typename make_constant_integer_sequence<T, n - n / 2, v>::type> {};

template <typename T, T v>
struct make_constant_integer_sequence<T, 1, v> {
  using type = integer_sequence<T, v>;
};

template <typename T, T v>
struct make_constant_integer_sequence<T, 0, v> {
  using type = integer_sequence<T>;
};

}  // namespace detail

/// \brief Generate a sequence of integers.
/// The integers are consecutive, in the range `0 .. n-1`.
/// \param T The type of the integers.
/// \param n The total number of integers the sequence will have.
template <typename T, std::size_t n>
using make_integer_sequence =
    typename detail::make_integer_sequence<T, n>::type;

/// \brief Generate a repeated sequence of the same integer.
/// This will generate `n` integers with the value `v` of type `T`.
/// \param T The type of the integers.
/// \param n The total number of integers the sequence will have.
/// \param v The value of the integers.
template <typename T, std::size_t n, T v>
using make_constant_integer_sequence =
    typename detail::make_constant_integer_sequence<T, n, v>::type;

/// \brief Types representing sequences of `std::size_t` which can be used as
/// indices.
/// An `index_sequence<ints...>` type represents a compile-time sequence of
/// a variable number of `std::size_t` values (given by `ints...`).
/// \param ints... The list of `std::size_t`-typed integers to encode in the
/// type.
template <std::size_t... ints>
using index_sequence = integer_sequence<std::size_t, ints...>;
/// \brief Generate a sequence of indices.
/// The integers are consecutive, in the range `0 .. n-1`, and of `std::size_t`
/// type.
/// \param n The total number of integers the sequence will have.
template <std::size_t n>
using make_index_sequence = make_integer_sequence<std::size_t, n>;
/// \brief Generate a repeated sequence of the same index.
/// This will generate `n` indices with the value `v` of type `std::size_t`.
/// \param n The total number of indices the sequence will have.
/// \param v The value of the indices.
template <std::size_t n, std::size_t v>
using make_constant_index_sequence =
    make_constant_integer_sequence<std::size_t, n, v>;

/// \brief Wrap one type so that it can be passed around without constructing
/// any instance of it.
/// `wrap_type<T>` encodes a single type in a class whose objects can be passed
/// around at runtime
/// without having to copy any data.
/// \param T The type to wrap.
template <typename T>
struct wrap_type {
  /// Actual type wrapped by the `wrap_type` specialization.
  using type = T;
};

/// \brief Template class to represent sequences of types.
/// `pack` structs contain no data; all the information they contain is the list
/// of types represented by `T...`.
/// \param T... List of types to pack.
template <typename... T>
struct pack {};

/// \brief Template class to represent sequences of constexpr values.
/// `value_pack` structs contain no data; all the information they contain is
/// the static list of values represented by `v...`.
/// \param v... List of values to pack, possibly of different types.
template <auto... v>
struct value_pack {};

namespace detail {
// Get the number of types.
template <typename T>
struct size;

template <typename... T>
struct size<pack<T...>> {
  static constexpr std::size_t value = sizeof...(T);
};

template <auto... v>
struct size<value_pack<v...>> {
  static constexpr std::size_t value = sizeof...(v);
};

template <typename... T>
struct size<std::tuple<T...>> {
  static constexpr std::size_t value = sizeof...(T);
};

template <typename T, T... ints>
struct size<integer_sequence<T, ints...>> {
  static constexpr std::size_t value = sizeof...(ints);
};
}  // namespace detail

/// \brief Get the size of a sequence-like type.
/// This template struct provides a standard way to look at the size (number of
/// elements)
/// in any of the sequence types supported by `ash::mpt`.
/// \param T The type from which to obtain the size.
template <typename T>
struct size
    : std::integral_constant<
          std::size_t,
          detail::size<std::remove_cv_t<std::remove_reference_t<T>>>::value> {};

template <typename T>
inline constexpr std::size_t size_v = size<T>::value;

namespace detail {

template <std::size_t n, typename = make_index_sequence<n>>
struct nth_element_impl;

template <std::size_t n, std::size_t... ignore>
struct nth_element_impl<n, index_sequence<ignore...>> {
  template <typename Tn>
  static Tn f(decltype(reinterpret_cast<void*>(ignore))..., Tn*, ...);
};

}  // namespace detail

/// \brief Get the type of the `i`th element of a sequence-like type.
/// This template struct provides a standard way to look at the type
/// of a particular element in any of the sequence types supported by
/// `ash::mpt`.
/// \param T The type for which to inspect the element type.
/// \param i The index of the element for which we want to retrieve the type.
template <std::size_t i, typename T>
struct element_type;
/// \brief Get the type of the `i`th element of a sequence-like type.
/// This template struct provides a standard way to look at the type
/// of a particular element in any of the sequence types supported by
/// `ash::mpt`.
/// \param T The type for which to inspect the element type.
/// \param i The index of the element for which we want to retrieve the type.
template <std::size_t i, typename... T>
struct element_type<i, pack<T...>> {
  /// Type of the `i`th element.
  using type = decltype(
      detail::nth_element_impl<i>::f(static_cast<wrap_type<T>*>(0)...));
};
/// \brief Get the type of the `i`th element of a sequence-like type.
/// This template struct provides a standard way to look at the type
/// of a particular element in any of the sequence types supported by
/// `ash::mpt`.
/// \param T The type for which to inspect the element type.
/// \param i The index of the element for which we want to retrieve the type.
template <std::size_t i, typename... T>
struct element_type<i, std::tuple<T...>> {
  /// Type of the `i`th element.
  using type = typename std::tuple_element<i, std::tuple<T...>>::type;
};
/// \brief Get the type of the `i`th element of a sequence-like type.
/// This template struct provides a standard way to look at the type
/// of a particular element in any of the sequence types supported by
/// `ash::mpt`.
/// \param T The type for which to inspect the element type.
/// \param i The index of the element for which we want to retrieve the type.
template <std::size_t i, typename T, T... ints>
struct element_type<i, integer_sequence<T, ints...>> {
  /// Type of the `i`th element.
  using type = T;
};
/// \brief Get the type of the `i`th element of a sequence-like type.
/// This template struct provides a standard shortcut way to look at the type
/// of a particular element in any of the sequence types supported by
/// `ash::mpt`.
/// \param T The type for which to inspect the element type.
/// \param i The index of the element for which we want to retrieve the type.
template <std::size_t i, typename T>
using element_type_t = typename element_type<i, T>::type;

/// \brief Convert an integer sequence value into a tuple value.
/// The result is a `std::tuple` with as many elements as the input sequence,
/// all of them of type `T`, set to the values of `ints...`.
/// \return An appropriate tuple type, containing the values of `ints...`.
template <typename T, T... ints>
constexpr auto as_tuple(integer_sequence<T, ints...>)
    -> decltype(std::make_tuple(ints...)) {
  return std::make_tuple(ints...);
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
template <typename... T>
constexpr std::tuple<T...> as_tuple(pack<T...>) {
  return {};
}

/// \brief Convert a tuple value into a `pack` value.
/// The result is a `pack` with as many elements as the input `std::tuple`,
/// every one of the same type as the same-index element of the `std::tuple`.
/// \return An appropriate `pack` type with the types in the tuple.
template <typename... T>
constexpr pack<T...> as_pack(std::tuple<T...>) {
  return {};
}
/// \brief Convert an `integer_sequence` value into a `pack` value.
/// The result is a `pack` with as many elements as the input
/// `integer_sequence`,
/// every one of the same type, which is the common type of the
/// `integer_sequence`
/// elements.
/// \return An appropriate `pack` type with all elements of the integer type in
/// the sequence.
template <typename T, T... ints>
constexpr auto as_pack(integer_sequence<T, ints...>)
    -> decltype(as_pack(std::make_tuple(ints...))) {
  return as_pack(std::make_tuple(ints...));
}

/// \brief Get the `i`th *value* from a `pack`.
/// This returns a `wrap_type` object wrapping the type
/// at index `i` in the `pack` type.
/// \param i The index to retrieve from the `pack` type sequence.
/// \param t The sequence from which to extract an element.
/// \return A `wrap_type` object for the type at index `i`.
template <std::size_t i, typename... T>
constexpr element_type_t<i, pack<T...>> at(pack<T...>) {
  return {};
}
/// \brief Get the `i`th *value* from a `std::tuple`.
/// This returns a value of the `i`th element of the
/// tuple, of the corresponding type.
/// \param i The index to retrieve within the `std::tuple` object.
/// \param t The sequence from which to extract an element.
/// \return The element at index `i`.
template <std::size_t i, typename... T>
constexpr element_type_t<i, std::tuple<T...>>& at(std::tuple<T...>& t) {
  return std::get<i>(t);
}
/// \brief Get the `i`th *value* from a `std::tuple`.
/// This returns a value of the `i`th element of the
/// tuple, of the corresponding type.
/// \param i The index to retrieve within the `std::tuple` object.
/// \param t The sequence from which to extract an element.
/// \return The element at index `i`.
template <std::size_t i, typename... T>
constexpr element_type_t<i, std::tuple<T...>> const& at(
    const std::tuple<T...>& t) {
  return std::get<i>(t);
}
/// \brief Get the `i`th *value* from a `std::tuple`.
/// This returns a value of the `i`th element of the
/// tuple, of the corresponding type.
/// \param i The index to retrieve within the `std::tuple` object.
/// \param t The sequence from which to extract an element.
/// \return The element at index `i`.
template <std::size_t i, typename... T>
constexpr element_type_t<i, std::tuple<T...>>&& at(std::tuple<T...>&& t) {
  return std::get<i>(std::forward<std::tuple<T...>>(t));
}
/// \brief Get the `i`th *value* from an `integer_sequence` value.
/// This returns a value of the `i`th element of the
/// `integer_sequence`, of the sequence's integer type.
/// \param i The index to retrieve within the `integer_sequence` object.
/// \param is The sequence from which to extract an element.
/// \return The element at index `i`.
template <std::size_t i, typename T, T... ints>
constexpr T at(integer_sequence<T, ints...> is) {
  return at<i>(as_tuple(is));
}

namespace detail {
template <typename Val, typename F, typename... Args, std::size_t... ints>
static constexpr auto transform_one(Val&& v, F f, std::tuple<Args...>&& t,
                                    index_sequence<ints...>)
    -> decltype(f(std::forward<Val>(v), (std::forward<Args>(at<ints>(t)))...)) {
  return f(std::forward<Val>(v), (std::forward<Args>(at<ints>(t)))...);
}

template <typename T, typename F, std::size_t... ints, typename... Args>
static void for_each(T&& v, index_sequence<ints...>, F f,
                     std::tuple<Args...>&& a) {
  using swallow = int[];
  (void)swallow{0, (void(transform_one(at<ints>(std::forward<T>(v)), f,
                                       std::forward<std::tuple<Args...>>(a),
                                       make_index_sequence<sizeof...(Args)>{})),
                    0)...};
}

template <typename T, typename F, std::size_t... ints, typename... Args>
static constexpr auto transform(T&& v, index_sequence<ints...>, F f,
                                std::tuple<Args...>&& a)
    -> decltype(std::make_tuple(transform_one(
        at<ints>(std::forward<T>(v)), f, std::forward<std::tuple<Args...>>(a),
        make_index_sequence<sizeof...(Args)>{})...)) {
  return std::make_tuple(transform_one(
      at<ints>(std::forward<T>(v)), f, std::forward<std::tuple<Args...>>(a),
      make_index_sequence<sizeof...(Args)>{})...);
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
template <typename T, typename F, typename... Args>
void for_each(T&& v, F f, Args&&... args) {
  detail::for_each(std::forward<T>(v), make_index_sequence<size_v<T>>{}, f,
                   std::forward_as_tuple(std::forward<Args>(args)...));
}

/// \brief Run a functor for each element in a sequence type and return the
/// results.
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
/// The returned result will be a `std::tuple` composed of all the values
/// returned
/// by the successive calls of the functor (each one can have a distinct type).
/// \param v The sequence over which to iterate.
/// \param f The functor to call on every element.
/// \param args... Further arguments to forward to the functor call.
/// \return A `std::tuple` of appropriate type to contain the results of every
/// call.
template <typename T, typename F, typename... Args>
constexpr auto transform(T&& v, F f, Args&&... args) -> decltype(
    detail::transform(std::forward<T>(v), make_index_sequence<size_v<T>>{}, f,
                      std::forward_as_tuple(std::forward<Args>(args)...))) {
  return detail::transform(std::forward<T>(v), make_index_sequence<size_v<T>>{},
                           f,
                           std::forward_as_tuple(std::forward<Args>(args)...));
}

/// Return a new tuple containing a subset of the fields as determined by the
/// passed index sequence.
/// \param t The sequence to subset.
/// \param ...idxs The indices to extract.
/// \return A sliced sequence containing just the elements specified by the
/// indices.
template <typename... T, std::size_t... idxs>
constexpr std::tuple<
    typename std::tuple_element<idxs, std::tuple<T...>>::type...>
subset(std::tuple<T...>& t, index_sequence<idxs...>) {
  return std::make_tuple(at<idxs>(t)...);
}
/// Return a new tuple containing a subset of the fields as determined by the
/// passed index sequence.
/// \param t The sequence to subset.
/// \param ...ints The indices to extract.
/// \return A sliced sequence containing just the elements specified by the
/// indices.
template <typename... T, std::size_t... idxs>
constexpr std::tuple<
    typename std::tuple_element<idxs, const std::tuple<T...>>::type...>
subset(const std::tuple<T...>& t, index_sequence<idxs...>) {
  return std::make_tuple(at<idxs>(t)...);
}
/// Return a new tuple containing a subset of the fields as determined by the
/// passed index sequence.
/// \param t The sequence to subset.
/// \param ...ints The indices to extract.
/// \return A sliced sequence containing just the elements specified by the
/// indices.
template <typename... T, std::size_t... idxs>
constexpr std::tuple<
    typename std::tuple_element<idxs, const std::tuple<T...>>::type...>
subset(std::tuple<T...>&& t, index_sequence<idxs...>) {
  return std::make_tuple(at<idxs>(t)...);
}
/// Return a new pack containing a subset of the types as determined by the
/// passed index sequence.
/// \param t The sequence to subset.
/// \param ...ints The indices to extract.
/// \return A sliced sequence containing just the elements specified by the
/// indices.
template <typename... T, std::size_t... idxs>
constexpr pack<typename element_type_t<idxs, pack<T...>>::type...> subset(
    pack<T...>, index_sequence<idxs...>) {
  return {};
}
/// Return a new integer sequence containing a subset of the integers determined
/// by the passed index sequence.
/// \param t The sequence to subset.
/// \param ...idxs The indices to extract.
/// \return A sliced sequence containing just the elements specified by the
/// indices.
template <typename T, T... ints, std::size_t... idxs>
constexpr integer_sequence<T, at<idxs>(integer_sequence<T, ints...>{})...>
subset(integer_sequence<T, ints...>, index_sequence<idxs...>) {
  return {};
}

/// Return a subsequence based on a semi-open index range.
/// \param t The sequence from which to extract a range.
/// \param begin The first index to extract.
/// \param end The index following the last one to extract.
/// \return A sliced sequence containing just the elements in the range `begin
/// .. end`.
template <std::size_t begin, std::size_t end, typename T>
constexpr auto range(T&& t) -> decltype(
    subset(std::forward<T>(t),
           make_index_sequence<(end - begin)>{} +
               make_constant_index_sequence<(end - begin), begin>{})) {
  return subset(std::forward<T>(t),
                make_index_sequence<(end - begin)>{} +
                    make_constant_index_sequence<(end - begin), begin>{});
}

/// Return the head element of a sequence.
/// \param t The sequence from which to extract the head element.
/// \return The result of calling `at<0>` over the sequence.
template <typename T>
constexpr auto head(T&& t) -> decltype(at<0>(std::forward<T>(t))) {
  return at<0>(std::forward<T>(t));
}

/// Return the tail subsequence of a sequence.
/// \param t The sequence from which to remove the head element.
/// \return The original sequence with the head removed.
template <typename T>
constexpr auto tail(T&& t) -> decltype(range<1, size_v<T>>(t)) {
  return range<1, size_v<T>>(t);
}

namespace detail {
template <std::size_t n>
struct accumulate_helper {
  template <typename I, typename T, typename O, typename... Args>
  static constexpr auto accumulate_internal(I&& a, T&& t, O o, Args&&... args)
      -> decltype(accumulate(o(std::forward<I>(a), head(std::forward<T>(t)),
                               std::forward<Args>(args)...),
                             tail(std::forward<T>(t)), o,
                             std::forward<Args>(args)...)) {
    return accumulate(o(std::forward<I>(a), head(std::forward<T>(t)),
                        std::forward<Args>(args)...),
                      tail(std::forward<T>(t)), o, std::forward<Args>(args)...);
  }
};
template <>
struct accumulate_helper<0> {
  template <typename I, typename T, typename O, typename... Args>
  static constexpr auto accumulate_internal(I&& a, T&& t, O o, Args&&... args)
      -> I {
    return a;
  }
};
}  // namespace detail

// Accumulate elements with an initial value using the given operator.
template <typename I, typename T, typename O, typename... Args>
constexpr auto accumulate(I&& a, T&& t, O o, Args&&... args)
    -> decltype(detail::accumulate_helper<size_v<T>>::accumulate_internal(
        std::forward<I>(a), std::forward<T>(t), o,
        std::forward<Args>(args)...)) {
  return detail::accumulate_helper<size_v<T>>::accumulate_internal(
      std::forward<I>(a), std::forward<T>(t), o, std::forward<Args>(args)...);
}

// Wrap and unwrap pack types. Wrapped types can be used in contexts where we
// are worried that type instantiation can happen. The wrapped types are inert.
template <typename... T>
constexpr pack<wrap_type<T>...> wrap(pack<T...>) {
  return {};
}

template <typename... T>
constexpr pack<T...> unwrap(pack<wrap_type<T>...>) {
  return {};
}

// Helpers to determine if a type is a pack / tuple / integer_sequence.
namespace detail {
template <typename T>
struct is_tuple : public std::false_type {};

template <typename... T>
struct is_tuple<std::tuple<T...>> : public std::true_type {};

template <typename T>
struct is_pack : public std::false_type {};

template <typename... T>
struct is_pack<pack<T...>> : public std::true_type {};

template <typename T>
struct is_value_pack : public std::false_type {};

template <auto... v>
struct is_value_pack<value_pack<v...>> : public std::true_type {};

template <typename T>
struct is_integer_sequence : public std::false_type {};

template <typename T, T... ints>
struct is_integer_sequence<integer_sequence<T, ints...>>
    : public std::true_type {};
}  // namespace detail

template <typename T>
using is_tuple = detail::is_tuple<std::remove_cv_t<std::remove_reference_t<T>>>;
template <typename T>
static constexpr bool is_tuple_v = is_tuple<T>::value;

template <typename T>
using is_pack = detail::is_pack<std::remove_cv_t<std::remove_reference_t<T>>>;
template <typename T>
static constexpr bool is_pack_v = is_pack<T>::value;

template <typename T>
using is_value_pack =
    detail::is_value_pack<std::remove_cv_t<std::remove_reference_t<T>>>;
template <typename T>
static constexpr bool is_value_pack_v = is_value_pack<T>::value;

template <typename T>
using is_integer_sequence =
    detail::is_integer_sequence<std::remove_cv_t<std::remove_reference_t<T>>>;
template <typename T>
static constexpr bool is_integer_sequence_v = is_integer_sequence<T>::value;

// Forward cat for tuples to tuple_cat.
template <typename... Args,
          typename Enable = typename std::enable_if<std::conjunction<
              is_tuple<typename std::remove_cv<typename std::remove_reference<
                  Args>::type>::type>...>::value>::type>
constexpr auto cat(Args&&... args)
    -> decltype(std::tuple_cat(std::forward<Args>(args)...)) {
  return std::tuple_cat(std::forward<Args>(args)...);
}

// Handle cat pack by wrapping the types and transforming into a tuple.
template <typename... Args,
          typename Enable = typename std::enable_if<std::conjunction<
              is_pack<typename std::remove_cv<typename std::remove_reference<
                  Args>::type>::type>...>::value>::type>
constexpr auto cat(Args... args)
    -> decltype(unwrap(as_pack(std::tuple_cat(as_tuple(wrap(args))...)))) {
  return unwrap(as_pack(std::tuple_cat(as_tuple(wrap(args))...)));
}

namespace detail {
template <typename... TN>
struct integer_sequence_cat_helper;

template <typename T1, typename T2, typename... TN>
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
// We need at least one to determine the integer type, and all of them must
// match.
// So a workaround for the empty case is to provide a zero-length "start" as the
// first argument.
template <typename... Args,
          typename Enable = typename std::enable_if<
              std::conjunction<is_integer_sequence<typename std::remove_cv<
                  typename std::remove_reference<Args>::type>::type>...>::
                  value>::type>
constexpr auto cat(Args... args) ->
    typename detail::integer_sequence_cat_helper<Args...>::type {
  return {};
}

// Check whether the passed wrapped type is T.
template <typename T>
struct is {
  constexpr bool operator()(wrap_type<T>) { return true; }
  constexpr bool operator()(...) { return false; }
};

namespace detail {
template <typename O>
struct index_cat {
  template <typename Prev, typename Current>
  constexpr auto operator()(Prev, Current) -> typename std::enable_if<
      O{}(Current{}),
      pack<decltype(index_sequence<1>{} +
                    typename element_type<0, Prev>::type::type{}),
           decltype(cat(typename element_type<1, Prev>::type::type{},
                        index_sequence<(at<0>(typename element_type<0, Prev>::
                                                  type::type{}))>{}))>>::type {
    return {};
  }

  template <typename Prev, typename Current>
  constexpr auto operator()(Prev, Current) -> typename std::enable_if<
      !O{}(Current{}),
      pack<decltype(index_sequence<1>{} +
                    typename element_type<0, Prev>::type::type{}),
           typename element_type<1, Prev>::type::type>>::type {
    return {};
  }
};
}  // namespace detail

/// Get the indexes of the elements that meet a condition.
template <typename T, typename O>
constexpr auto find_if(T&& t, O o) -> typename element_type<
    1, decltype(accumulate(pack<index_sequence<0>, index_sequence<>>{},
                           std::forward<T>(t),
                           detail::index_cat<O>{}))>::type::type {
  return {};
}

template <typename T, typename O>
constexpr auto filter_if(T&& t, O o)
    -> decltype(subset(t, find_if(std::forward<T>(t), o))) {
  return subset(t, find_if(std::forward<T>(t), o));
}

template <typename T, typename O>
constexpr std::size_t count_if(T&& t, O o) {
  return size_v<decltype(find_if(std::forward<T>(t), o))>;
}

template <typename T, typename Pack>
struct is_in {
  static constexpr bool value = (count_if(Pack{}, is<T>{}) > 0);
};

template <typename T, typename Pack>
using insert_into_t = std::conditional_t<is_in<T, Pack>::value, Pack,
                                         decltype(cat(Pack{}, pack<T>{}))>;

}  // namespace mpt

}  // namespace ash

#endif  // INCLUDE_ASH_MPT_H_
