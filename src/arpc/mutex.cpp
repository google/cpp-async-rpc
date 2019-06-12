/// \file
/// \brief select-friendly mutex objects.
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

#include "arpc/mutex.h"
#include <unistd.h>
#include <utility>
#include "arpc/context.h"
#include "arpc/errors.h"
#include "arpc/pipe.h"
#include "arpc/select.h"

namespace arpc {

mutex::mutex() {
  pipe(pipe_);
  pipe_[0].make_non_blocking();
  pipe_[1].make_non_blocking();
  pipe_[1].write("*", 1);
}

void mutex::lock() {
  auto [res] = select(async_lock());
  *res;
}

void mutex::maybe_lock() {
  char c;
  pipe_[0].maybe_read(&c, 1);
}

bool mutex::try_lock() {
  try {
    maybe_lock();
    return true;
  } catch (const errors::try_again&) {
    return false;
  }
}

void mutex::unlock() {
  shield shield;
  pipe_[1].write("*", 1);
}

awaitable<void> mutex::can_lock() { return pipe_[0].can_read(); }
awaitable<void> mutex::async_lock() {
  return can_lock().then([this]() { maybe_lock(); });
}

}  // namespace arpc
