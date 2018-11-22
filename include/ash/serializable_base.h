/// \file
/// \brief Header defining a base class for serializable data.
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

#ifndef INCLUDE_ASH_SERIALIZABLE_BASE_H_
#define INCLUDE_ASH_SERIALIZABLE_BASE_H_

#include <cstdint>
#include "ash/dynamic_base_class.h"
#include "ash/errors.h"
#include "ash/mpt.h"
#include "ash/preprocessor.h"

namespace ash {

namespace detail {
struct dynamic_class_filter {
  template <typename T>
  constexpr bool operator()(mpt::wrap_type<T>) {
    return is_dynamic<T>::value;
  }
};

template <bool dynamic, typename OwnType, typename... Bases>
class serializable_mixin;

template <typename OwnType, typename... Bases>
struct serializable_mixin<false, OwnType, Bases...> : Bases... {
  using own_type = OwnType;
  using base_classes = mpt::pack<Bases...>;
  using dynamic_base_classes =
      decltype(mpt::filter_if(base_classes{}, dynamic_class_filter{}));

  // Don't inherit load/save behavior from the parent, and provide an empty set
  // of fields.
  template <typename S>
  void save(S& s) const = delete;
  template <typename S>
  void load(S& s) = delete;
  using field_descriptors = mpt::pack<>;
};

template <typename OwnType, typename... Bases>
struct serializable_mixin<true, OwnType, Bases...>
    : serializable_mixin<false, OwnType, Bases...> {
 private:
  // Implement the virtual function that returns the name of our type.
  const char* portable_class_name_internal() const override {
    using Descriptor = ash::detail::dynamic_class_descriptor<OwnType>;
    if (Descriptor::class_name == nullptr)
      throw errors::invalid_state("Dynamic class had no name set");
    return Descriptor::class_name;
  }
};

}  // namespace detail

/// A field_descriptor type specifies how to access one data member.
template <typename MPtr, MPtr m_ptr>
struct field_descriptor;

/// A field_descriptor type specifies how to access one data member.
template <typename C, typename T, T C::*m_ptr>
struct field_descriptor<T C::*, m_ptr> {
  using class_type = C;
  using member_type = T;
  static constexpr auto member_pointer = m_ptr;
};

/// Inherit publicly from this in serializable classes, specifying own type and
/// public bases.
template <typename OwnType, typename... Bases>
using serializable = detail::serializable_mixin<
    (mpt::count_if(mpt::pack<Bases...>{}, detail::dynamic_class_filter{}) > 0),
    OwnType, Bases...>;

/// Inherit publicly from this in dynamic classes, specifying own type and
/// public bases.
template <typename OwnType, typename... Bases>
using dynamic = mpt::conditional_t<
    (mpt::count_if(mpt::pack<Bases...>{}, detail::dynamic_class_filter{}) > 0),
    serializable<OwnType, Bases...>,
    serializable<OwnType, ::ash::dynamic_base_class, Bases...>>;

/// Define a `field_descriptor` type for a member field named `NAME`.
#define ASH_FIELD(NAME) \
  ::ash::field_descriptor<decltype(&own_type::NAME), &own_type::NAME>
#define ASH_FIELD_SEP() ,
#define ASH_FIELD_NAME(NAME) #NAME
#define ASH_FIELD_NAME_SEP() ,

/// Needed to find our own type in template classes, as the base class is
/// dependent.
#define ASH_OWN_TYPE(...) using own_type = __VA_ARGS__

/// Define the list of `field_descriptor` elements for the current class.
#define ASH_FIELDS(...)                                              \
  static constexpr const char* field_names[] = {                     \
      ASH_FOREACH(ASH_FIELD_NAME, ASH_FIELD_NAME_SEP, __VA_ARGS__)}; \
  using field_descriptors =                                          \
      ::ash::mpt::pack<ASH_FOREACH(ASH_FIELD, ASH_FIELD_SEP, __VA_ARGS__)>

/// Version of the load/save methods.
#define ASH_CUSTOM_SERIALIZATION_VERSION(VERSION)                  \
  static_assert(VERSION != 0,                                      \
                "Custom serialization version must be non-zero."); \
  static constexpr std::uint32_t custom_serialization_version = VERSION

/// Get the field name associated to a field descriptor.
template <typename FieldDescriptor>
constexpr const char* field_name() {
  return FieldDescriptor::class_type::field_names[mpt::at<0>(
      mpt::find_if(typename FieldDescriptor::class_type::field_descriptors{},
                   mpt::is<FieldDescriptor>{}))];
}

}  // namespace ash

#endif  // INCLUDE_ASH_SERIALIZABLE_BASE_H_
