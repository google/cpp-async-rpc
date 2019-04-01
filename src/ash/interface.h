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

#ifndef ASH_INTERFACE_H_
#define ASH_INTERFACE_H_

#include <array>
#include <utility>
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

  template <typename O>
  static auto make_proxy(O obj) {
    return typename own_interface::template proxy<O>(std::move(obj));
  }

  virtual ~interface() {}
};

template <typename OwnInterface, typename... Extends>
struct async_interface : virtual Extends::async... {
  using own_interface = OwnInterface;

  template <typename O>
  static auto make_proxy(O obj) {
    return typename own_interface::async::template proxy<O>(std::move(obj));
  }

  virtual ~async_interface() {}
};

#define ASH_INTERFACE_EXTENDS_ONE(...) ASH_EXPAND_1 __VA_ARGS__
#define ASH_INTERFACE_EXTENDS_SEP() ,
#define ASH_INTERFACE_EXTENDS(...) \
  ASH_FOREACH(ASH_INTERFACE_EXTENDS_ONE, ASH_INTERFACE_EXTENDS_SEP, __VA_ARGS__)

#define ASH_INTERFACE_PROXY_EXTENDS_ONE(...) \
  virtual ASH_EXPAND_1 __VA_ARGS__::proxy<O>
#define ASH_INTERFACE_PROXY_EXTENDS_SEP() ,
#define ASH_INTERFACE_PROXY_EXTENDS(...)       \
  ASH_FOREACH(ASH_INTERFACE_PROXY_EXTENDS_ONE, \
              ASH_INTERFACE_PROXY_EXTENDS_SEP, __VA_ARGS__)

#define ASH_INTERFACE_ASYNC_PROXY_EXTENDS_ONE(...) \
  virtual ASH_EXPAND_1 __VA_ARGS__::async::proxy<O>
#define ASH_INTERFACE_ASYNC_PROXY_EXTENDS_SEP() ,
#define ASH_INTERFACE_ASYNC_PROXY_EXTENDS(...)       \
  ASH_FOREACH(ASH_INTERFACE_ASYNC_PROXY_EXTENDS_ONE, \
              ASH_INTERFACE_ASYNC_PROXY_EXTENDS_SEP, __VA_ARGS__)

#define ASH_INTERFACE_PROXY_CTOR_ONE(...) \
  ASH_EXPAND_1 __VA_ARGS__::proxy<O>(obj)
#define ASH_INTERFACE_PROXY_CTOR_SEP() ,
#define ASH_INTERFACE_PROXY_CTOR(...)                                     \
  ASH_FOREACH(ASH_INTERFACE_PROXY_CTOR_ONE, ASH_INTERFACE_PROXY_CTOR_SEP, \
              __VA_ARGS__)

#define ASH_INTERFACE_ASYNC_PROXY_CTOR_ONE(...) \
  ASH_EXPAND_1 __VA_ARGS__::async::proxy<O>(obj)
#define ASH_INTERFACE_ASYNC_PROXY_CTOR_SEP() ,
#define ASH_INTERFACE_ASYNC_PROXY_CTOR(...)       \
  ASH_FOREACH(ASH_INTERFACE_ASYNC_PROXY_CTOR_ONE, \
              ASH_INTERFACE_ASYNC_PROXY_CTOR_SEP, __VA_ARGS__)

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

#define ASH_INTERFACE_ASYNC_DECL(RETURN, METHOD, ARGS) \
  virtual ::ash::future<ASH_EXPAND_1 RETURN> METHOD(   \
      ASH_INTERFACE_DECL_ARGS ARGS) = 0;
#define ASH_INTERFACE_ASYNC_DECLS_ONE(...) ASH_INTERFACE_ASYNC_DECL __VA_ARGS__
#define ASH_INTERFACE_ASYNC_DECLS_SEP()
#define ASH_INTERFACE_ASYNC_DECLS(...)                                      \
  ASH_FOREACH(ASH_INTERFACE_ASYNC_DECLS_ONE, ASH_INTERFACE_ASYNC_DECLS_SEP, \
              __VA_ARGS__)

#define ASH_INTERFACE_PROXY_IMPL_ARG(TYPE, NAME) ASH_EXPAND_1 TYPE NAME
#define ASH_INTERFACE_PROXY_IMPL_ARGS_ONE(...) \
  ASH_INTERFACE_PROXY_IMPL_ARG __VA_ARGS__
#define ASH_INTERFACE_PROXY_IMPL_ARGS_SEP() ,
#define ASH_INTERFACE_PROXY_IMPL_ARGS(...)                                 \
  ASH_DEFER_2(ASH_FOREACH_AGAIN)                                           \
  ()(ASH_INTERFACE_PROXY_IMPL_ARGS_ONE, ASH_INTERFACE_PROXY_IMPL_ARGS_SEP, \
     __VA_ARGS__)
#define ASH_INTERFACE_PROXY_FORWARD_ARG(TYPE, NAME) \
  std::forward<ASH_EXPAND_1 TYPE>(NAME)
#define ASH_INTERFACE_PROXY_FORWARD_ARGS_ONE(...) \
  ASH_INTERFACE_PROXY_FORWARD_ARG __VA_ARGS__
#define ASH_INTERFACE_PROXY_FORWARD_ARGS_SEP() ,
#define ASH_INTERFACE_PROXY_FORWARD_ARGS(...) \
  ASH_DEFER_2(ASH_FOREACH_AGAIN)              \
  ()(ASH_INTERFACE_PROXY_FORWARD_ARGS_ONE,    \
     ASH_INTERFACE_PROXY_FORWARD_ARGS_SEP, __VA_ARGS__)
#define ASH_INTERFACE_PROXY_IMPL(RETURN, METHOD, ARGS)                      \
  ASH_EXPAND_1 RETURN METHOD(ASH_INTERFACE_PROXY_IMPL_ARGS ARGS) override { \
    return obj_.template call<&own_interface::METHOD>(                      \
        ASH_INTERFACE_PROXY_FORWARD_ARGS ARGS);                             \
  }
#define ASH_INTERFACE_PROXY_IMPLS_ONE(...) ASH_INTERFACE_PROXY_IMPL __VA_ARGS__
#define ASH_INTERFACE_PROXY_IMPLS_SEP()
#define ASH_INTERFACE_PROXY_IMPLS(...)                                      \
  ASH_FOREACH(ASH_INTERFACE_PROXY_IMPLS_ONE, ASH_INTERFACE_PROXY_IMPLS_SEP, \
              __VA_ARGS__)

#define ASH_INTERFACE_ASYNC_PROXY_IMPL(RETURN, METHOD, ARGS) \
  ::ash::future<ASH_EXPAND_1 RETURN> METHOD(                 \
      ASH_INTERFACE_PROXY_IMPL_ARGS ARGS) override {         \
    return obj_.template async_call<&own_interface::METHOD>( \
        ASH_INTERFACE_PROXY_FORWARD_ARGS ARGS);              \
  }
#define ASH_INTERFACE_ASYNC_PROXY_IMPLS_ONE(...) \
  ASH_INTERFACE_ASYNC_PROXY_IMPL __VA_ARGS__
#define ASH_INTERFACE_ASYNC_PROXY_IMPLS_SEP()
#define ASH_INTERFACE_ASYNC_PROXY_IMPLS(...)       \
  ASH_FOREACH(ASH_INTERFACE_ASYNC_PROXY_IMPLS_ONE, \
              ASH_INTERFACE_ASYNC_PROXY_IMPLS_SEP, __VA_ARGS__)

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

#define ASH_INTERFACE(NAME, EXTENDS, METHODS)                                  \
  struct NAME                                                                  \
      : ::ash::interface<NAME ASH_IF(ASH_NOT(ASH_IS_EMPTY EXTENDS))(           \
            , /* NOLINT(whitespace/parens) */)                                 \
                             ASH_EXPAND(ASH_INTERFACE_EXTENDS EXTENDS)> {      \
    ASH_EXPAND(ASH_INTERFACE_DECLS METHODS)                                    \
    using method_descriptors =                                                 \
        ::ash::mpt::pack<ASH_EXPAND(ASH_INTERFACE_METHODS METHODS)>;           \
    static const std::array<const char *,                                      \
                            ::ash::mpt::size_v<method_descriptors>>            \
        &method_names() {                                                      \
      static const std::array<const char *,                                    \
                              ::ash::mpt::size_v<method_descriptors>>          \
          names{ASH_EXPAND(ASH_INTERFACE_METHOD_NAMES METHODS)};               \
      return names;                                                            \
    }                                                                          \
    struct async                                                               \
        : ::ash::async_interface<NAME ASH_IF(ASH_NOT(ASH_IS_EMPTY EXTENDS))(   \
              , /* NOLINT(whitespace/parens) */)                               \
                                     ASH_EXPAND(                               \
                                         ASH_INTERFACE_EXTENDS EXTENDS)> {     \
      ASH_EXPAND(ASH_INTERFACE_ASYNC_DECLS METHODS)                            \
      template <typename O>                                                    \
      struct proxy;                                                            \
    };                                                                         \
    template <typename O>                                                      \
    struct proxy;                                                              \
  };                                                                           \
  template <typename O>                                                        \
  struct NAME::proxy : virtual NAME ASH_IF(ASH_NOT(ASH_IS_EMPTY EXTENDS))(     \
                           , /* NOLINT(whitespace/parens) */)                  \
                           ASH_EXPAND(ASH_INTERFACE_PROXY_EXTENDS EXTENDS) {   \
    proxy(O obj)                                                               \
        : ASH_EXPAND(ASH_INTERFACE_PROXY_CTOR EXTENDS)                         \
              ASH_IF(ASH_NOT(ASH_IS_EMPTY EXTENDS))(                           \
                  , /* NOLINT(whitespace/parens) */) obj_(std::move(obj)) {}   \
    ASH_EXPAND(ASH_INTERFACE_PROXY_IMPLS METHODS)                              \
   private:                                                                    \
    O obj_;                                                                    \
  };                                                                           \
  template <typename O>                                                        \
  struct NAME::async::proxy                                                    \
      : virtual NAME::async                                                    \
        ASH_IF(ASH_NOT(ASH_IS_EMPTY EXTENDS))(,                                \
                                              /* NOLINT(whitespace/parens) */) \
            ASH_EXPAND(ASH_INTERFACE_ASYNC_PROXY_EXTENDS EXTENDS) {            \
    proxy(O obj)                                                               \
        : ASH_EXPAND(ASH_INTERFACE_ASYNC_PROXY_CTOR EXTENDS)                   \
              ASH_IF(ASH_NOT(ASH_IS_EMPTY EXTENDS))(                           \
                  , /* NOLINT(whitespace/parens) */) obj_(std::move(obj)) {}   \
    ASH_EXPAND(ASH_INTERFACE_ASYNC_PROXY_IMPLS METHODS)                        \
   private:                                                                    \
    O obj_;                                                                    \
  }
}  // namespace ash

#endif  // ASH_INTERFACE_H_
