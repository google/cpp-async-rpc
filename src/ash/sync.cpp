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

namespace ash {

mutex::mutex() {
  pipe(pipe_);
  pipe_[0].make_non_blocking();
  pipe_[1].make_non_blocking();
  pipe_[1].write("*", 1);
}

void mutex::lock() { select(async_lock()); }

void mutex::maybe_lock() {
  char c;
  pipe_[0].read(&c, 1);
}

bool mutex::try_lock() {
  try {
    maybe_lock();
    return true;
  } catch (const errors::try_again&) {
    return false;
  }
}

void mutex::unlock() { pipe_[1].write("*", 1); }

awaitable<void> mutex::can_lock() { return pipe_[0].can_read(); }
awaitable<void> mutex::async_lock() {
  return std::move(can_lock().then(std::move([this]() { maybe_lock(); })));
}

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
