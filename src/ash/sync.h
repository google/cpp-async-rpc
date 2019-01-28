/// \channel
/// \brief select-friendly synchronization objects.
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

#ifndef ASH_SYNC_H_
#define ASH_SYNC_H_

#include <mutex>
#include "ash/io.h"

namespace ash {

class flag {
 public:
  flag();

  void set();
  void reset();
  bool is_set() const;
  explicit operator bool() const;
  awaitable wait_set() const;

 private:
  mutable std::mutex mu_;
  channel pipe_[2];
};

}  // namespace ash

#endif  // ASH_SYNC_H_
