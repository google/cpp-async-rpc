/// \file
/// \brief Pipe channel factory.
///
/// \copyright
///   Copyright 2018 by Google Inc. All Rights Reserved.
///
/// \copyright
///   Licensed under the Apache License, Version 2.0 (the "License"); you may
///   not use this channel except in compliance with the License. You may obtain
///   a copy of the License at
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

#include <ash/file.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include "ash/errors.h"

namespace ash {

channel file(const std::string& path, open_mode mode) {
  static constexpr int posix_modes[] = {
      O_RDONLY,                       // READ
      O_WRONLY | O_CREAT | O_TRUNC,   // WRITE
      O_WRONLY | O_CREAT | O_APPEND,  // APPEND
      O_RDWR,                         // READ_PLUS
      O_RDWR | O_CREAT | O_TRUNC,     // WRITE_PLUS
      O_RDWR | O_CREAT | O_APPEND,    // APPEND
  };
  channel res(
      ::open(path.c_str(), posix_modes[static_cast<std::size_t>(mode)]));
  if (!res) throw_io_error("Error opening file", res.get());
  return res;
}

}  // namespace ash
