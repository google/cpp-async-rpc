/// \file
/// \brief Socket channel factory.
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

#ifndef ASH_SOCKET_H_
#define ASH_SOCKET_H_

#include <vector>
#include "ash/address.h"
#include "ash/channel.h"

namespace ash {

channel socket(int family, int type, int protocol);
channel socket(const address& addr);

channel dial(endpoint name, bool non_blocking = true);

class listener {
 public:
  explicit listener(endpoint name, bool reuse_addr = true,
                    bool non_blocking = true,
                    int backlog = channel::default_backlog);

  channel accept();

 private:
  bool non_blocking_;
  std::vector<channel> listening_, pending_;
  std::vector<awaitable<channel>> acceptors_;
};

}  // namespace ash

#endif  // ASH_SOCKET_H_
