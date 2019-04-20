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

#ifndef LASR_INTERFACE_H_
#define LASR_INTERFACE_H_

#include <array>
#include <utility>
#include "lasr/future.h"
#include "lasr/message_defs.h"
#include "lasr/mpt.h"
#include "lasr/preprocessor.h"
#include "lasr/traits/type_traits.h"

namespace lasr {

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

#define LASR_INTERFACE_EXTENDS_ONE(...) LASR_EXPAND_1 __VA_ARGS__
#define LASR_INTERFACE_EXTENDS_SEP() ,
#define LASR_INTERFACE_EXTENDS(...)                                    \
  LASR_FOREACH(LASR_INTERFACE_EXTENDS_ONE, LASR_INTERFACE_EXTENDS_SEP, \
               __VA_ARGS__)

#define LASR_INTERFACE_PROXY_EXTENDS_ONE(...) \
  virtual LASR_EXPAND_1 __VA_ARGS__::proxy<O>
#define LASR_INTERFACE_PROXY_EXTENDS_SEP() ,
#define LASR_INTERFACE_PROXY_EXTENDS(...)        \
  LASR_FOREACH(LASR_INTERFACE_PROXY_EXTENDS_ONE, \
               LASR_INTERFACE_PROXY_EXTENDS_SEP, __VA_ARGS__)

#define LASR_INTERFACE_ASYNC_PROXY_EXTENDS_ONE(...) \
  virtual LASR_EXPAND_1 __VA_ARGS__::async::proxy<O>
#define LASR_INTERFACE_ASYNC_PROXY_EXTENDS_SEP() ,
#define LASR_INTERFACE_ASYNC_PROXY_EXTENDS(...)        \
  LASR_FOREACH(LASR_INTERFACE_ASYNC_PROXY_EXTENDS_ONE, \
               LASR_INTERFACE_ASYNC_PROXY_EXTENDS_SEP, __VA_ARGS__)

#define LASR_INTERFACE_PROXY_CTOR_ONE(...) \
  LASR_EXPAND_1 __VA_ARGS__::proxy<O>(obj)
#define LASR_INTERFACE_PROXY_CTOR_SEP() ,
#define LASR_INTERFACE_PROXY_CTOR(...)                                       \
  LASR_FOREACH(LASR_INTERFACE_PROXY_CTOR_ONE, LASR_INTERFACE_PROXY_CTOR_SEP, \
               __VA_ARGS__)

#define LASR_INTERFACE_ASYNC_PROXY_CTOR_ONE(...) \
  LASR_EXPAND_1 __VA_ARGS__::async::proxy<O>(obj)
#define LASR_INTERFACE_ASYNC_PROXY_CTOR_SEP() ,
#define LASR_INTERFACE_ASYNC_PROXY_CTOR(...)        \
  LASR_FOREACH(LASR_INTERFACE_ASYNC_PROXY_CTOR_ONE, \
               LASR_INTERFACE_ASYNC_PROXY_CTOR_SEP, __VA_ARGS__)

#define LASR_INTERFACE_DECL_ARG(TYPE, NAME) LASR_EXPAND_1 TYPE NAME
#define LASR_INTERFACE_DECL_ARGS_ONE(...) LASR_INTERFACE_DECL_ARG __VA_ARGS__
#define LASR_INTERFACE_DECL_ARGS_SEP() ,
#define LASR_INTERFACE_DECL_ARGS(...) \
  LASR_DEFER_2(LASR_FOREACH_AGAIN)    \
  ()(LASR_INTERFACE_DECL_ARGS_ONE, LASR_INTERFACE_DECL_ARGS_SEP, __VA_ARGS__)
#define LASR_INTERFACE_DECL(RETURN, METHOD, ARGS) \
  virtual LASR_EXPAND_1 RETURN METHOD(LASR_INTERFACE_DECL_ARGS ARGS) = 0;
#define LASR_INTERFACE_DECLS_ONE(...) LASR_INTERFACE_DECL __VA_ARGS__
#define LASR_INTERFACE_DECLS_SEP()
#define LASR_INTERFACE_DECLS(...) \
  LASR_FOREACH(LASR_INTERFACE_DECLS_ONE, LASR_INTERFACE_DECLS_SEP, __VA_ARGS__)

#define LASR_INTERFACE_ASYNC_DECL(RETURN, METHOD, ARGS)     \
  virtual ::std::pair<::lasr::future<LASR_EXPAND_1 RETURN>, \
                      ::lasr::rpc_defs::request_id_type>    \
  METHOD(LASR_INTERFACE_DECL_ARGS ARGS) = 0;
#define LASR_INTERFACE_ASYNC_DECLS_ONE(...) \
  LASR_INTERFACE_ASYNC_DECL __VA_ARGS__
#define LASR_INTERFACE_ASYNC_DECLS_SEP()
#define LASR_INTERFACE_ASYNC_DECLS(...)                                        \
  LASR_FOREACH(LASR_INTERFACE_ASYNC_DECLS_ONE, LASR_INTERFACE_ASYNC_DECLS_SEP, \
               __VA_ARGS__)

#define LASR_INTERFACE_PROXY_IMPL_ARG(TYPE, NAME) LASR_EXPAND_1 TYPE NAME
#define LASR_INTERFACE_PROXY_IMPL_ARGS_ONE(...) \
  LASR_INTERFACE_PROXY_IMPL_ARG __VA_ARGS__
#define LASR_INTERFACE_PROXY_IMPL_ARGS_SEP() ,
#define LASR_INTERFACE_PROXY_IMPL_ARGS(...)                                  \
  LASR_DEFER_2(LASR_FOREACH_AGAIN)                                           \
  ()(LASR_INTERFACE_PROXY_IMPL_ARGS_ONE, LASR_INTERFACE_PROXY_IMPL_ARGS_SEP, \
     __VA_ARGS__)
#define LASR_INTERFACE_PROXY_FORWARD_ARG(TYPE, NAME) \
  std::forward<LASR_EXPAND_1 TYPE>(NAME)
#define LASR_INTERFACE_PROXY_FORWARD_ARGS_ONE(...) \
  LASR_INTERFACE_PROXY_FORWARD_ARG __VA_ARGS__
#define LASR_INTERFACE_PROXY_FORWARD_ARGS_SEP() ,
#define LASR_INTERFACE_PROXY_FORWARD_ARGS(...) \
  LASR_DEFER_2(LASR_FOREACH_AGAIN)             \
  ()(LASR_INTERFACE_PROXY_FORWARD_ARGS_ONE,    \
     LASR_INTERFACE_PROXY_FORWARD_ARGS_SEP, __VA_ARGS__)
#define LASR_INTERFACE_PROXY_IMPL(RETURN, METHOD, ARGS)                       \
  LASR_EXPAND_1 RETURN METHOD(LASR_INTERFACE_PROXY_IMPL_ARGS ARGS) override { \
    return obj_.template call<&own_interface::METHOD>(                        \
        LASR_INTERFACE_PROXY_FORWARD_ARGS ARGS);                              \
  }
#define LASR_INTERFACE_PROXY_IMPLS_ONE(...) \
  LASR_INTERFACE_PROXY_IMPL __VA_ARGS__
#define LASR_INTERFACE_PROXY_IMPLS_SEP()
#define LASR_INTERFACE_PROXY_IMPLS(...)                                        \
  LASR_FOREACH(LASR_INTERFACE_PROXY_IMPLS_ONE, LASR_INTERFACE_PROXY_IMPLS_SEP, \
               __VA_ARGS__)

#define LASR_INTERFACE_ASYNC_PROXY_IMPL(RETURN, METHOD, ARGS) \
  ::std::pair<::lasr::future<LASR_EXPAND_1 RETURN>,           \
              ::lasr::rpc_defs::request_id_type>              \
  METHOD(LASR_INTERFACE_PROXY_IMPL_ARGS ARGS) override {      \
    return obj_.template async_call<&own_interface::METHOD>(  \
        LASR_INTERFACE_PROXY_FORWARD_ARGS ARGS);              \
  }
#define LASR_INTERFACE_ASYNC_PROXY_IMPLS_ONE(...) \
  LASR_INTERFACE_ASYNC_PROXY_IMPL __VA_ARGS__
#define LASR_INTERFACE_ASYNC_PROXY_IMPLS_SEP()
#define LASR_INTERFACE_ASYNC_PROXY_IMPLS(...)        \
  LASR_FOREACH(LASR_INTERFACE_ASYNC_PROXY_IMPLS_ONE, \
               LASR_INTERFACE_ASYNC_PROXY_IMPLS_SEP, __VA_ARGS__)

#define LASR_INTERFACE_METHOD(RETURN, METHOD, ARGS) \
  ::lasr::method_descriptor<&own_interface::METHOD>
#define LASR_INTERFACE_METHODS_ONE(...) LASR_INTERFACE_METHOD __VA_ARGS__
#define LASR_INTERFACE_METHODS_SEP() ,
#define LASR_INTERFACE_METHODS(...)                                    \
  LASR_FOREACH(LASR_INTERFACE_METHODS_ONE, LASR_INTERFACE_METHODS_SEP, \
               __VA_ARGS__)

#define LASR_INTERFACE_METHOD_NAME(RETURN, METHOD, ARGS) #METHOD
#define LASR_INTERFACE_METHOD_NAMES_ONE(...) \
  LASR_INTERFACE_METHOD_NAME __VA_ARGS__
#define LASR_INTERFACE_METHOD_NAMES_SEP() ,
#define LASR_INTERFACE_METHOD_NAMES(...)        \
  LASR_FOREACH(LASR_INTERFACE_METHOD_NAMES_ONE, \
               LASR_INTERFACE_METHOD_NAMES_SEP, __VA_ARGS__)

#define LASR_INTERFACE(NAME, EXTENDS, METHODS)                                 \
  struct NAME                                                                  \
      : ::lasr::interface<NAME LASR_IF(LASR_NOT(LASR_IS_EMPTY EXTENDS))(       \
            , /* NOLINT(whitespace/parens) */)                                 \
                              LASR_EXPAND(LASR_INTERFACE_EXTENDS EXTENDS)> {   \
    LASR_EXPAND(LASR_INTERFACE_DECLS METHODS)                                  \
    using method_descriptors =                                                 \
        ::lasr::mpt::pack<LASR_EXPAND(LASR_INTERFACE_METHODS METHODS)>;        \
    static const std::array<const char *,                                      \
                            ::lasr::mpt::size_v<method_descriptors>>           \
        &method_names() {                                                      \
      static const std::array<const char *,                                    \
                              ::lasr::mpt::size_v<method_descriptors>>         \
          names{LASR_EXPAND(LASR_INTERFACE_METHOD_NAMES METHODS)};             \
      return names;                                                            \
    }                                                                          \
    struct async : ::lasr::async_interface<                                    \
                       NAME LASR_IF(LASR_NOT(LASR_IS_EMPTY EXTENDS))(          \
                           , /* NOLINT(whitespace/parens) */)                  \
                           LASR_EXPAND(LASR_INTERFACE_EXTENDS EXTENDS)> {      \
      LASR_EXPAND(LASR_INTERFACE_ASYNC_DECLS METHODS)                          \
      template <typename O>                                                    \
      struct proxy;                                                            \
    };                                                                         \
    template <typename O>                                                      \
    struct proxy;                                                              \
  };                                                                           \
  template <typename O>                                                        \
  struct NAME::proxy : virtual NAME LASR_IF(LASR_NOT(LASR_IS_EMPTY EXTENDS))(  \
                           , /* NOLINT(whitespace/parens) */)                  \
                           LASR_EXPAND(LASR_INTERFACE_PROXY_EXTENDS EXTENDS) { \
    proxy(O obj)                                                               \
        : LASR_EXPAND(LASR_INTERFACE_PROXY_CTOR EXTENDS)                       \
              LASR_IF(LASR_NOT(LASR_IS_EMPTY EXTENDS))(                        \
                  , /* NOLINT(whitespace/parens) */) obj_(std::move(obj)) {}   \
    LASR_EXPAND(LASR_INTERFACE_PROXY_IMPLS METHODS)                            \
   private:                                                                    \
    O obj_;                                                                    \
  };                                                                           \
  template <typename O>                                                        \
  struct NAME::async::proxy                                                    \
      : virtual NAME::async                                                    \
        LASR_IF(LASR_NOT(LASR_IS_EMPTY EXTENDS))(                              \
            , /* NOLINT(whitespace/parens) */)                                 \
            LASR_EXPAND(LASR_INTERFACE_ASYNC_PROXY_EXTENDS EXTENDS) {          \
    proxy(O obj)                                                               \
        : LASR_EXPAND(LASR_INTERFACE_ASYNC_PROXY_CTOR EXTENDS)                 \
              LASR_IF(LASR_NOT(LASR_IS_EMPTY EXTENDS))(                        \
                  , /* NOLINT(whitespace/parens) */) obj_(std::move(obj)) {}   \
    LASR_EXPAND(LASR_INTERFACE_ASYNC_PROXY_IMPLS METHODS)                      \
   private:                                                                    \
    O obj_;                                                                    \
  }
}  // namespace lasr

#endif  // LASR_INTERFACE_H_
