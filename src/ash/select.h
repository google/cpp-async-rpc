/// \file
/// \brief Reactor support implemented as select().
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

#ifndef ASH_SELECT_H_
#define ASH_SELECT_H_

#include <poll.h>
#include <algorithm>
#include <array>
#include <chrono>
#include <cstddef>
#include <exception>
#include <tuple>
#include <type_traits>
#include <utility>
#include <vector>
#include "ash/awaitable.h"
#include "ash/context.h"
#include "ash/errors.h"
#include "ash/mpt.h"
#include "ash/result_holder.h"

namespace ash {

namespace detail {

template <typename A>
result_holder<typename A::return_type> make_one_select_result(A& a, const pollfd& fd,
                                                              bool was_timeout,
                                                              std::chrono::milliseconds min_timeout,
                                                              bool min_timeout_is_polling) {
  bool active;
  if (was_timeout) {
    active = (a.timeout() >= std::chrono::milliseconds::zero() &&
              (a.timeout() <= min_timeout || (min_timeout_is_polling && a.for_polling())));
  } else {
    active = ((fd.revents & fd.events) != 0);
  }

  result_holder<typename A::return_type> res;

  if (active) {
    try {
      if constexpr (std::is_same_v<typename A::return_type, void>) {
        a.get_react_fn()();
        res.set_value();
      } else {
        res.set_value(std::move(a.get_react_fn()()));
      }
    } catch (const errors::try_again&) {
    } catch (...) {
      res.set_exception(std::current_exception());
    }
  }

  return res;
}

template <typename T>
constexpr pollfd make_pollfd(const T& awaitable) {
  return {awaitable.get_fd(),
          static_cast<short> /* NOLINT(runtime/int) */ (awaitable.for_write() ? (POLLOUT | POLLERR)
                                                                              : (POLLIN | POLLHUP)),
          0};
}

template <typename T>
struct select_input_helper;

template <typename T>
struct select_input_helper<awaitable<T>> {
  using result_type = result_holder<T>;
  static constexpr bool has_static_size = true;
  static constexpr std::size_t size(const awaitable<T>&) { return 1; }
  static result_type make_result(awaitable<T>& a, const pollfd* fd, bool was_timeout,
                                 std::chrono::milliseconds min_timeout,
                                 bool min_timeout_is_polling) {
    return make_one_select_result(a, *fd, was_timeout, min_timeout, min_timeout_is_polling);
  }
  static void fill_fds(const awaitable<T>& a, pollfd* fds) { *fds = make_pollfd(a); }
  static auto timeout_info(const awaitable<T>& a, std::chrono::milliseconds elapsed) {
    auto timeout = a.timeout();
    bool is_polling = a.for_polling();

    if (timeout >= std::chrono::milliseconds::zero()) {
      if (is_polling) {
        timeout = std::max(timeout - elapsed, std::chrono::milliseconds::zero());
      }
    }

    return std::pair(timeout, is_polling);
  }
};

template <typename T>
struct select_input_helper<std::vector<awaitable<T>>> {
  using result_type = std::vector<result_holder<T>>;
  static constexpr bool has_static_size = false;
  static constexpr std::size_t size(const std::vector<awaitable<T>>& v) { return v.size(); }
  static result_type make_result(std::vector<awaitable<T>>& v, const pollfd* fd, bool was_timeout,
                                 std::chrono::milliseconds min_timeout,
                                 bool min_timeout_is_polling) {
    result_type res;
    res.reserve(v.size());
    for (auto& a : v) {
      res.push_back(
          make_one_select_result(a, *fd++, was_timeout, min_timeout, min_timeout_is_polling));
    }
    return res;
  }
  static void fill_fds(const std::vector<awaitable<T>>& v, pollfd* fds) {
    for (const auto& a : v) {
      *fds++ = make_pollfd(a);
    }
  }
  static auto timeout_info(const std::vector<awaitable<T>>& v, std::chrono::milliseconds elapsed) {
    auto min_timeout = std::chrono::milliseconds(-1);
    bool min_timeout_is_polling = false;

    for (const auto& a : v) {
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
    }

    return std::pair(min_timeout, min_timeout_is_polling);
  }
};

template <typename T>
using select_input = select_input_helper<std::remove_cv_t<std::remove_reference_t<T>>>;

template <typename T>
struct select_output_helper;

template <typename T>
struct select_output_helper<result_holder<T>> {
  using result_type = result_holder<T>;
  static bool is_active(const result_type& r) { return r.has_value(); }
};

template <typename T>
struct select_output_helper<std::vector<result_holder<T>>> {
  using result_type = std::vector<result_holder<T>>;
  static bool is_active(const result_type& v) {
    for (const auto& r : v) {
      if (r) return true;
    }
    return false;
  }
};

template <typename T>
using select_output = select_output_helper<std::remove_cv_t<std::remove_reference_t<T>>>;

template <typename... Args, std::size_t... ints>
constexpr std::size_t pollfds_index_helper(const std::tuple<Args...>& awaitables,
                                           mpt::index_sequence<ints...>) {
  return (0 + ... +
          select_input<mpt::element_type_t<ints, std::tuple<Args...>>>::size(
              mpt::at<ints>(awaitables)));
}

template <std::size_t idx, typename... Args>
constexpr std::size_t pollfds_index(const std::tuple<Args...>& awaitables) {
  return pollfds_index_helper(awaitables, mpt::make_index_sequence<idx>{});
}

template <typename... Args>
constexpr bool select_inputs_have_static_size(const std::tuple<Args...>& awaitables) {
  return (... && select_input<Args>::has_static_size);
}

template <typename... Args>
constexpr std::size_t select_inputs_static_size(const std::tuple<Args...>& awaitables) {
  return (0 + ... + select_input<Args>::static_size);
}
template <typename... Args, std::size_t... ints>
std::tuple<typename select_input<Args>::result_type...> make_select_result(
    std::tuple<Args...>& awaitables, const pollfd* pollfds, bool was_timeout,
    std::chrono::milliseconds min_timeout, bool min_timeout_is_polling,
    mpt::index_sequence<ints...>) {
  return {std::move(select_input<Args>::make_result(
      mpt::at<ints>(awaitables), pollfds + pollfds_index<ints>(awaitables), was_timeout,
      min_timeout, min_timeout_is_polling))...};
}

template <typename... Args, std::size_t... ints>
void make_select_pollfds_helper(const std::tuple<Args...>& awaitables, pollfd* fds,
                                mpt::index_sequence<ints...>) {
  (..., select_input<mpt::element_type_t<ints, std::tuple<Args...>>>::fill_fds(
            mpt::at<ints>(awaitables), fds + pollfds_index<ints>(awaitables)));
}

template <typename... Args>
auto make_select_pollfds(const std::tuple<Args...>& awaitables) {
  if constexpr (select_inputs_have_static_size(awaitables)) {
    constexpr std::size_t size = pollfds_index<sizeof...(Args)>(awaitables);
    std::array<pollfd, size> res;
    make_select_pollfds_helper(awaitables, res.data(), mpt::make_index_sequence<sizeof...(Args)>{});
    return res;
  } else {
    std::size_t size = pollfds_index<sizeof...(Args)>(awaitables);
    std::vector<pollfd> res(size);
    make_select_pollfds_helper(awaitables, res.data(), mpt::make_index_sequence<sizeof...(Args)>{});
    return res;
  }
}

}  // namespace detail

template <typename... Args>
auto select(Args&&... args) {
  auto& current_context = context::current();

  constexpr std::size_t n = sizeof...(Args) + 2;
  auto a = std::tuple<Args..., awaitable<void>, awaitable<void>>(std::forward<Args>(args)...,
                                                                 current_context.wait_cancelled(),
                                                                 current_context.wait_deadline());

  std::chrono::milliseconds elapsed = std::chrono::milliseconds::zero();
  auto last = std::chrono::steady_clock::now();

  do {
    // Find whether we have a timeout to apply, and whether it's a polling one.
    auto [min_timeout, min_timeout_is_polling] = mpt::accumulate(
        std::pair(std::chrono::milliseconds(-1), false), a, [elapsed](auto prev, const auto& a) {
          auto& [min_timeout, min_timeout_is_polling] = prev;
          auto [timeout, is_polling] = detail::select_input<decltype(a)>::timeout_info(a, elapsed);

          if (timeout >= std::chrono::milliseconds::zero()) {
            if (min_timeout < std::chrono::milliseconds::zero() || timeout < min_timeout) {
              min_timeout = timeout;
              min_timeout_is_polling = is_polling;
            }
          }

          return std::pair(min_timeout, min_timeout_is_polling);
        });

    auto fds(detail::make_select_pollfds(a));

    int pres = poll(fds.data(), n, min_timeout / std::chrono::milliseconds(1));
    if (pres < 0) throw_io_error("Error in select");

    auto res(detail::make_select_result(a, fds.data(), pres == 0, min_timeout,
                                        min_timeout_is_polling, mpt::make_index_sequence<n>{}));

    bool active = mpt::accumulate(false, res, [](bool so_far, const auto& val) {
      return so_far || detail::select_output<decltype(val)>::is_active(val);
    });
    if (active) {
      auto [cancelled, deadline_exceeded] = mpt::range<n - 2, n>(res);
      if (cancelled) *cancelled;
      if (deadline_exceeded) *deadline_exceeded;
      return mpt::range<0, n - 2>(std::move(res));
    }

    auto now = std::chrono::steady_clock::now();
    elapsed += std::chrono::duration_cast<std::chrono::milliseconds>(now - last);
    last = now;
  } while (true);
}

}  // namespace ash

#endif  // ASH_SELECT_H_
