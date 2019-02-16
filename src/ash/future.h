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

#ifndef ASH_FUTURE_H_
#define ASH_FUTURE_H_

#include <exception>
#include <memory>
#include <mutex>
#include <optional>
#include <utility>
#include "ash/flag.h"
#include "ash/io.h"

namespace ash {
namespace detail {

template <typename T>
class future_state {
 public:
  using value_type = T;
  using releaser = void (*)(future_state<T>*);

  void release_reader() {
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

  void release_writer() {
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

  template <typename U>
  void set_value(U&& u) {
    std::scoped_lock lock(mu_);
    result_ = std::forward<U>(u);
    set_.set();
  }

  void set_exception(std::exception_ptr exc) {
    std::scoped_lock lock(mu_);
    exception_ = exc;
    set_.set();
  }

  value_type maybe_get() {
    std::scoped_lock lock(mu_);
    if (result_) {
      set_.reset();
      return std::move(*result_);
    }
    if (exception_) {
      set_.reset();
      std::rethrow_exception(exception_);
    }
    throw errors::try_again("Future not ready yet");
  }

  awaitable<void> can_get() { return set_.wait_set(); }

  awaitable<value_type> async_get() {
    return std::move(
        can_get().then(std::move([this]() { return std::move(maybe_get()); })));
  }

  value_type get() {
    auto [res] = select(async_get());
    return std::move(*res);
  }

 private:
  std::mutex mu_;
  bool has_writer_ = true;
  bool has_reader_ = true;
  flag set_;
  std::optional<T> result_;
  std::exception_ptr exception_;
};

}  // namespace detail

template <typename T>
class promise;

template <typename T>
class future {
 public:
  using value_type = T;

  future()
      : state_(nullptr,
               [](detail::future_state<T>* s) { s->release_reader(); }) {}

  value_type maybe_get() { return state()->maybe_get(); }

  awaitable<void> can_get() { return state()->can_get(); }

  awaitable<value_type> async_get() { return state()->async_get(); }

  value_type get() { return state()->get(); }

 private:
  friend class promise<T>;

  auto& state() {
    if (!state_) throw errors::invalid_state("Empty future");
    return state_;
  }

  explicit future(detail::future_state<T>* state)
      : state_(state, [](detail::future_state<T>* s) { s->release_reader(); }) {
  }

  std::unique_ptr<detail::future_state<T>,
                  typename detail::future_state<T>::releaser>
      state_;
};  // namespace ash

template <typename T>
class promise {
 public:
  using value_type = T;

  promise()
      : state_(new detail::future_state<T>(),
               [](detail::future_state<T>* state) { state->release_writer(); }),
        future_(state_.get()) {}

  ~promise() {
    if (state_) {
      state()->set_exception(
          std::make_exception_ptr(errors::invalid_state("Broken promise")));
    }
  }

  future<T> get_future() { return std::move(future_); }

  template <typename U>
  void set_value(U&& u) {
    state()->set_value(std::forward<U>(u));
  }

  void set_exception(std::exception_ptr exc) { state()->set_exception(exc); }

 private:
  auto state() {
    if (!state_) throw errors::invalid_state("Promise already set");
    return std::move(state_);
  }

  std::unique_ptr<detail::future_state<T>,
                  typename detail::future_state<T>::releaser>
      state_;
  future<T> future_;
};
}  // namespace ash

#endif  // ASH_FUTURE_H_
