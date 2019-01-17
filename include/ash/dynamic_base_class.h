/// \file
/// \brief Header defining a base class for polymorphic serializable data.
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

#ifndef INCLUDE_ASH_DYNAMIC_BASE_CLASS_H_
#define INCLUDE_ASH_DYNAMIC_BASE_CLASS_H_

#include <type_traits>
#include "ash/mpt.h"

namespace ash {

/// Base class for all run-time polymorphic serializable classes.
class dynamic_base_class {
 public:
  const char* portable_class_name() const {
    return portable_class_name_internal();
  }
  using field_descriptors = mpt::pack<>;
  using dynamic_base_classes = mpt::pack<>;

  /// Make sure deleters work.
  virtual ~dynamic_base_class() {}

 private:
  /// Get the string that portably identifies the object's class.
  virtual const char* portable_class_name_internal() const = 0;
};

/// Checker for whether a class `T` is dynamic.
template <typename T>
struct is_dynamic : std::is_base_of<dynamic_base_class, T> {};

template <typename T>
inline constexpr bool is_dynamic_v = is_dynamic<T>::value;

namespace detail {
template <typename T>
struct dynamic_class_descriptor {
  static const char* class_name;
};
}  // namespace detail

}  // namespace ash

#endif  // INCLUDE_ASH_DYNAMIC_BASE_CLASS_H_
