/// \file
/// \brief Header defining a base class for polymorphic serializable data.
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

#include "lasr/dynamic_base_class.h"

namespace lasr {

std::string_view dynamic_base_class::portable_class_name() const {
  return portable_class_name_internal();
}

dynamic_base_class::~dynamic_base_class() {}

}  // namespace ash
