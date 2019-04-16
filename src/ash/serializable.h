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

#ifndef ASH_SERIALIZABLE_H_
#define ASH_SERIALIZABLE_H_

#include "ash/config.h"
#include "ash/dynamic_base_class.h"
#include "ash/registry.h"
#include "ash/serializable_base.h"

namespace ash {

/// Register a dynamic class into the class factories.
#define ASH_REGISTER(...)                                               \
  template <>                                                           \
  std::string_view                                                      \
      ash::detail::dynamic_class_descriptor<__VA_ARGS__>::class_name =  \
          ::ash::registry::dynamic_object_factory::get()                \
              .register_class<__VA_ARGS__, ::ash::config::all_encoders, \
                              ::ash::config::all_decoders>(#__VA_ARGS__)

/// Register a dynamic class into the class factories under a custom name.
#define ASH_REGISTER_WITH_NAME(NAME, ...)                               \
  template <>                                                           \
  std::string_view                                                      \
      ash::detail::dynamic_class_descriptor<__VA_ARGS__>::class_name =  \
          ::ash::registry::dynamic_object_factory::get()                \
              .register_class<__VA_ARGS__, ::ash::config::all_encoders, \
                              ::ash::config::all_decoders>(NAME)

}  // namespace ash

#endif  // ASH_SERIALIZABLE_H_
