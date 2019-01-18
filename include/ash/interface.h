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
    return class_type::method_names()
        [mpt::find_v<typename class_type::method_descriptors,
                     mpt::is_type<method_descriptor<mptr>>>];
  }
};

template <typename OwnInterface, typename... Extends>
struct interface : virtual Extends... {
  using own_interface = OwnInterface;
  using extended_interfaces = mpt::pack<Extends...>;
  using method_descriptors = mpt::pack<>;

  virtual ~interface() {}
};

#define ASH_INTERFACE_EXTENDS_ONE(...) ASH_EXPAND_1 __VA_ARGS__
#define ASH_INTERFACE_EXTENDS_SEP() ,
#define ASH_INTERFACE_EXTENDS(...) \
  ASH_FOREACH(ASH_INTERFACE_EXTENDS_ONE, ASH_INTERFACE_EXTENDS_SEP, __VA_ARGS__)

#define ASH_INTERFACE_DECL_ARG(TYPE, NAME) ASH_EXPAND_1 TYPE NAME
#define ASH_INTERFACE_DECL_ARGS_ONE(...) ASH_INTERFACE_DECL_ARG __VA_ARGS__
#define ASH_INTERFACE_DECL_ARGS_SEP() ,
#define ASH_INTERFACE_DECL_ARGS(...) \
  ASH_DEFER_2(ASH_FOREACH_AGAIN)     \
  ()(ASH_INTERFACE_DECL_ARGS_ONE, ASH_INTERFACE_DECL_ARGS_SEP, __VA_ARGS__)
#define ASH_INTERFACE_DECL(RETURN, METHOD, ARGS) \
  virtual ASH_EXPAND_1 RETURN METHOD(ASH_INTERFACE_DECL_ARGS ARGS) = 0;
#define ASH_INTERFACE_DECLS_ONE(...) ASH_INTERFACE_DECL __VA_ARGS__
#define ASH_INTERFACE_DECLS_SEP()
#define ASH_INTERFACE_DECLS(...) \
  ASH_FOREACH(ASH_INTERFACE_DECLS_ONE, ASH_INTERFACE_DECLS_SEP, __VA_ARGS__)

#define ASH_INTERFACE_METHOD(RETURN, METHOD, ARGS) \
  ::ash::method_descriptor<&own_interface::METHOD>
#define ASH_INTERFACE_METHODS_ONE(...) ASH_INTERFACE_METHOD __VA_ARGS__
#define ASH_INTERFACE_METHODS_SEP() ,
#define ASH_INTERFACE_METHODS(...) \
  ASH_FOREACH(ASH_INTERFACE_METHODS_ONE, ASH_INTERFACE_METHODS_SEP, __VA_ARGS__)

#define ASH_INTERFACE_METHOD_NAME(RETURN, METHOD, ARGS) #METHOD
#define ASH_INTERFACE_METHOD_NAMES_ONE(...) \
  ASH_INTERFACE_METHOD_NAME __VA_ARGS__
#define ASH_INTERFACE_METHOD_NAMES_SEP() ,
#define ASH_INTERFACE_METHOD_NAMES(...)                                       \
  ASH_FOREACH(ASH_INTERFACE_METHOD_NAMES_ONE, ASH_INTERFACE_METHOD_NAMES_SEP, \
              __VA_ARGS__)

#define ASH_INTERFACE(NAME, EXTENDS, METHODS)                                \
  struct NAME : ::ash::interface<NAME ASH_IF(ASH_NOT(ASH_IS_EMPTY EXTENDS))( \
                    , ) ASH_EXPAND(ASH_INTERFACE_EXTENDS EXTENDS)> {         \
    ASH_EXPAND(ASH_INTERFACE_DECLS METHODS)                                  \
    using method_descriptors =                                               \
        ::ash::mpt::pack<ASH_EXPAND(ASH_INTERFACE_METHODS METHODS)>;         \
    static const std::array<const char *,                                    \
                            ::ash::mpt::size_v<method_descriptors>>          \
        &method_names() {                                                    \
      static const std::array<const char *,                                  \
                              ::ash::mpt::size_v<method_descriptors>>        \
          names{ASH_EXPAND(ASH_INTERFACE_METHOD_NAMES METHODS)};             \
      return names;                                                          \
    }                                                                        \
  }

}  // namespace ash

#endif  // INCLUDE_ASH_INTERFACE_H_
