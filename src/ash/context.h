/// \file
/// \brief Execution context with deadlines and cancellation.
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

#ifndef ASH_CONTEXT_H_
#define ASH_CONTEXT_H_

#include <chrono>
#include <mutex>
#include "ash/awaitable.h"
#include "ash/container/flat_set.h"
#include "ash/sync.h"

namespace ash {

class context {
 public:
  using time_point = std::chrono::system_clock::time_point;
  using duration = std::chrono::system_clock::duration;

  ~context();

  void cancel();

  bool is_cancelled();

  time_point deadline();

  duration deadline_left();

  awaitable<void> wait_cancelled();

  static context& current();

  static context with_deadline(time_point when);

  static context with_timeout(duration timeout);

  static context with_cancel();

 private:
  explicit context(time_point deadline = time_point::max());
  void add_child(context* child);
  void remove_child(context* child);

  std::mutex mu_;
  context* parent_;
  ash::flat_set<context*> children_;
  time_point deadline_;
  flag cancelled_;
  static thread_local context* current_;
};  // namespace ash

namespace detail {}

}  // namespace ash

#endif  // ASH_CONTEXT_H_
