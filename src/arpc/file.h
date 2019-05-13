/// \file
/// \brief File channel factory.
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

#ifndef ARPC_FILE_H_
#define ARPC_FILE_H_

#include <string>
#include "arpc/channel.h"

namespace arpc {

enum class open_mode : int {
  READ,
  WRITE,
  APPEND,
  READ_PLUS,
  WRITE_PLUS,
  APPEND_PLUS,
};

channel file(const std::string& path, open_mode mode = open_mode::READ);

}  // namespace arpc

#endif  // ARPC_FILE_H_
