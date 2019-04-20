/// \file
/// \brief select-friendly futures and promises.
///
/// \copyright
///   Copyright 2019 by Google LLC. All Rights Reserved.
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

#ifndef LASR_FUTURE_H_
#define LASR_FUTURE_H_

#include <exception>
#include <memory>
#include <mutex>
#include <optional>
#include <type_traits>
#include <utility>

#include "lasr/errors.h"
#include "lasr/flag.h"
#include "lasr/result_holder.h"
#include "lasr/select.h"
#include "function2/function2.hpp"

namespace lasr {
namespace detail {

class future_state_base {
 public:
  using releaser = void (*)(future_state_base*);

  virtual ~future_state_base();

  void release_reader();

  void release_writer();

  awaitable<void> can_get();

 protected:
  std::mutex mu_;
  bool has_writer_ = true;
  bool has_reader_ = true;
  flag set_;
};

template <typename T>
class future_state : public future_state_base {
 public:
  using value_type = T;

  template <typename U>
  void set_value(U&& u) {
    std::scoped_lock lock(mu_);
    result_.set_value(std::forward<U>(u));
    set_.set();
  }

  void set_exception(std::exception_ptr exception) {
    std::scoped_lock lock(mu_);
    result_.set_exception(exception);
    set_.set();
  }

  void set(result_holder<value_type>&& result) { result_ = std::move(result); }

  value_type maybe_get() {
    std::scoped_lock lock(mu_);
    if (result_) {
      set_.reset();
      return *std::move(result_);
    }
    throw errors::try_again("Future not ready yet");
  }

 private:
  result_holder<value_type> result_;
};

template <>
class future_state<void> : public future_state_base {
 public:
  using value_type = void;

  void set_value() {
    std::scoped_lock lock(mu_);
    result_.set_value();
    set_.set();
  }

  void set_exception(std::exception_ptr exception) {
    std::scoped_lock lock(mu_);
    result_.set_exception(exception);
    set_.set();
  }

  void set(result_holder<value_type>&& result) { result_ = std::move(result); }

  void maybe_get() {
    std::scoped_lock lock(mu_);
    if (result_) {
      set_.reset();
      *result_;
      return;
    }
    throw errors::try_again("Future not ready yet");
  }

 private:
  result_holder<value_type> result_;
};

}  // namespace detail

template <typename T>
class promise;

template <typename T>
class future {
 public:
  using value_type = T;

 private:
  using pointer_type =
      std::unique_ptr<detail::future_state_base,
                      typename detail::future_state_base::releaser>;
  using get_fn_type =
      fu2::unique_function<value_type(detail::future_state_base&)>;

 public:
  future(future<value_type>&&) = default;
  future<value_type>& operator=(future<value_type>&&) = default;

  future()
      : state_(nullptr,
               [](detail::future_state_base* s) { s->release_reader(); }),
        get_fn_(std::move([this](detail::future_state_base& s) {
          return static_cast<detail::future_state<value_type>&>(s).maybe_get();
        })) {}

  value_type maybe_get() { return get_fn_(state()); }

  awaitable<void> can_get() { return state().can_get(); }

  awaitable<value_type> async_get() {
    return std::move(
        can_get().then(std::move([this]() { return std::move(maybe_get()); })));
  }

  value_type get() {
    auto [res] = select(async_get());
    return std::move(*res);
  }

  template <typename OGF>
  auto then(OGF&& get_fn) {
    using new_value_type = std::invoke_result_t<OGF, value_type>;
    auto new_get_fn = [outer(std::move(get_fn)), inner(std::move(get_fn_))](
                          detail::future_state_base& state) mutable {
      if constexpr (std::is_same_v<new_value_type, void>) {
        outer(inner(state));
      } else {
        return outer(inner(state));
      }
    };
    return future<new_value_type>(std::move(state_), std::move(new_get_fn));
  }

 private:
  friend class promise<value_type>;
  template <typename U>
  friend class future;

  detail::future_state_base& state() {
    if (!state_) throw errors::invalid_state("Empty future");
    return *state_;
  }

  explicit future(detail::future_state<value_type>* new_state)
      : state_(new_state,
               [](detail::future_state_base* s) { s->release_reader(); }),
        get_fn_(std::move([this](detail::future_state_base& s) {
          return static_cast<detail::future_state<value_type>&>(s).maybe_get();
        })) {}

  future(pointer_type new_state, get_fn_type new_get_fn)
      : state_(std::move(new_state)), get_fn_(std::move(new_get_fn)) {}

  pointer_type state_;
  get_fn_type get_fn_;
};

template <>
class future<void> {
 public:
  using value_type = void;

 private:
  using pointer_type =
      std::unique_ptr<detail::future_state_base,
                      typename detail::future_state_base::releaser>;
  using get_fn_type =
      fu2::unique_function<value_type(detail::future_state_base&)>;

 public:
  future(future<value_type>&&) = default;
  future<value_type>& operator=(future<value_type>&&) = default;

  future()
      : state_(nullptr,
               [](detail::future_state_base* s) { s->release_reader(); }),
        get_fn_(std::move([this](detail::future_state_base& s) {
          static_cast<detail::future_state<value_type>&>(s).maybe_get();
        })) {}

  value_type maybe_get() { get_fn_(state()); }

  awaitable<void> can_get() { return state().can_get(); }

  awaitable<value_type> async_get() {
    return std::move(can_get().then(std::move([this]() { maybe_get(); })));
  }

  value_type get() {
    auto [res] = select(async_get());
    *res;
  }

  template <typename OGF>
  auto then(OGF&& get_fn) {
    using new_value_type = std::invoke_result_t<OGF>;
    auto new_get_fn = [outer(std::move(get_fn)), inner(std::move(get_fn_))](
                          detail::future_state_base& state) mutable {
      inner(state);
      if constexpr (std::is_same_v<new_value_type, void>) {
        outer();
      } else {
        return outer();
      }
    };
    return future<new_value_type>(std::move(state_), std::move(new_get_fn));
  }

 private:
  friend class promise<value_type>;
  template <typename U>
  friend class future;

  detail::future_state_base& state() {
    if (!state_) throw errors::invalid_state("Empty future");
    return *state_;
  }

  explicit future(detail::future_state<value_type>* new_state)
      : state_(new_state,
               [](detail::future_state_base* s) { s->release_reader(); }),
        get_fn_(std::move([this](detail::future_state_base& s) {
          static_cast<detail::future_state<value_type>&>(s).maybe_get();
        })) {}

  future(pointer_type new_state, get_fn_type new_get_fn)
      : state_(std::move(new_state)), get_fn_(std::move(new_get_fn)) {}

  pointer_type state_;
  get_fn_type get_fn_;
};

template <typename T>
class promise {
 public:
  using value_type = T;

  promise(promise<value_type>&&) = default;
  promise<value_type>& operator=(promise<value_type>&&) = default;

  promise()
      : state_(
            new detail::future_state<value_type>(),
            [](detail::future_state_base* state) { state->release_writer(); }),
        future_(state_.get()) {}

  ~promise() {
    if (state_) {
      state()->set_exception(
          std::make_exception_ptr(errors::invalid_state("Broken promise")));
    }
  }

  future<value_type> get_future() { return std::move(future_); }

  template <typename U>
  void set_value(U&& u) {
    state()->set_value(std::forward<U>(u));
  }

  void set_exception(std::exception_ptr exc) { state()->set_exception(exc); }

  void set(result_holder<value_type>&& result) {
    state()->set(std::move(result));
  }

 private:
  using pointer_type =
      std::unique_ptr<detail::future_state<value_type>,
                      typename detail::future_state<value_type>::releaser>;

  pointer_type state() {
    if (!state_) throw errors::invalid_state("Promise already set");
    return std::move(state_);
  }

  pointer_type state_;
  future<value_type> future_;
};

template <>
class promise<void> {
 public:
  using value_type = void;

  promise(promise<value_type>&&) = default;
  promise<value_type>& operator=(promise<value_type>&&) = default;

  promise()
      : state_(
            new detail::future_state<value_type>(),
            [](detail::future_state_base* state) { state->release_writer(); }),
        future_(state_.get()) {}

  ~promise() {
    if (state_) {
      state()->set_exception(
          std::make_exception_ptr(errors::invalid_state("Broken promise")));
    }
  }

  future<value_type> get_future() { return std::move(future_); }

  void set_value() { state()->set_value(); }

  void set_exception(std::exception_ptr exc) { state()->set_exception(exc); }

 private:
  using pointer_type =
      std::unique_ptr<detail::future_state<value_type>,
                      typename detail::future_state<value_type>::releaser>;

  pointer_type state() {
    if (!state_) throw errors::invalid_state("Promise already set");
    return std::move(state_);
  }

  pointer_type state_;
  future<value_type> future_;
};

}  // namespace lasr

#endif  // LASR_FUTURE_H_
