/// \file
/// \brief Macros and base class support for serializable classes.
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

#ifndef ARPC_SERIALIZABLE_H_
#define ARPC_SERIALIZABLE_H_

#include "arpc/config.h"
#include "arpc/dynamic_base_class.h"
#include "arpc/registry.h"
#include "arpc/serializable_base.h"

namespace arpc {

/// Register a dynamic class into the class factories.
#define ARPC_REGISTER(...)                                               \
  template <>                                                           \
  std::string_view                                                      \
      arpc::detail::dynamic_class_descriptor<__VA_ARGS__>::class_name =  \
          ::arpc::registry::dynamic_object_factory::get()                \
              .register_class<__VA_ARGS__, ::arpc::config::all_encoders, \
                              ::arpc::config::all_decoders>(#__VA_ARGS__)

/// Register a dynamic class into the class factories under a custom name.
#define ARPC_REGISTER_WITH_NAME(NAME, ...)                               \
  template <>                                                           \
  std::string_view                                                      \
      arpc::detail::dynamic_class_descriptor<__VA_ARGS__>::class_name =  \
          ::arpc::registry::dynamic_object_factory::get()                \
              .register_class<__VA_ARGS__, ::arpc::config::all_encoders, \
                              ::arpc::config::all_decoders>(NAME)

}  // namespace arpc

#endif  // ARPC_SERIALIZABLE_H_
