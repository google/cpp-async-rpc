/// \file
/// \brief Definitions of common exception classes.
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

#ifndef INCLUDE_ASH_ERRORS_H_
#define INCLUDE_ASH_ERRORS_H_

#include <stdexcept>
#include <string>
#include "ash/const_char_ptr_compare.h"
#include "ash/container/flat_map.h"
#include "ash/singleton.h"

#define ERROR_CLASS(NAME)                                                   \
  class NAME;                                                               \
  template <>                                                               \
  const char* ::ash::errors::detail::error_class_descriptor<                \
      NAME>::error_class_name =                                             \
      ::ash::error_factory::get().register_error_class<NAME>(#NAME);        \
                                                                            \
  class NAME : public ::ash::errors::base_error {                           \
   public:                                                                  \
    using ::ash::errors::base_error::base_error;                            \
                                                                            \
   private:                                                                 \
    const char* portable_error_class_name_internal() const override {       \
      using Descriptor = ash::errors::detail::error_class_descriptor<NAME>; \
      if (Descriptor::error_class_name == nullptr)                          \
        throw std::runtime_error("Error class had no name set");            \
      return Descriptor::error_class_name;                                  \
    }                                                                       \
  }

namespace ash {

class error_factory : public singleton<error_factory> {
 public:
  using error_function_type = void (*)(const char*);

  void throw_error(const char* error_class_name, const char* what);

  template <typename T>
  const char* register_error_class(const char* error_class_name);

 private:
  ash::flat_map<const char*, error_function_type, const_char_ptr_compare>
      error_function_map_;
};

namespace errors {
namespace detail {
template <typename T>
struct error_class_descriptor {
  static const char* error_class_name;
};
}  // namespace detail

class base_error : public std::runtime_error {
 public:
  using std::runtime_error::runtime_error;
  virtual ~base_error() {}

  const char* portable_error_class_name() const {
    return portable_error_class_name_internal();
  }

 private:
  virtual const char* portable_error_class_name_internal() const = 0;
};

// Actual error class definitions.

ERROR_CLASS(unknown_error);
ERROR_CLASS(eof);
ERROR_CLASS(shutting_down);
ERROR_CLASS(io_error);
ERROR_CLASS(not_found);
ERROR_CLASS(invalid_argument);
ERROR_CLASS(invalid_state);
ERROR_CLASS(data_mismatch);
ERROR_CLASS(out_of_range);
ERROR_CLASS(not_implemented);

}  // namespace errors

void error_factory::throw_error(const char* error_class_name,
                                const char* what) {
  auto it = error_function_map_.find(error_class_name);
  if (it == error_function_map_.end()) {
    // If we don't know the error type, just use unknown_error.
    throw errors::unknown_error(what);
  }
  // Throw the specific error type otherwise.
  it->second(what);
}

template <typename T>
const char* error_factory::register_error_class(const char* error_class_name) {
  // Register the class into this factory for object creation.
  error_function_type f = [](const char* what) { throw T(what); };
  if (!error_function_map_.emplace(error_class_name, f).second)
    throw errors::invalid_state("Duplicate class registration");

  return error_class_name;
}

}  // namespace ash

#endif  // INCLUDE_ASH_ERRORS_H_
