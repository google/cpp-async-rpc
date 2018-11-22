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

#include "ash/mpt.h"
#include "ash/preprocessor.h"

namespace ash {

// A method_descriptor type specifies one remotely callable interface method.
template <typename MPtr, MPtr m_ptr>
struct method_descriptor;

template <typename C, typename Result, typename... Args,
          Result (C::*m_ptr)(const Args &...)>
struct method_descriptor<Result (C::*)(const Args &...), m_ptr> {
  using class_type = C;
  using result_type = Result;
  using method_type_ptr = Result (C::*)(const Args &...);
  using method_type = Result(const Args &&...);
  static constexpr Result (C::*method)(const Args &...) = m_ptr;
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
#define ASH_METHOD(NAME) \
  ::ash::method_descriptor<decltype(&own_interface::NAME), &own_interface::NAME>
#define ASH_METHOD_SEP() ,
#define ASH_METHOD_NAME(NAME) #NAME
#define ASH_METHOD_NAME_SEP() ,

/// Needed to find our own interface class in template interfaces, as the base
/// class is dependent.
#define ASH_OWN_INTERFACE(...) using own_interface = __VA_ARGS__

/// Define the list of `method_descriptor` elements for the current interface.
#define ASH_METHODS(...)                                               \
  static constexpr const char *method_names[] = {                      \
      ASH_FOREACH(ASH_METHOD_NAME, ASH_METHOD_NAME_SEP, __VA_ARGS__)}; \
  using method_descriptors =                                           \
      ::ash::mpt::pack<ASH_FOREACH(ASH_METHOD, ASH_METHOD_SEP, __VA_ARGS__)>

/// Get the method name associated to a method descriptor.
template <typename MethodDescriptor>
constexpr const char *method_name() {
  return MethodDescriptor::class_type::field_names[mpt::at<0>(
      mpt::find_if(typename MethodDescriptor::class_type::method_descriptors{},
                   mpt::is<MethodDescriptor>{}))];

}  // namespace ash

#endif  // INCLUDE_ASH_INTERFACE_H_
