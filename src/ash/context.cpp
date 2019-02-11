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

#include "ash/context.h"
#include <algorithm>

namespace ash {

thread_local context* context::current_ = nullptr;

context& context::current() {
  static context base_context;
  return *current_;
}

context::context(time_point deadline) : parent_(current_), deadline_(deadline) {
  if (parent_) {
    parent_->add_child(this);

    deadline_ = std::min(deadline_, parent_->deadline_);
  }

  current_ = this;
}

context::~context() {
  current_ = parent_;

  if (parent_) parent_->remove_child(this);
}

void context::add_child(context* child) {
  std::scoped_lock lock(mu_);
  children_.insert(child);
  if (cancelled_) {
    child->cancel();
  }
}

void context::remove_child(context* child) {
  std::scoped_lock lock(mu_);
  children_.erase(child);
}

context::time_point context::deadline() { return deadline_; }

context::duration context::deadline_left() {
  return deadline_ - std::chrono::system_clock::now();
}

void context::cancel() {
  std::scoped_lock lock(mu_);
  cancelled_.set();
  for (auto* child : children_) {
    child->cancel();
  }
}

bool context::is_cancelled() { return cancelled_.is_set(); }

awaitable<void> context::wait_cancelled() { return cancelled_.wait_set(); }

context context::with_deadline(time_point when) { return context(when); }

context context::with_timeout(duration timeout) {
  return context(std::chrono::system_clock::now() + timeout);
}

context context::with_cancel() { return context(); }

}  // namespace ash
