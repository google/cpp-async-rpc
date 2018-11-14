/// \file
/// \brief Trait templates for obtaining information about containers.
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

#ifndef INCLUDE_ASH_TRAITS_CONTAINER_TRAITS_H_
#define INCLUDE_ASH_TRAITS_CONTAINER_TRAITS_H_

#include <array>
#include <iterator>
#include <string>
#include <tuple>
#include <type_traits>
#include <utility>
#include <vector>
#include "ash/traits/trait_factories.h"

namespace ash {

/// Trait classes for type introspection.
namespace traits {

/// \brief Check whether a container supports iteration.
///
/// This implementation checks for an overload of `std::begin(T t)` to apply.
template <typename T, typename T2 = void>
struct is_iterable : public std::integral_constant<bool, false> {};

/// \copydoc is_iterable
template <typename T>
struct is_iterable<
    T, typename enable_if_type<decltype(std::begin(std::declval<T&>()))>::type>
    : public std::integral_constant<bool, true> {};

/// \brief Check whether a container supports iteration.
///
/// This implementation checks for an overload of `std::begin(const T t)` to
/// apply.
template <typename T, typename T2 = void>
struct is_const_iterable : public std::integral_constant<bool, false> {};

/// \copydoc is_const_iterable
template <typename T>
struct is_const_iterable<T, typename enable_if_type<decltype(
                                std::begin(std::declval<const T&>()))>::type>
    : public std::integral_constant<bool, true> {};

/// \brief Check if `T` is an associative container.
///
/// This implementation just checks for a nested `T::key_type` type.
ASH_MAKE_NESTED_TYPE_CHECKER(is_associative, key_type);

namespace detail {
template <typename T>
struct is_contiguous_sequence : public std::false_type {};

template <typename T, std::size_t size>
struct is_contiguous_sequence<T[size]> : public std::true_type {};

template <typename T, std::size_t size>
struct is_contiguous_sequence<std::array<T, size>> : public std::true_type {};

template <typename T, typename Allocator>
struct is_contiguous_sequence<std::vector<T, Allocator>>
    : public std::true_type {};

template <typename CharT, typename Traits, typename Allocator>
struct is_contiguous_sequence<std::basic_string<CharT, Traits, Allocator>>
    : public std::true_type {};
}  // namespace detail

/// \brief Check if `T` is a container storing elements contiguously in memory.
template <typename T>
using is_contiguous_sequence = detail::is_contiguous_sequence<
    typename std::remove_cv<typename std::remove_reference<T>::type>::type>;

namespace detail {
ASH_MAKE_METHOD_CHECKER(has_reserve, reserve);
}  // namespace detail

/// \brief Check whether a container allows for capacity resizing.
///
/// This implementation checks for a `void reserve(T::size_type)` method.
template <typename T, typename T2 = void>
struct can_reserve_capacity : public std::integral_constant<bool, false> {};

/// \copydoc can_reserve_capacity
template <typename T>
struct can_reserve_capacity<
    T, typename enable_if_type<typename T::size_type>::type>
    : public std::integral_constant<
          bool, detail::has_reserve<T, void(typename T::size_type)>::value> {};

namespace detail {
ASH_MAKE_METHOD_CHECKER(has_resize, resize);
}  // namespace detail

/// \brief Check whether a container supports resizing.
///
/// This implementation checks for a `void resize(T::size_type)` method.
template <typename T, typename T2 = void>
struct can_be_resized : public std::integral_constant<bool, false> {};

/// \copydoc can_be_resized
template <typename T>
struct can_be_resized<T, typename enable_if_type<typename T::size_type>::type>
    : public std::integral_constant<
          bool, detail::has_resize<T, void(typename T::size_type)>::value> {};

namespace detail {
template <typename T>
struct has_static_size : public std::false_type {};

template <typename T, std::size_t size>
struct has_static_size<std::array<T, size>> : public std::true_type {};

template <typename T, std::size_t size>
struct has_static_size<T[size]> : public std::true_type {};
}  // namespace detail

/// \brief Check whether a container has a size known at compile time.
template <typename T>
using has_static_size = detail::has_static_size<
    typename std::remove_cv<typename std::remove_reference<T>::type>::type>;

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
using static_size = detail::static_size<
    typename std::remove_cv<typename std::remove_reference<T>::type>::type>;

};  // namespace traits

}  // namespace ash

#endif  // INCLUDE_ASH_TRAITS_CONTAINER_TRAITS_H_
