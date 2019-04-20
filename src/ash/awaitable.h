/// \file
/// \brief Awaitable primitive.
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

#ifndef LASR_AWAITABLE_H_
#define LASR_AWAITABLE_H_

#include <chrono>
#include <type_traits>
#include <utility>
#include "function2/function2.hpp"

namespace ash {

template <typename R>
class awaitable {
 public:
  using return_type = R;

 private:
  using react_fn_type = fu2::unique_function<return_type()>;

 public:
  explicit awaitable(int fd, bool for_write = false)
      : fd_(fd), for_write_(for_write) {}

  explicit awaitable(std::chrono::milliseconds timeout,
                     bool for_polling = false)
      : timeout_(timeout), for_polling_(for_polling) {}

  awaitable(const awaitable&) = delete;
  awaitable(awaitable&&) = default;

  ~awaitable() {}

  template <typename ORF>
  auto then(ORF&& react_fn) {
    if constexpr (std::is_same_v<return_type, void>) {
      using new_return_type = std::invoke_result_t<ORF>;
      auto new_react_fn = [outer(std::move(react_fn)),
                           inner(std::move(react_fn_))]() mutable {
        inner();
        if constexpr (std::is_same_v<new_return_type, void>) {
          outer();
        } else {
          return outer();
        }
      };
      return awaitable<new_return_type>(std::move(*this),
                                        std::move(new_react_fn));
    } else {
      using new_return_type = std::invoke_result_t<ORF, return_type>;
      auto new_react_fn = [outer(std::move(react_fn)),
                           inner(std::move(react_fn_))]() mutable {
        if constexpr (std::is_same_v<new_return_type, void>) {
          outer(inner());
        } else {
          return outer(inner());
        }
      };
      return awaitable<new_return_type>(std::move(*this),
                                        std::move(new_react_fn));
    }
  }

  react_fn_type& get_react_fn() { return react_fn_; }
  int get_fd() const { return fd_; }
  bool for_write() const { return for_write_; }
  std::chrono::milliseconds timeout() const { return timeout_; }
  bool for_polling() const { return for_polling_; }

 private:
  template <typename OR>
  friend class awaitable;

  template <typename OR>
  awaitable(awaitable<OR>&& old, react_fn_type&& react_fn)
      : react_fn_(std::move(react_fn)),
        fd_(old.fd_),
        for_write_(old.for_write_),
        timeout_(std::move(old.timeout_)),
        for_polling_(old.for_polling_) {}

  react_fn_type react_fn_ = []() { return; };
  const int fd_ = -1;
  const bool for_write_ = false;
  const std::chrono::milliseconds timeout_ = std::chrono::milliseconds(-1);
  const bool for_polling_ = false;
};

awaitable<void> never();
awaitable<void> always();

template <typename Rep, typename Period>
awaitable<void> timeout(const std::chrono::duration<Rep, Period>& duration) {
  return awaitable<void>(
      std::chrono::duration_cast<std::chrono::milliseconds>(duration));
}

template <typename Rep, typename Period>
awaitable<void> polling(const std::chrono::duration<Rep, Period>& duration) {
  return awaitable<void>(
      std::chrono::duration_cast<std::chrono::milliseconds>(duration), true);
}

template <typename Clock, typename Duration>
awaitable<void> deadline(const std::chrono::time_point<Clock, Duration>& when) {
  std::chrono::milliseconds delta =
      std::chrono::duration_cast<std::chrono::milliseconds>(
          when - std::chrono::system_clock::now());
  return awaitable<void>(std::max(std::chrono::milliseconds::zero(), delta));
}

}  // namespace ash

#endif  // LASR_AWAITABLE_H_
