/// \file
/// \brief select-friendly flag objects.
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

#include "ash/flag.h"

namespace ash {

flag::flag() {
  pipe(pipe_);
  pipe_[0].make_non_blocking();
  pipe_[1].make_non_blocking();
}

void flag::set() {
  std::scoped_lock lock(mu_);
  if (!set_) {
    pipe_[1].write("*", 1);
    set_ = true;
  }
}

void flag::reset() {
  std::scoped_lock lock(mu_);
  if (set_) {
    char c;
    pipe_[0].read(&c, 1);
    set_ = false;
  }
}

bool flag::is_set() const {
  std::scoped_lock lock(mu_);
  return set_;
}

flag::operator bool() const { return is_set(); }

awaitable<void> flag::wait_set() { return pipe_[0].can_read(); }

}  // namespace ash
