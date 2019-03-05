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
#include <utility>
#include "ash/errors.h"

namespace ash {

thread_local context* context::current_ = nullptr;

context& context::current() {
  if (!current_) return top();
  return *current_;
}

context& context::top() {
  static context base_context{nullptr, time_point::max(), false};
  return base_context;
}

context::context(context* parent, time_point deadline, bool set_current)
    : set_current_(set_current), parent_(parent), deadline_(deadline) {
  if (parent_) {
    parent_->add_child(this);

    deadline_ = std::min(deadline_, parent_->deadline_);
    data_ = parent_->data_;
  }

  if (set_current_) current_ = this;
}

context::~context() {
  if (set_current_) current_ = parent_;

  cancel();

  std::unique_lock lock(mu_);
  child_detached_.wait(lock, [this]() { return children_.empty(); });

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
  {
    std::scoped_lock lock(mu_);
    children_.erase(child);
  }
  child_detached_.notify_one();
}

context::time_point context::deadline() const { return deadline_; }

context::duration context::deadline_left() const {
  return deadline_ - std::chrono::system_clock::now();
}

void context::cancel() {
  std::scoped_lock lock(mu_);
  cancelled_.set();
  for (auto* child : children_) {
    child->cancel();
  }
}

bool context::is_cancelled() const { return cancelled_.is_set(); }

awaitable<void> context::wait_cancelled() {
  return cancelled_.wait_set().then(
      std::move([]() { throw errors::cancelled("Context is cancelled"); }));
}

awaitable<void> context::wait_deadline() {
  return ash::deadline(deadline_).then(std::move(
      []() { throw errors::deadline_exceeded("Deadline exceeded"); }));
}

context context::with_deadline(time_point when) {
  return context(current_, when);
}

context context::with_timeout(duration timeout) {
  return context(current_, std::chrono::system_clock::now() + timeout);
}

context context::with_cancel() { return context(); }

context context::make_child() { return context(this); }

}  // namespace ash
