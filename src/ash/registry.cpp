/// \file
/// \brief Registry classes for encoders, decoders and dynamic class factories.
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

#include "ash/registry.h"

namespace ash {

namespace registry {

dynamic_object_factory::info dynamic_object_factory::operator[](
    const char* class_name) const {
  const auto it = factory_function_map_.find(class_name);
  if (it == factory_function_map_.end())
    throw errors::not_found("Class factory function not found");
  return it->second;
}

}  // namespace registry

}  // namespace ash
