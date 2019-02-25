/// \file
/// \brief Socket channel factory.
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

#include <ash/socket.h>
#include <sys/socket.h>
#include <sys/types.h>
#include "ash/errors.h"

namespace ash {
channel socket(int family, int type, int protocol) {
  channel res(::socket(family, type, protocol));
  if (!res) throw_io_error("Error creating socket");
  return res;
}

}  // namespace ash
