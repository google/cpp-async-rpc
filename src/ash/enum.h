/// \file
/// \brief Utility for defining enumerations with named entries.
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

#ifndef INCLUDE_ASH_ENUM_H_
#define INCLUDE_ASH_ENUM_H_

#include <algorithm>
#include <type_traits>
#include <utility>
#include "ash/preprocessor.h"

namespace ash {

template <typename T>
struct enum_names {
  static const char* name(const T& value) {
    const base_type v = static_cast<base_type>(value);
    auto it = std::find_if(entries, entries + size,
                           [&v](const std::pair<base_type, const char*>& x) {
                             return x.first == v;
                           });
    if (it == entries + size) {
      return unknown_name;
    } else {
      return it->second;
    }
  }

 private:
  using base_type = std::underlying_type_t<T>;
  static const std::size_t size;
  static const char* unknown_name;
  static const std::pair<base_type, const char*> entries[];
};

#define ASH_ENUM_ENTRY_IN_BODY_IMPL(NAME, VALUE) NAME = VALUE
#define ASH_ENUM_ENTRY_IN_BODY(...) \
  ASH_NO_PARENS(ASH_ENUM_ENTRY_IN_BODY_IMPL, __VA_ARGS__)
#define ASH_ENUM_ENTRY_IN_ARRAY_IMPL(NAME, VALUE) \
  { VALUE, #NAME }
#define ASH_ENUM_ENTRY_IN_ARRAY(...) \
  ASH_NO_PARENS(ASH_ENUM_ENTRY_IN_ARRAY_IMPL, __VA_ARGS__)
#define ASH_ENUM_ENTRY_SEP() ,

#define ASH_ENUM(NAME, BASE_TYPE, UNKNOWN_NAME, ...)                           \
  enum class NAME : BASE_TYPE {                                                \
    ASH_FOREACH(ASH_ENUM_ENTRY_IN_BODY, ASH_ENUM_ENTRY_SEP, __VA_ARGS__)       \
  };                                                                           \
  template <>                                                                  \
  const std::size_t::ash::enum_names<NAME>::size = ASH_ARG_COUNT(__VA_ARGS__); \
  template <>                                                                  \
  const char* ::ash::enum_names<NAME>::unknown_name = UNKNOWN_NAME;            \
  template <>                                                                  \
  const std::pair<typename ::ash::enum_names<NAME>::base_type, const char*>    \
      ash::enum_names<NAME>::entries[] = {ASH_FOREACH(                         \
          ASH_ENUM_ENTRY_IN_ARRAY, ASH_ENUM_ENTRY_SEP, __VA_ARGS__)}

}  // namespace ash

#endif  // INCLUDE_ASH_ENUM_H_
