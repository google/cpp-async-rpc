/// \file
/// \brief Utility for defining enumerations with named entries.
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

#ifndef ARPC_ENUM_H_
#define ARPC_ENUM_H_

#include <algorithm>
#include <type_traits>
#include <utility>
#include "arpc/preprocessor.h"

namespace arpc {

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

#define ARPC_ENUM_ENTRY_IN_BODY_IMPL(NAME, VALUE) NAME = VALUE
#define ARPC_ENUM_ENTRY_IN_BODY(...) \
  ARPC_NO_PARENS(ARPC_ENUM_ENTRY_IN_BODY_IMPL, __VA_ARGS__)
#define ARPC_ENUM_ENTRY_IN_ARRAY_IMPL(NAME, VALUE) \
  { VALUE, #NAME }
#define ARPC_ENUM_ENTRY_IN_ARRAY(...) \
  ARPC_NO_PARENS(ARPC_ENUM_ENTRY_IN_ARRAY_IMPL, __VA_ARGS__)
#define ARPC_ENUM_ENTRY_SEP() ,

#define ARPC_ENUM(NAME, BASE_TYPE, UNKNOWN_NAME, ...)                        \
  enum class NAME : BASE_TYPE {                                              \
    ARPC_FOREACH(ARPC_ENUM_ENTRY_IN_BODY, ARPC_ENUM_ENTRY_SEP, __VA_ARGS__)  \
  };                                                                         \
  template <>                                                                \
  const std::size_t::arpc::enum_names<NAME>::size =                          \
      ARPC_ARG_COUNT(__VA_ARGS__);                                           \
  template <>                                                                \
  const char* ::arpc::enum_names<NAME>::unknown_name = UNKNOWN_NAME;         \
  template <>                                                                \
  const std::pair<typename ::arpc::enum_names<NAME>::base_type, const char*> \
      arpc::enum_names<NAME>::entries[] = {ARPC_FOREACH(                     \
          ARPC_ENUM_ENTRY_IN_ARRAY, ARPC_ENUM_ENTRY_SEP, __VA_ARGS__)}

}  // namespace arpc

#endif  // ARPC_ENUM_H_
