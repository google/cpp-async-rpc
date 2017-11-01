/// \file
/// \brief Trait templates for obtaining serialization-related metadata.
///
/// \copyright
///   Copyright 2017 by Google Inc. All Rights Reserved.
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

#ifndef INCLUDE_ASH_TRAITS_SERIALIZATION_TRAITS_H_
#define INCLUDE_ASH_TRAITS_SERIALIZATION_TRAITS_H_

#include <cstdint>
#include <type_traits>
#include "ash/mpt.h"
#include "ash/traits/trait_factories.h"

namespace ash {

/// Trait classes for type introspection.
namespace traits {

/// \brief Check for field descriptors in `T` to support automatic field
/// serialization.
///
/// The implementation checks for a nested type `T::field_descriptors`.
ASH_MAKE_NESTED_TYPE_CHECKER(has_field_descriptors, field_descriptors);

/// \brief Retrieve the field descriptors for a type `T`.
template <typename T, typename Enable = void>
struct get_field_descriptors;

/// \copydoc get_field_descriptors
///
/// Specialization for types explicitly defining `T::field_descriptors`.
template <typename T>
struct get_field_descriptors<
    T, typename std::enable_if<has_field_descriptors<T>::value>::type> {
  using type = typename T::field_descriptors;
};

/// \copydoc get_field_descriptors
///
/// Specialization for types not defining `T::field_descriptors`, which defaults
/// to an empty list of field descriptors.
template <typename T>
struct get_field_descriptors<
    T, typename std::enable_if<!has_field_descriptors<T>::value>::type> {
  using type = mpt::pack<>;
};

/// \brief Check for base classes in `T` to support automatic base class
/// serialization.
///
/// For automatic serialization support, all of these must be public,
/// non-virtual classes.
///
/// The implementation checks for a nested type `T::base_classes`.
ASH_MAKE_NESTED_TYPE_CHECKER(has_base_classes, base_classes);

/// \brief Retrieve the base classes for a type `T`.
template <typename T, typename Enable = void>
struct get_base_classes;

/// \copydoc get_base_classes
///
/// Specialization for types explicitly defining `T::base_classes`.
template <typename T>
struct get_base_classes<
    T, typename std::enable_if<has_base_classes<T>::value>::type> {
  using type = typename T::base_classes;
};

/// \copydoc get_base_classes
///
/// Specialization for types not defining `T::base_classes`, which defaults
/// to an empty list of base classes.
template <typename T>
struct get_base_classes<
    T, typename std::enable_if<!has_base_classes<T>::value>::type> {
  using type = mpt::pack<>;
};

/// \brief Type for custom serialization version numbers.
using custom_serialization_version_type = std::uint32_t;

namespace detail {
ASH_MAKE_NESTED_CONSTANT_CHECKER(has_custom_serialization_version,
                                 custom_serialization_version);
}  // namespace detail

/// \brief Check if a type `T` implements custom serialization.
///
/// The result is `true` if the type defines a
/// `T::custom_serialization_version` of type
/// `custom_serialization_version_type`
/// constant.
template <typename T>
struct has_custom_serialization : detail::has_custom_serialization_version<
                                      T, custom_serialization_version_type> {};

/// \brief Retrieve the custom serialization version for a type `T`.
template <typename T, typename Enable = void>
struct get_custom_serialization_version;

/// \copydoc get_custom_serialization_version
///
/// Specialization for types explicitly defining
/// `T::custom_serialization_version`.
template <typename T>
struct get_custom_serialization_version<
    T, typename std::enable_if<has_custom_serialization<T>::value>::type> {
  static constexpr custom_serialization_version_type value =
      T::custom_serialization_version;
};

/// \copydoc get_custom_serialization_version
///
/// Specialization for types not defining `T::custom_serialization_version`,
/// which defaults to zero.
template <typename T>
struct get_custom_serialization_version<
    T, typename std::enable_if<!has_custom_serialization<T>::value>::type> {
  static constexpr custom_serialization_version_type value = 0;
};

/// \brief Check if a type `T` can be serialized.
/// The result is `true` if the type either has field descriptors defined or
/// it implements custom serialization.
template <typename T>
struct can_be_serialized
    : std::integral_constant<bool,
                             has_field_descriptors<T>::value ||
                                 has_custom_serialization<T>::value> {};

}  // namespace traits

}  // namespace ash

#endif  // INCLUDE_ASH_TRAITS_SERIALIZATION_TRAITS_H_
