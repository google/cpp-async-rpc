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

#define ERROR_CLASS(NAME)                    \
  class NAME : public std::runtime_error {   \
    using std::runtime_error::runtime_error; \
  }

namespace ash {
namespace errors {
ERROR_CLASS(eof);
ERROR_CLASS(io_error);
ERROR_CLASS(key_error);
ERROR_CLASS(invalid_state);
ERROR_CLASS(data_mismatch);
}  // namespace errors
}  // namespace ash

#endif  // INCLUDE_ASH_ERRORS_H_
