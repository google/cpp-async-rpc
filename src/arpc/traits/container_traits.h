/// \file
/// \brief Trait templates for obtaining information about containers.
///
/// \copyright
///   Copyright 2019 by Google LLC.
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

#ifndef ARPC_TRAITS_CONTAINER_TRAITS_H_
#define ARPC_TRAITS_CONTAINER_TRAITS_H_

#include <array>
#include <iterator>
#include <string>
#include <string_view>
#include <tuple>
#include <type_traits>
#include <utility>
#include <vector>
#include "arpc/mpt.h"
#include "arpc/traits/trait_factories.h"

namespace arpc {

/// Trait classes for type introspection.
namespace traits {

/// \brief Check whether a container supports iteration.
///
/// This implementation checks for an overload of `std::begin(T t)` to apply.
template <typename T, typename T2 = void>
struct is_iterable : public std::false_type {};

/// \copydoc is_iterable
template <typename T>
struct is_iterable<T, std::void_t<decltype(std::begin(std::declval<T&>()))>>
    : public std::true_type {};

template <typename T>
inline constexpr bool is_iterable_v = is_iterable<T>::value;

/// \brief Check whether a container supports iteration.
///
/// This implementation checks for an overload of `std::begin(const T t)` to
/// apply.
template <typename T, typename T2 = void>
struct is_const_iterable : public std::false_type {};

/// \copydoc is_const_iterable
template <typename T>
struct is_const_iterable<
    T, std::void_t<decltype(std::begin(std::declval<const T&>()))>>
    : public std::true_type {};

template <typename T>
inline constexpr bool is_const_iterable_v = is_const_iterable<T>::value;

/// \brief Check if `T` is an associative container.
///
/// This implementation just checks for a nested `T::key_type` type.
ARPC_MAKE_NESTED_TYPE_CHECKER(is_associative, key_type);

template <typename T, typename Enable = void>
struct is_map : public std::false_type {};

template <typename T>
struct is_map<T, std::enable_if_t<is_associative_v<T> &&
                                  !std::is_same_v<typename T::key_type,
                                                  typename T::value_type>>>
    : public std::true_type {};

template <typename T>
inline constexpr bool is_map_v = is_map<T>::value;

template <typename T, typename Enable = void>
struct is_set : public std::false_type {};

template <typename T>
struct is_set<T, std::enable_if_t<is_associative_v<T> &&
                                  std::is_same_v<typename T::key_type,
                                                 typename T::value_type>>>
    : public std::true_type {};

template <typename T>
inline constexpr bool is_set_v = is_set<T>::value;

template <typename T, typename Enable = void>
struct is_multi_key_associative
    : public std::bool_constant<is_associative_v<T>> {};

template <typename T>
struct is_multi_key_associative<
    T, std::void_t<typename decltype(std::declval<T>().insert(
           std::declval<typename T::value_type>()))::second_type>>
    : public std::false_type {};

template <typename T>
inline constexpr bool is_multi_key_associative_v =
    is_multi_key_associative<T>::value;

namespace detail {
template <typename T, typename Enable = void>
struct is_contiguous_sequence : public std::false_type {};

template <typename T, std::size_t size>
struct is_contiguous_sequence<T[size]> : public std::true_type {};

template <typename T>
struct is_contiguous_sequence<
    T, std::enable_if_t<std::is_same_v<
           decltype(std::declval<T>().data()),
           std::remove_reference_t<decltype(*(std::declval<T>().begin()))>*>>>
    : public std::true_type {};

}  // namespace detail

/// \brief Check if `T` is a container storing elements contiguously in memory.
template <typename T>
using is_contiguous_sequence =
    detail::is_contiguous_sequence<remove_cvref_t<T>>;

template <typename T>
inline constexpr bool is_contiguous_sequence_v =
    is_contiguous_sequence<T>::value;

namespace detail {
ARPC_MAKE_METHOD_CHECKER(has_reserve, reserve);
}  // namespace detail

/// \brief Check whether a container allows for capacity resizing.
///
/// This implementation checks for a `void reserve(T::size_type)` method.
template <typename T, typename T2 = void>
struct can_reserve_capacity : public std::false_type {};

/// \copydoc can_reserve_capacity
template <typename T>
struct can_reserve_capacity<T, std::void_t<typename T::size_type>>
    : public std::integral_constant<
          bool, detail::has_reserve_v<T, void(typename T::size_type)>> {};

template <typename T>
inline constexpr bool can_reserve_capacity_v = can_reserve_capacity<T>::value;

namespace detail {
ARPC_MAKE_METHOD_CHECKER(has_resize, resize);
}  // namespace detail

/// \brief Check whether a container supports resizing.
///
/// This implementation checks for a `void resize(T::size_type)` method.
template <typename T, typename T2 = void>
struct can_be_resized : public std::false_type {};

/// \copydoc can_be_resized
template <typename T>
struct can_be_resized<T, std::void_t<typename T::size_type>>
    : public std::integral_constant<
          bool, detail::has_resize_v<T, void(typename T::size_type)>> {};

template <typename T>
inline constexpr bool can_be_resized_v = can_be_resized<T>::value;

namespace detail {
template <typename T>
struct has_static_size : public std::false_type {};

template <typename T, std::size_t size>
struct has_static_size<std::array<T, size>> : public std::true_type {};

template <typename U, typename V>
struct has_static_size<std::pair<U, V>> : public std::true_type {};

template <typename... T>
struct has_static_size<std::tuple<T...>> : public std::true_type {};

template <typename T, std::size_t size>
struct has_static_size<T[size]> : public std::true_type {};
}  // namespace detail

/// \brief Check whether a container has a size known at compile time.
template <typename T>
using has_static_size = detail::has_static_size<remove_cvref_t<T>>;

template <typename T>
inline constexpr bool has_static_size_v = has_static_size<T>::value;

namespace detail {
template <typename T>
struct static_size
    : public std::integral_constant<std::size_t, std::tuple_size<T>::value> {};

template <typename T, std::size_t size>
struct static_size<T[size]> : public std::integral_constant<std::size_t, size> {
};
}  // namespace detail

/// \brief Get a container's static size at compile time.
template <typename T>
using static_size = detail::static_size<remove_cvref_t<T>>;

template <typename T>
inline constexpr std::size_t static_size_v = static_size<T>::value;

// Technique for detecting the number of fields from an aggregate adapted from
// https://github.com/felixguendling/cista
//
// Credits: Implementation by Anatoliy V. Tomilov (@tomilov),
//          based on gist by Rafal T. Janik (@ChemiaAion)
//
// Resources:
// https://playfulprogramming.blogspot.com/2016/12/serializing-structs-with-c17-structured.html
// https://codereview.stackexchange.com/questions/142804/get-n-th-data-member-of-a-struct
// https://stackoverflow.com/questions/39768517/structured-bindings-width
// https://stackoverflow.com/questions/35463646/arity-of-aggregate-in-logarithmic-time
// https://stackoverflow.com/questions/38393302/returning-variadic-aggregates-struct-and-syntax-for-c17-variadic-template-c
namespace detail {

struct wildcard_type {
  template <typename T>
  operator T() const;
};

template <typename Aggregate, typename IndexSequence = mpt::index_sequence<>,
          typename = void>
struct aggregate_arity_impl : IndexSequence {};

template <typename Aggregate, std::size_t... Indices>
struct aggregate_arity_impl<
    Aggregate, mpt::index_sequence<Indices...>,
    std::void_t<decltype(Aggregate{
        (static_cast<void>(Indices), std::declval<wildcard_type>())...,
        std::declval<wildcard_type>()})>>
    : aggregate_arity_impl<
          Aggregate, mpt::index_sequence<Indices..., sizeof...(Indices)>> {};

}  // namespace detail

template <typename T>
struct aggregate_arity
    : std::integral_constant<
          std::size_t, detail::aggregate_arity_impl<remove_cvref_t<T>>::size> {
};

template <typename T>
inline constexpr std::size_t aggregate_arity_v = aggregate_arity<T>::value;

template <typename T>
struct is_bindable_aggregate
    : std::bool_constant<std::is_aggregate_v<T> && std::is_class_v<T> &&
                         std::is_standard_layout_v<T>> {};
template <typename T>
inline constexpr bool is_bindable_aggregate_v = is_bindable_aggregate<T>::value;

};  // namespace traits

}  // namespace arpc

#endif  // ARPC_TRAITS_CONTAINER_TRAITS_H_
