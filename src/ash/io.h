/// \channel
/// \brief Channel descriptor wrapper.
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

#ifndef ASH_IO_H_
#define ASH_IO_H_

#include <fcntl.h>
#include <poll.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include <algorithm>
#include <array>
#include <chrono>
#include <functional>
#include <iostream>
#include <optional>
#include <string>
#include <tuple>
#include <type_traits>
#include <utility>
#include "ash/errors.h"
#include "ash/mpt.h"
#include "function2/function2.hpp"

namespace ash {

template <typename R>
class awaitable;

enum class open_mode : int {
  READ,
  WRITE,
  APPEND,
  READ_PLUS,
  WRITE_PLUS,
  APPEND_PLUS,
};

namespace detail {

void throw_io_error(const std::string& message);

}  // namespace detail

class channel {
 public:
  channel() noexcept;
  explicit channel(int fd) noexcept;
  channel(channel&& fd) noexcept;
  ~channel() noexcept;
  void swap(channel& fd) noexcept;
  channel& operator=(channel&& fd) noexcept;
  int release() noexcept;
  int get() const noexcept;
  int operator*() const noexcept;
  void reset(int fd = -1) noexcept;
  explicit operator bool() const noexcept;
  void close() noexcept;
  std::size_t read(void* buf, std::size_t len);
  std::size_t write(const void* buf, std::size_t len);
  void make_blocking();
  void make_non_blocking();
  channel dup() const;
  awaitable<void> read();
  awaitable<void> write();

 private:
  int fd_;
};

bool operator==(const channel& a, const channel& b);
bool operator!=(const channel& a, const channel& b);
bool operator<(const channel& a, const channel& b);
bool operator<=(const channel& a, const channel& b);
bool operator>(const channel& a, const channel& b);
bool operator>=(const channel& a, const channel& b);

void pipe(channel fds[2]);

channel file(const std::string& path, open_mode mode = open_mode::READ);

template <typename R>
class awaitable {
 public:
  using return_type = R;

  using react_fn_type = fu2::unique_function<return_type()>;

  explicit awaitable(const channel& ch, bool for_write = false) : fd_(*ch), for_write_(for_write) {}

  explicit awaitable(std::chrono::milliseconds timeout, bool for_polling = false)
      : timeout_(timeout), for_polling_(for_polling) {}

  awaitable(const awaitable&) = delete;
  awaitable(awaitable&&) = default;

  ~awaitable() {}

  template <typename ORF>
  auto then(ORF&& react_fn) {
    if constexpr (std::is_same_v<return_type, void>) {
      using new_return_type = std::invoke_result_t<ORF>;
      auto new_react_fn = [outer(std::move(react_fn)), inner(std::move(react_fn_))]() mutable {
        inner();
        if constexpr (std::is_same_v<new_return_type, void>) {
          outer();
        } else {
          return std::move(outer());
        }
      };
      return std::move(awaitable<new_return_type>(std::move(*this), std::move(new_react_fn)));
    } else {
      using new_return_type = std::invoke_result_t<ORF, return_type>;
      auto new_react_fn = [outer(std::move(react_fn)), inner(std::move(react_fn_))]() mutable {
        if constexpr (std::is_same_v<new_return_type, void>) {
          outer(std::move(inner()));
        } else {
          return std::move(outer(std::move(inner())));
        }
      };
      return std::move(awaitable<new_return_type>(std::move(*this), std::move(new_react_fn)));
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

template <typename Rep, typename Period>
awaitable<void> timeout(const std::chrono::duration<Rep, Period>& duration) {
  return awaitable<void>(std::chrono::duration_cast<std::chrono::milliseconds>(duration));
}

template <typename Clock, typename Duration>
awaitable<void> deadline(const std::chrono::time_point<Clock, Duration>& when) {
  std::chrono::milliseconds delta = std::chrono::duration_cast<std::chrono::milliseconds>(
      when - std::chrono::system_clock::now());
  return awaitable<void>(std::max(std::chrono::milliseconds::zero(), delta));
}

namespace detail {

template <typename T>
struct select_return_type {
  using type = std::optional<T>;
};

template <>
struct select_return_type<void> {
  using type = bool;
};

template <typename T>
using select_return_type_t = typename select_return_type<T>::type;

template <typename... Args, std::size_t... ints>
std::array<pollfd, sizeof...(Args)> make_select_pollfds(std::tuple<Args...>& awaitables,
                                                        mpt::index_sequence<ints...>) {
  return {pollfd{mpt::at<ints>(awaitables).get_fd(),
                 static_cast<short> /* NOLINT(runtime/int) */ (mpt::at<ints>(awaitables).for_write()
                                                                   ? (POLLOUT | POLLERR)
                                                                   : (POLLIN | POLLHUP)),
                 0}...};
}

template <typename A>
typename select_return_type<typename A::return_type>::type make_one_select_result(
    A& a, const pollfd& fd, bool was_timeout, std::chrono::milliseconds min_timeout,
    bool min_timeout_is_polling) {
  bool active;
  if (was_timeout) {
    active = (a.timeout() >= std::chrono::milliseconds::zero() &&
              (a.timeout() <= min_timeout || (min_timeout_is_polling && a.for_polling())));
  } else {
    active = ((fd.revents & fd.events) != 0);
  }

  if (active) {
    try {
      if constexpr (std::is_same_v<typename A::return_type, void>) {
        a.get_react_fn()();
        return true;
      } else {
        return {std::move(a.get_react_fn()())};
      }
    } catch (const errors::try_again&) {
    }
  }
  return {};
}

template <typename... Args, std::size_t... ints>
std::tuple<typename select_return_type<typename Args::return_type>::type...> make_select_result(
    std::tuple<Args...>& awaitables, const std::array<pollfd, sizeof...(Args)>& pollfds,
    bool was_timeout, std::chrono::milliseconds min_timeout, bool min_timeout_is_polling,
    mpt::index_sequence<ints...>) {
  return {make_one_select_result(mpt::at<ints>(awaitables), pollfds[ints], was_timeout, min_timeout,
                                 min_timeout_is_polling)...};
}

}  // namespace detail

template <typename... Args>
auto select(Args&&... args) {
  constexpr std::size_t n = sizeof...(Args);
  auto a = std::make_tuple(std::forward<Args>(args)...);
  std::chrono::milliseconds elapsed = std::chrono::milliseconds::zero();
  auto last = std::chrono::steady_clock::now();

  do {
    // Find whether we have a timeout to apply, and whether it's a polling one.
    auto [min_timeout, min_timeout_is_polling] = mpt::accumulate(
        std::pair(std::chrono::milliseconds(-1), false), a, [elapsed](auto prev, const auto& a) {
          auto& [min_timeout, min_timeout_is_polling] = prev;

          auto timeout = a.timeout();
          bool is_polling = a.for_polling();

          if (timeout >= std::chrono::milliseconds::zero()) {
            if (is_polling) {
              timeout = std::max(timeout - elapsed, std::chrono::milliseconds::zero());
            }
            if (min_timeout < std::chrono::milliseconds::zero() || timeout < min_timeout) {
              min_timeout = timeout;
              min_timeout_is_polling = is_polling;
            }
          }

          return std::pair(min_timeout, min_timeout_is_polling);
        });

    auto fds(detail::make_select_pollfds(a, mpt::make_index_sequence<n>{}));

    int pres = poll(fds.data(), n, min_timeout / std::chrono::milliseconds(1));
    if (pres < 0) detail::throw_io_error("Error in select");

    auto res(detail::make_select_result(a, fds, pres == 0, min_timeout, min_timeout_is_polling,
                                        mpt::make_index_sequence<n>{}));

    bool active =
        mpt::accumulate(false, res, [](bool so_far, const auto& val) { return so_far || val; });
    if (active) {
      return res;
    }

    auto now = std::chrono::steady_clock::now();
    elapsed += std::chrono::duration_cast<std::chrono::milliseconds>(now - last);
    last = now;
  } while (true);
}

}  // namespace ash

#endif  // ASH_IO_H_
