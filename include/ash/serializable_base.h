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

#include <array>
#include <cstdint>
#include <type_traits>
#include "ash/dynamic_base_class.h"
#include "ash/errors.h"
#include "ash/mpt.h"
#include "ash/preprocessor.h"
#include "ash/traits/type_traits.h"

namespace ash {

namespace detail {
struct dynamic_class_filter {
  template <typename T>
  constexpr bool operator()(mpt::wrap_type<T>) {
    return is_dynamic<T>::value;
  }
};

template <bool dynamic, typename OwnType, typename... Bases>
struct serializable_mixin;

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
template <auto mptr>
struct field_descriptor : public traits::member_data_pointer_traits<mptr> {
  /// Get the field's name.
  static const char* name() {
    using class_type = typename field_descriptor<mptr>::class_type;
    return class_type::field_names()[mpt::at<0>(
        mpt::find_if(typename class_type::field_descriptors{},
                     mpt::is<field_descriptor<mptr>>{}))];
  }
};  // namespace ash

/// Inherit publicly from this in serializable classes, specifying own type and
/// public bases.
template <typename OwnType, typename... Bases>
using serializable = detail::serializable_mixin<
    (mpt::count_if(mpt::pack<Bases...>{}, detail::dynamic_class_filter{}) > 0),
    OwnType, Bases...>;

/// Inherit publicly from this in dynamic classes, specifying own type and
/// public bases.
template <typename OwnType, typename... Bases>
using dynamic = std::conditional_t<
    (mpt::count_if(mpt::pack<Bases...>{}, detail::dynamic_class_filter{}) > 0),
    serializable<OwnType, Bases...>,
    serializable<OwnType, ::ash::dynamic_base_class, Bases...>>;

/// Define a `field_descriptor` type for a member field named `NAME`.
#define ASH_FIELD(NAME) ::ash::field_descriptor<&own_type::NAME>
#define ASH_FIELD_SEP() ,
#define ASH_FIELD_NAME(NAME) #NAME
#define ASH_FIELD_NAME_SEP() ,

/// Needed to find our own type in template classes, as the base class is
/// dependent.
#define ASH_OWN_TYPE(...) using own_type = __VA_ARGS__

/// Define the list of `field_descriptor` elements for the current class.
#define ASH_FIELDS(...)                                                      \
  using field_descriptors =                                                  \
      ::ash::mpt::pack<ASH_FOREACH(ASH_FIELD, ASH_FIELD_SEP, __VA_ARGS__)>;  \
  static const std::array<const char*,                                       \
                          ::ash::mpt::size<field_descriptors>::value>&       \
  field_names() {                                                            \
    static const std::array<const char*,                                     \
                            ::ash::mpt::size<field_descriptors>::value>      \
        names{ASH_FOREACH(ASH_FIELD_NAME, ASH_FIELD_NAME_SEP, __VA_ARGS__)}; \
    return names;                                                            \
  }

/// Version of the load/save methods.
#define ASH_CUSTOM_SERIALIZATION_VERSION(VERSION)                  \
  static_assert(VERSION != 0,                                      \
                "Custom serialization version must be non-zero."); \
  static constexpr std::uint32_t custom_serialization_version = VERSION

/// Get the field descriptor for a given member pointer.
template <auto mptr>
struct get_field_descriptor {
  using class_type =
      typename traits::member_data_pointer_traits<mptr>::class_type;
  static constexpr auto field_descriptor_index =
      mpt::at<0>(mpt::find_if(typename class_type::field_descriptors{},
                              mpt::is<field_descriptor<mptr>>{}));
  using type = typename decltype(mpt::at<field_descriptor_index>(
      typename class_type::field_descriptors{}))::type;
};

template <auto mptr>
using get_field_descriptor_t = typename get_field_descriptor<mptr>::type;
}  // namespace ash

#endif  // INCLUDE_ASH_SERIALIZABLE_BASE_H_
