/// \file
/// \brief Generic type trait templates.
///
/// \copyright
///   Copyright 2018 by Google Inc. All Rights Reserved.
///
/// \copyright
///   Licensed under the Apache License, Version 2.0 (the "License"); you may
///   not use this file except in compliance with the License. You may obtain a
///   copy of the License at
///
/// \copyright
///   http://www.apache.org/licenses/LICENSE-2.0
///
/// \copyright
///   Unless required by applicable law or agreed to in writing, software
///   distributed under the License is distributed on an "AS IS" BASIS, WITHOUT
///   WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied. See the
///   License for the specific language governing permissions and limitations
///   under the License.

#ifndef INCLUDE_ASH_TRAITS_TYPE_TRAITS_H_
#define INCLUDE_ASH_TRAITS_TYPE_TRAITS_H_

#include <tuple>
#include <type_traits>
#include <utility>

namespace ash {

/// Trait classes for type introspection.
namespace traits {

/// Will have a `true` value if the compilation target is little-endian.
constexpr bool target_is_little_endian =
    (__BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__);

/// Will have a `true` value if the compilation target is big-endian.
constexpr bool target_is_big_endian = (__BYTE_ORDER__ == __ORDER_BIG_ENDIAN__);

static_assert(target_is_little_endian ^ target_is_big_endian,
              "Target endianness isn't either big or little endian.");

/// Enable type `R` if `T` evaluates to a type.
///
/// \param T the expression to check.
template <typename T, typename R = void>
struct enable_if_type {
  /// The resulting replacement type.
  using type = R;
};

/// \brief Check whether `T` is a POD scalar that can be transferred as-is
/// between memory and a file.
///
/// The `value` is true when `T` is a scalar type that has a stable meaning when
/// serialized between  processes or program runs; i.e. pointers don't qualify
/// so `std::is_scalar` isn't a good enough test.
///
/// \param T the type to check.
template <typename T>
struct is_bit_transferrable_scalar
    : public std::integral_constant<bool, std::is_arithmetic_v<T> ||
                                              std::is_enum_v<T>> {};
template <typename T>
static inline constexpr bool is_bit_transferrable_scalar_v =
    is_bit_transferrable_scalar<T>::value;

namespace detail {
template <typename T>
struct writable_value_type {
  using type = typename std::remove_cv<T>::type;
};

template <typename U, typename V>
struct writable_value_type<std::pair<U, V>> {
  using type = std::pair<typename std::remove_cv<U>::type,
                         typename std::remove_cv<V>::type>;
};

template <typename... U>
struct writable_value_type<std::tuple<U...>> {
  using type = std::tuple<typename std::remove_cv<U>::type...>;
};
}  // namespace detail
/// \brief Create a type derived of `T` suitable to create a temporary onto
/// which we can read data from a stream.
///
/// The `type` produced can be written to (its members are not
/// `const`-qualified) but the serialization format is compatible to the one for
/// the original type `T`.
template <typename T>
using writable_value_type = detail::writable_value_type<
    typename std::remove_cv<typename std::remove_reference<T>::type>::type>;

template <typename T>
using writable_value_type_t = typename writable_value_type<T>::type;

template <auto mptr>
struct member_data_pointer_traits;

template <typename C, typename T, T(C::*mptr)>
struct member_data_pointer_traits<mptr> {
  static constexpr auto data_ptr = mptr;
  using data_ptr_type = decltype(mptr);
  using data_type = T;
  using class_type = C;
};

template <auto mptr>
struct member_function_pointer_traits;

template <typename C, typename R, typename... A, R (C::*mptr)(A...)>
struct member_function_pointer_traits<mptr> {
  static constexpr auto method_ptr = mptr;
  static constexpr bool is_const = false;
  using method_ptr_type = decltype(mptr);
  using method_type = R(A...);
  using return_type = R;
  using class_type = C;
  using args_ref_tuple_type = std::tuple<const typename std::remove_cv<
      typename std::remove_reference<A>::type>::type&...>;
  using args_tuple_type = std::tuple<typename std::remove_cv<
      typename std::remove_reference<A>::type>::type...>;
  using return_tuple_type =
      typename std::conditional<std::is_same<void, R>::value, std::tuple<>,
                                std::tuple<R>>::type;
};

template <typename C, typename R, typename... A, R (C::*mptr)(A...) const>
struct member_function_pointer_traits<mptr> {
  static constexpr auto method_ptr = mptr;
  static constexpr bool is_const = true;
  using method_ptr_type = decltype(mptr);
  using method_type = R(A...) const;
  using return_type = R;
  using class_type = C;
  using args_ref_tuple_type = std::tuple<const typename std::remove_cv<
      typename std::remove_reference<A>::type>::type&...>;
  using args_tuple_type = std::tuple<typename std::remove_cv<
      typename std::remove_reference<A>::type>::type...>;
  using return_tuple_type =
      typename std::conditional<std::is_same<void, R>::value, std::tuple<>,
                                std::tuple<R>>::type;
};

}  // namespace traits

}  // namespace ash

#endif  // INCLUDE_ASH_TRAITS_TYPE_TRAITS_H_
