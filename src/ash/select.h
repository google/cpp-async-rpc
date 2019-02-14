/// \file
/// \brief Reactor support implemented as select().
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

#ifndef ASH_SELECT_H_
#define ASH_SELECT_H_

#include <poll.h>
#include <algorithm>
#include <array>
#include <chrono>
#include <cstddef>
#include <optional>
#include <tuple>
#include <type_traits>
#include <utility>
#include "ash/awaitable.h"
#include "ash/errors.h"
#include "ash/mpt.h"

namespace ash {

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
    if (pres < 0) throw_io_error("Error in select");

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

#endif  // ASH_SELECT_H_
