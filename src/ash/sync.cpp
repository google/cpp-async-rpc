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

#include "ash/sync.h"
#include "ash/errors.h"

namespace ash {

flag::flag() {
  pipe(pipe_);
  pipe_[0].make_non_blocking();
  pipe_[1].make_non_blocking();
}

void flag::set() {
  std::scoped_lock lock(mu_);
  pipe_[1].write("*", 1);
  set_ = true;
}

void flag::reset() {
  std::scoped_lock lock(mu_);
  do {
    try {
      char c;
      pipe_[0].read(&c, 1);
    } catch (const errors::try_again&) {
      set_ = false;
      return;
    }
  } while (true);
}

bool flag::is_set() const {
  std::scoped_lock lock(mu_);
  return set_;
}

flag::operator bool() const { return is_set(); }

awaitable flag::wait() { return pipe_[0].read(); }

notification::notification() {
  pipe(pipe_);
  pipe_[0].make_non_blocking();
  pipe_[1].make_non_blocking();
}

void notification::notify_one() {
  std::scoped_lock lock(mu_);
  if (num_waiters_) pipe_[1].write("*", 1);
}
void notification::notify_all() {
  std::scoped_lock lock(mu_);
  for (std::size_t i = 0; i < num_waiters_; i++) pipe_[1].write("*", 1);
}

awaitable notification::wait() {
  std::scoped_lock lock(mu_);
  num_waiters_++;
  return awaitable(pipe_[0], false,
                   [this]() {
                     try {
                       char c;
                       pipe_[0].read(&c, 1);
                       return true;
                     } catch (const errors::try_again&) {
                       return false;
                     }
                   },
                   [this]() {
                     std::scoped_lock lock(mu_);
                     num_waiters_--;
                   });
}

}  // namespace ash
