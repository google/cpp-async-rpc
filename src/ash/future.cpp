/// \file
/// \brief select-friendly futures and promises.
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

#include "ash/future.h"

namespace ash {
namespace detail {

future_state_base::~future_state_base() {}

void future_state_base::release_reader() {
  bool has_writer;
  {
    std::scoped_lock lock(mu_);
    has_reader_ = false;
    has_writer = has_writer_;
  }
  if (!has_writer) {
    delete this;
  }
}

void future_state_base::release_writer() {
  bool has_reader;
  {
    std::scoped_lock lock(mu_);
    has_writer_ = false;
    has_reader = has_reader_;
  }
  if (!has_reader) {
    delete this;
  }
}

awaitable<void> future_state_base::can_get() { return set_.wait_set(); }

}  // namespace detail
}  // namespace ash
