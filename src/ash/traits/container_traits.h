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

#ifndef ASH_TRAITS_CONTAINER_TRAITS_H_
#define ASH_TRAITS_CONTAINER_TRAITS_H_

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
struct is_iterable : public std::false_type {};

/// \copydoc is_iterable
template <typename T>
struct is_iterable<T,
                   enable_if_type_t<decltype(std::begin(std::declval<T&>()))>>
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
    T, enable_if_type_t<decltype(std::begin(std::declval<const T&>()))>>
    : public std::true_type {};

template <typename T>
inline constexpr bool is_const_iterable_v = is_const_iterable<T>::value;

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
    std::remove_cv_t<std::remove_reference_t<T>>>;

template <typename T>
inline constexpr bool is_contiguous_sequence_v =
    is_contiguous_sequence<T>::value;

namespace detail {
ASH_MAKE_METHOD_CHECKER(has_reserve, reserve);
}  // namespace detail

/// \brief Check whether a container allows for capacity resizing.
///
/// This implementation checks for a `void reserve(T::size_type)` method.
template <typename T, typename T2 = void>
struct can_reserve_capacity : public std::false_type {};

/// \copydoc can_reserve_capacity
template <typename T>
struct can_reserve_capacity<T, enable_if_type_t<typename T::size_type>>
    : public std::integral_constant<
          bool, detail::has_reserve_v<T, void(typename T::size_type)>> {};

template <typename T>
inline constexpr bool can_reserve_capacity_v = can_reserve_capacity<T>::value;

namespace detail {
ASH_MAKE_METHOD_CHECKER(has_resize, resize);
}  // namespace detail

/// \brief Check whether a container supports resizing.
///
/// This implementation checks for a `void resize(T::size_type)` method.
template <typename T, typename T2 = void>
struct can_be_resized : public std::false_type {};

/// \copydoc can_be_resized
template <typename T>
struct can_be_resized<T, enable_if_type_t<typename T::size_type>>
    : public std::integral_constant<
          bool, detail::has_resize_v<T, void(typename T::size_type)>> {};

template <typename T>
inline constexpr bool can_be_resized_v = can_be_resized<T>::value;

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
using has_static_size =
    detail::has_static_size<std::remove_cv_t<std::remove_reference_t<T>>>;

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
using static_size =
    detail::static_size<std::remove_cv_t<std::remove_reference_t<T>>>;

template <typename T>
inline constexpr std::size_t static_size_v = static_size<T>::value;

};  // namespace traits

}  // namespace ash

#endif  // ASH_TRAITS_CONTAINER_TRAITS_H_
