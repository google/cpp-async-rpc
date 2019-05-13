/// \file
/// \brief Classes for defining RPC interfaces.
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

#ifndef ARPC_INTERFACE_H_
#define ARPC_INTERFACE_H_

#include <array>
#include <utility>
#include "arpc/future.h"
#include "arpc/message_defs.h"
#include "arpc/mpt.h"
#include "arpc/preprocessor.h"
#include "arpc/traits/type_traits.h"

namespace arpc {

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

#define ARPC_INTERFACE_EXTENDS_ONE(...) ARPC_EXPAND_1 __VA_ARGS__
#define ARPC_INTERFACE_EXTENDS_SEP() ,
#define ARPC_INTERFACE_EXTENDS(...)                                    \
  ARPC_FOREACH(ARPC_INTERFACE_EXTENDS_ONE, ARPC_INTERFACE_EXTENDS_SEP, \
               __VA_ARGS__)

#define ARPC_INTERFACE_PROXY_EXTENDS_ONE(...) \
  virtual ARPC_EXPAND_1 __VA_ARGS__::proxy<O>
#define ARPC_INTERFACE_PROXY_EXTENDS_SEP() ,
#define ARPC_INTERFACE_PROXY_EXTENDS(...)        \
  ARPC_FOREACH(ARPC_INTERFACE_PROXY_EXTENDS_ONE, \
               ARPC_INTERFACE_PROXY_EXTENDS_SEP, __VA_ARGS__)

#define ARPC_INTERFACE_ASYNC_PROXY_EXTENDS_ONE(...) \
  virtual ARPC_EXPAND_1 __VA_ARGS__::async::proxy<O>
#define ARPC_INTERFACE_ASYNC_PROXY_EXTENDS_SEP() ,
#define ARPC_INTERFACE_ASYNC_PROXY_EXTENDS(...)        \
  ARPC_FOREACH(ARPC_INTERFACE_ASYNC_PROXY_EXTENDS_ONE, \
               ARPC_INTERFACE_ASYNC_PROXY_EXTENDS_SEP, __VA_ARGS__)

#define ARPC_INTERFACE_PROXY_CTOR_ONE(...) \
  ARPC_EXPAND_1 __VA_ARGS__::proxy<O>(obj)
#define ARPC_INTERFACE_PROXY_CTOR_SEP() ,
#define ARPC_INTERFACE_PROXY_CTOR(...)                                       \
  ARPC_FOREACH(ARPC_INTERFACE_PROXY_CTOR_ONE, ARPC_INTERFACE_PROXY_CTOR_SEP, \
               __VA_ARGS__)

#define ARPC_INTERFACE_ASYNC_PROXY_CTOR_ONE(...) \
  ARPC_EXPAND_1 __VA_ARGS__::async::proxy<O>(obj)
#define ARPC_INTERFACE_ASYNC_PROXY_CTOR_SEP() ,
#define ARPC_INTERFACE_ASYNC_PROXY_CTOR(...)        \
  ARPC_FOREACH(ARPC_INTERFACE_ASYNC_PROXY_CTOR_ONE, \
               ARPC_INTERFACE_ASYNC_PROXY_CTOR_SEP, __VA_ARGS__)

#define ARPC_INTERFACE_DECL_ARG(TYPE, NAME) ARPC_EXPAND_1 TYPE NAME
#define ARPC_INTERFACE_DECL_ARGS_ONE(...) ARPC_INTERFACE_DECL_ARG __VA_ARGS__
#define ARPC_INTERFACE_DECL_ARGS_SEP() ,
#define ARPC_INTERFACE_DECL_ARGS(...) \
  ARPC_DEFER_2(ARPC_FOREACH_AGAIN)    \
  ()(ARPC_INTERFACE_DECL_ARGS_ONE, ARPC_INTERFACE_DECL_ARGS_SEP, __VA_ARGS__)
#define ARPC_INTERFACE_DECL(RETURN, METHOD, ARGS) \
  virtual ARPC_EXPAND_1 RETURN METHOD(ARPC_INTERFACE_DECL_ARGS ARGS) = 0;
#define ARPC_INTERFACE_DECLS_ONE(...) ARPC_INTERFACE_DECL __VA_ARGS__
#define ARPC_INTERFACE_DECLS_SEP()
#define ARPC_INTERFACE_DECLS(...) \
  ARPC_FOREACH(ARPC_INTERFACE_DECLS_ONE, ARPC_INTERFACE_DECLS_SEP, __VA_ARGS__)

#define ARPC_INTERFACE_ASYNC_DECL(RETURN, METHOD, ARGS)     \
  virtual ::std::pair<::arpc::future<ARPC_EXPAND_1 RETURN>, \
                      ::arpc::rpc_defs::request_id_type>    \
  METHOD(ARPC_INTERFACE_DECL_ARGS ARGS) = 0;
#define ARPC_INTERFACE_ASYNC_DECLS_ONE(...) \
  ARPC_INTERFACE_ASYNC_DECL __VA_ARGS__
#define ARPC_INTERFACE_ASYNC_DECLS_SEP()
#define ARPC_INTERFACE_ASYNC_DECLS(...)                                        \
  ARPC_FOREACH(ARPC_INTERFACE_ASYNC_DECLS_ONE, ARPC_INTERFACE_ASYNC_DECLS_SEP, \
               __VA_ARGS__)

#define ARPC_INTERFACE_PROXY_IMPL_ARG(TYPE, NAME) ARPC_EXPAND_1 TYPE NAME
#define ARPC_INTERFACE_PROXY_IMPL_ARGS_ONE(...) \
  ARPC_INTERFACE_PROXY_IMPL_ARG __VA_ARGS__
#define ARPC_INTERFACE_PROXY_IMPL_ARGS_SEP() ,
#define ARPC_INTERFACE_PROXY_IMPL_ARGS(...)                                  \
  ARPC_DEFER_2(ARPC_FOREACH_AGAIN)                                           \
  ()(ARPC_INTERFACE_PROXY_IMPL_ARGS_ONE, ARPC_INTERFACE_PROXY_IMPL_ARGS_SEP, \
     __VA_ARGS__)
#define ARPC_INTERFACE_PROXY_FORWARD_ARG(TYPE, NAME) \
  std::forward<ARPC_EXPAND_1 TYPE>(NAME)
#define ARPC_INTERFACE_PROXY_FORWARD_ARGS_ONE(...) \
  ARPC_INTERFACE_PROXY_FORWARD_ARG __VA_ARGS__
#define ARPC_INTERFACE_PROXY_FORWARD_ARGS_SEP() ,
#define ARPC_INTERFACE_PROXY_FORWARD_ARGS(...) \
  ARPC_DEFER_2(ARPC_FOREACH_AGAIN)             \
  ()(ARPC_INTERFACE_PROXY_FORWARD_ARGS_ONE,    \
     ARPC_INTERFACE_PROXY_FORWARD_ARGS_SEP, __VA_ARGS__)
#define ARPC_INTERFACE_PROXY_IMPL(RETURN, METHOD, ARGS)                       \
  ARPC_EXPAND_1 RETURN METHOD(ARPC_INTERFACE_PROXY_IMPL_ARGS ARGS) override { \
    return obj_.template call<&own_interface::METHOD>(                        \
        ARPC_INTERFACE_PROXY_FORWARD_ARGS ARGS);                              \
  }
#define ARPC_INTERFACE_PROXY_IMPLS_ONE(...) \
  ARPC_INTERFACE_PROXY_IMPL __VA_ARGS__
#define ARPC_INTERFACE_PROXY_IMPLS_SEP()
#define ARPC_INTERFACE_PROXY_IMPLS(...)                                        \
  ARPC_FOREACH(ARPC_INTERFACE_PROXY_IMPLS_ONE, ARPC_INTERFACE_PROXY_IMPLS_SEP, \
               __VA_ARGS__)

#define ARPC_INTERFACE_ASYNC_PROXY_IMPL(RETURN, METHOD, ARGS) \
  ::std::pair<::arpc::future<ARPC_EXPAND_1 RETURN>,           \
              ::arpc::rpc_defs::request_id_type>              \
  METHOD(ARPC_INTERFACE_PROXY_IMPL_ARGS ARGS) override {      \
    return obj_.template async_call<&own_interface::METHOD>(  \
        ARPC_INTERFACE_PROXY_FORWARD_ARGS ARGS);              \
  }
#define ARPC_INTERFACE_ASYNC_PROXY_IMPLS_ONE(...) \
  ARPC_INTERFACE_ASYNC_PROXY_IMPL __VA_ARGS__
#define ARPC_INTERFACE_ASYNC_PROXY_IMPLS_SEP()
#define ARPC_INTERFACE_ASYNC_PROXY_IMPLS(...)        \
  ARPC_FOREACH(ARPC_INTERFACE_ASYNC_PROXY_IMPLS_ONE, \
               ARPC_INTERFACE_ASYNC_PROXY_IMPLS_SEP, __VA_ARGS__)

#define ARPC_INTERFACE_METHOD(RETURN, METHOD, ARGS) \
  ::arpc::method_descriptor<&own_interface::METHOD>
#define ARPC_INTERFACE_METHODS_ONE(...) ARPC_INTERFACE_METHOD __VA_ARGS__
#define ARPC_INTERFACE_METHODS_SEP() ,
#define ARPC_INTERFACE_METHODS(...)                                    \
  ARPC_FOREACH(ARPC_INTERFACE_METHODS_ONE, ARPC_INTERFACE_METHODS_SEP, \
               __VA_ARGS__)

#define ARPC_INTERFACE_METHOD_NAME(RETURN, METHOD, ARGS) #METHOD
#define ARPC_INTERFACE_METHOD_NAMES_ONE(...) \
  ARPC_INTERFACE_METHOD_NAME __VA_ARGS__
#define ARPC_INTERFACE_METHOD_NAMES_SEP() ,
#define ARPC_INTERFACE_METHOD_NAMES(...)        \
  ARPC_FOREACH(ARPC_INTERFACE_METHOD_NAMES_ONE, \
               ARPC_INTERFACE_METHOD_NAMES_SEP, __VA_ARGS__)

#define ARPC_INTERFACE(NAME, EXTENDS, METHODS)                                 \
  struct NAME                                                                  \
      : ::arpc::interface<NAME ARPC_IF(ARPC_NOT(ARPC_IS_EMPTY EXTENDS))(       \
            , /* NOLINT(whitespace/parens) */)                                 \
                              ARPC_EXPAND(ARPC_INTERFACE_EXTENDS EXTENDS)> {   \
    ARPC_EXPAND(ARPC_INTERFACE_DECLS METHODS)                                  \
    using method_descriptors =                                                 \
        ::arpc::mpt::pack<ARPC_EXPAND(ARPC_INTERFACE_METHODS METHODS)>;        \
    static const std::array<const char *,                                      \
                            ::arpc::mpt::size_v<method_descriptors>>           \
        &method_names() {                                                      \
      static const std::array<const char *,                                    \
                              ::arpc::mpt::size_v<method_descriptors>>         \
          names{ARPC_EXPAND(ARPC_INTERFACE_METHOD_NAMES METHODS)};             \
      return names;                                                            \
    }                                                                          \
    struct async : ::arpc::async_interface<                                    \
                       NAME ARPC_IF(ARPC_NOT(ARPC_IS_EMPTY EXTENDS))(          \
                           , /* NOLINT(whitespace/parens) */)                  \
                           ARPC_EXPAND(ARPC_INTERFACE_EXTENDS EXTENDS)> {      \
      ARPC_EXPAND(ARPC_INTERFACE_ASYNC_DECLS METHODS)                          \
      template <typename O>                                                    \
      struct proxy;                                                            \
    };                                                                         \
    template <typename O>                                                      \
    struct proxy;                                                              \
  };                                                                           \
  template <typename O>                                                        \
  struct NAME::proxy : virtual NAME ARPC_IF(ARPC_NOT(ARPC_IS_EMPTY EXTENDS))(  \
                           , /* NOLINT(whitespace/parens) */)                  \
                           ARPC_EXPAND(ARPC_INTERFACE_PROXY_EXTENDS EXTENDS) { \
    proxy(O obj)                                                               \
        : ARPC_EXPAND(ARPC_INTERFACE_PROXY_CTOR EXTENDS)                       \
              ARPC_IF(ARPC_NOT(ARPC_IS_EMPTY EXTENDS))(                        \
                  , /* NOLINT(whitespace/parens) */) obj_(std::move(obj)) {}   \
    ARPC_EXPAND(ARPC_INTERFACE_PROXY_IMPLS METHODS)                            \
   private:                                                                    \
    O obj_;                                                                    \
  };                                                                           \
  template <typename O>                                                        \
  struct NAME::async::proxy                                                    \
      : virtual NAME::async                                                    \
        ARPC_IF(ARPC_NOT(ARPC_IS_EMPTY EXTENDS))(                              \
            , /* NOLINT(whitespace/parens) */)                                 \
            ARPC_EXPAND(ARPC_INTERFACE_ASYNC_PROXY_EXTENDS EXTENDS) {          \
    proxy(O obj)                                                               \
        : ARPC_EXPAND(ARPC_INTERFACE_ASYNC_PROXY_CTOR EXTENDS)                 \
              ARPC_IF(ARPC_NOT(ARPC_IS_EMPTY EXTENDS))(                        \
                  , /* NOLINT(whitespace/parens) */) obj_(std::move(obj)) {}   \
    ARPC_EXPAND(ARPC_INTERFACE_ASYNC_PROXY_IMPLS METHODS)                      \
   private:                                                                    \
    O obj_;                                                                    \
  }
}  // namespace arpc

#endif  // ARPC_INTERFACE_H_
