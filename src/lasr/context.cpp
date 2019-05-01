/// \file
/// \brief Execution context with deadlines and cancellation.
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

#include "lasr/context.h"
#include <utility>
#include "lasr/errors.h"

namespace lasr {

thread_local context* context::current_ = nullptr;

context& context::current() {
  if (!current_) return top();
  return *current_;
}

context& context::top() {
  static context base_context{root{}};
  return base_context;
}

context::context(context& parent, bool set_current, bool shield)
    : set_current_(set_current), parent_(&parent), deadline_(std::nullopt) {
  if (parent_) {
    if (!shield) {
      parent_->add_child(this);
    }
    deadline_ = parent_->deadline_;
    data_ = parent_->data_;
  }

  if (set_current_) {
    previous_ = current_;
    current_ = this;
  }
}

context::context(root)
    : set_current_(false),
      parent_(nullptr),
      previous_(nullptr),
      deadline_(std::nullopt) {}

context::~context() {
  if (parent_) parent_->remove_child(this);
  if (set_current_) current_ = previous_;

  cancel();

  std::unique_lock lock(children_mu_);
  child_detached_.wait(lock, [this]() { return children_.empty(); });
}

void context::add_child(context* child) {
  std::scoped_lock lock(children_mu_);
  children_.insert(child);
  if (cancelled_) {
    child->cancel();
  }
}

void context::remove_child(context* child) {
  {
    std::scoped_lock lock(children_mu_);
    children_.erase(child);
  }
  child_detached_.notify_one();
}

std::optional<context::time_point> context::deadline() const {
  std::scoped_lock lock(data_mu_);
  return deadline_;
}

std::optional<context::duration> context::deadline_left() const {
  std::scoped_lock lock(data_mu_);
  if (deadline_) {
    return std::chrono::duration_cast<duration>(
        *deadline_ - std::chrono::system_clock::now());
  } else {
    return std::nullopt;
  }
}

void context::cancel() {
  std::scoped_lock lock(children_mu_);
  for (auto* child : children_) {
    child->cancel();
  }
  cancelled_.set();
}

bool context::is_cancelled() const { return cancelled_.is_set(); }

awaitable<void> context::wait_cancelled() {
  return cancelled_.wait_set().then(
      []() { throw errors::cancelled("Context is cancelled"); });
}

awaitable<void> context::wait_deadline() {
  std::scoped_lock lock(data_mu_);
  if (deadline_) {
    return lasr::deadline(*deadline_).then([]() {
      throw errors::deadline_exceeded("Deadline exceeded");
    });
  } else {
    return lasr::never();
  }
}

void context::set_deadline(time_point when) {
  std::scoped_lock lock(data_mu_);
  if (!deadline_ || *deadline_ > when) {
    deadline_ = when;
  }
}

void context::set_timeout(duration timeout) {
  set_deadline(std::chrono::time_point_cast<context::duration>(
      std::chrono::system_clock::now() + timeout));
}

void context::reset_all() {
  std::scoped_lock lock(data_mu_);
  data_.clear();
}

std::vector<std::shared_ptr<const dynamic_base_class>> context::data() const {
  std::vector<std::shared_ptr<const dynamic_base_class>> res;
  res.reserve(data_.size());
  {
    std::scoped_lock lock(data_mu_);
    for (auto it : data_) {
      res.push_back(it.second);
    }
  }
  return res;
}

void context::set_data(
    std::vector<std::shared_ptr<dynamic_base_class>>&& new_data) {
  data_.clear();
  for (auto it : new_data) {
    data_[it->portable_class_name()] =
        std::static_pointer_cast<const dynamic_base_class>(std::move(it));
  }
}

shield::shield() : context(current(), true, true) {}

}  // namespace lasr
