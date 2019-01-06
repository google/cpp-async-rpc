/// \file
/// \brief Classes for defining RPC interfaces.
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

#ifndef INCLUDE_ASH_INTERFACE_H_
#define INCLUDE_ASH_INTERFACE_H_

#include <array>
#include "ash/mpt.h"
#include "ash/preprocessor.h"
#include "ash/traits/type_traits.h"

namespace ash {

// A method_descriptor type specifies one remotely callable interface method.
template <auto mptr>
struct method_descriptor : public traits::member_function_pointer_traits<mptr> {
  /// Get the method's name.
  static const char *name() {
    using class_type = typename method_descriptor<mptr>::class_type;
    return class_type::method_names()[mpt::at<0>(
        mpt::find_if(typename class_type::method_descriptors{},
                     mpt::is<method_descriptor<mptr>>{}))];
  }
};

template <typename OwnInterface, typename... Bases>
struct interface : Bases... {
  using own_interface = OwnInterface;
  using base_interfaces = mpt::pack<Bases...>;
  using method_descriptors = mpt::pack<>;
  static constexpr const char *method_names[] = {};

  virtual ~interface() {}
};

/// Define a `method_descriptor` type for a method named `NAME`.
#define ASH_METHOD(NAME) ::ash::method_descriptor<&own_interface::NAME>
#define ASH_METHOD_SEP() ,
#define ASH_METHOD_NAME(NAME) #NAME
#define ASH_METHOD_NAME_SEP() ,

/// Needed to find our own interface class in template interfaces, as the base
/// class is dependent.
#define ASH_OWN_INTERFACE(...) using own_interface = __VA_ARGS__

/// Define the list of `method_descriptor` elements for the current interface.
#define ASH_METHODS(...)                                                       \
  using method_descriptors =                                                   \
      ::ash::mpt::pack<ASH_FOREACH(ASH_METHOD, ASH_METHOD_SEP, __VA_ARGS__)>;  \
  static const std::array<const char *,                                        \
                          ::ash::mpt::size<method_descriptors>::value>         \
      &method_names() {                                                        \
    static const std::array<const char *,                                      \
                            ::ash::mpt::size<method_descriptors>::value>       \
        names{ASH_FOREACH(ASH_METHOD_NAME, ASH_METHOD_NAME_SEP, __VA_ARGS__)}; \
    return names;                                                              \
  }

/// Get the method descriptor for a given member function pointer.
template <auto mptr>
struct get_method_descriptor {
  using class_type =
      typename traits::member_function_pointer_traits<mptr>::class_type;
  static constexpr auto method_descriptor_index =
      mpt::at<0>(mpt::find_if(typename class_type::method_descriptors{},
                              mpt::is<method_descriptor<mptr>>{}));
  using type = typename decltype(mpt::at<method_descriptor_index>(
      typename class_type::method_descriptors{}))::type;
};
}  // namespace ash

#endif  // INCLUDE_ASH_INTERFACE_H_
