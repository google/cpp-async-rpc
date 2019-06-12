/// \file
/// \brief Implementations of common exception classes.
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

#include "arpc/errors.h"

namespace arpc {

namespace errors {

base_error::~base_error() {}

}  // namespace errors

void error_factory::throw_error(std::string_view error_class_name,
                                const char* what) {
  auto it = error_function_map_.find(error_class_name);
  if (it == error_function_map_.end()) {
    // If we don't know the error type, just use unknown_error.
    throw errors::unknown_error(what);
  }
  // Throw the specific error type otherwise.
  it->second(what);
}

std::pair<std::string, std::string> error_factory::analyze_exception(
    std::exception_ptr exc) {
  std::string type, what;

  if (exc) {
    try {
      std::rethrow_exception(exc);
    } catch (const errors::base_error& e) {
      type = e.portable_error_class_name();
      what = e.what();
    } catch (const std::exception& e) {
      what = e.what();
    } catch (...) {
    }
  }

  return {type, what};
}

void throw_io_error(const std::string& message, int code) {
  if (code == EINPROGRESS || code == EALREADY || code == EISCONN) {
    // This is just an ongoing non-blocking connection. Nothing to see here.
    return;
  }
  if (code == EAGAIN || code == EWOULDBLOCK) {
    throw errors::try_again("Try again");
  }
  throw errors::io_error(message + std::string(": ") + std::to_string(code));
}

void throw_io_error(const std::string& message) {
  throw_io_error(message, errno);
}

}  // namespace arpc
