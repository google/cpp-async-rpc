/// \file
/// \brief Macros and base class support for serializable classes.
///
/// \copyright
///   Copyright 2019 by Google LLC. All Rights Reserved.
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

#ifndef LASR_SERIALIZABLE_H_
#define LASR_SERIALIZABLE_H_

#include "lasr/config.h"
#include "lasr/dynamic_base_class.h"
#include "lasr/registry.h"
#include "lasr/serializable_base.h"

namespace lasr {

/// Register a dynamic class into the class factories.
#define LASR_REGISTER(...)                                               \
  template <>                                                           \
  std::string_view                                                      \
      lasr::detail::dynamic_class_descriptor<__VA_ARGS__>::class_name =  \
          ::lasr::registry::dynamic_object_factory::get()                \
              .register_class<__VA_ARGS__, ::lasr::config::all_encoders, \
                              ::lasr::config::all_decoders>(#__VA_ARGS__)

/// Register a dynamic class into the class factories under a custom name.
#define LASR_REGISTER_WITH_NAME(NAME, ...)                               \
  template <>                                                           \
  std::string_view                                                      \
      lasr::detail::dynamic_class_descriptor<__VA_ARGS__>::class_name =  \
          ::lasr::registry::dynamic_object_factory::get()                \
              .register_class<__VA_ARGS__, ::lasr::config::all_encoders, \
                              ::lasr::config::all_decoders>(NAME)

}  // namespace lasr

#endif  // LASR_SERIALIZABLE_H_
