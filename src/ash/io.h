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
#include <string>
#include <utility>

namespace ash {

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
  awaitable read();
  awaitable write();

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

template <typename... Args>
std::array<bool, sizeof...(Args)> select(const Args&... args);

class awaitable {
 public:
  using checker_fn_type = std::function<bool()>;
  using finish_fn_type = std::function<void()>;

  explicit awaitable(const channel& ch, bool for_write = false,
                     checker_fn_type checker = nullptr,
                     finish_fn_type finish = nullptr);
  explicit awaitable(std::chrono::milliseconds timeout,
                     checker_fn_type checker = nullptr,
                     finish_fn_type finish = nullptr);
  ~awaitable();

  checker_fn_type get_checker() const;
  int get_fd() const;
  bool for_write() const;
  std::chrono::milliseconds timeout() const;

 private:
  const checker_fn_type checker_;
  const finish_fn_type finish_;
  const int fd_ = -1;
  const bool for_write_;
  const std::chrono::milliseconds timeout_ = std::chrono::milliseconds(-1);
};

template <typename Rep, typename Period>
awaitable timeout(const std::chrono::duration<Rep, Period>& duration) {
  return awaitable(
      std::chrono::duration_cast<std::chrono::milliseconds>(duration));
}

template <typename Clock, typename Duration>
awaitable deadline(const std::chrono::time_point<Clock, Duration>& when) {
  std::chrono::milliseconds delta =
      std::chrono::duration_cast<std::chrono::milliseconds>(
          when - std::chrono::system_clock::now());
  return awaitable(std::max(std::chrono::milliseconds::zero(), delta));
}

template <typename... Args>
std::array<bool, sizeof...(Args)> select(const Args&... args) {
  constexpr std::size_t n = sizeof...(Args);

  pollfd fds[n] = {
      {.fd = args.get_fd(),
       .events = static_cast<short>(/* NOLINT(runtime/int) */ args.for_write()
                                        ? (POLLOUT | POLLERR)
                                        : (POLLIN | POLLHUP))}...};

  std::chrono::milliseconds timeouts[n] = {args.timeout()...};

  awaitable::checker_fn_type checkers[n] = {args.get_checker()...};

  std::chrono::milliseconds timeout = std::chrono::milliseconds(-1);
  constexpr auto zero = std::chrono::milliseconds ::zero();
  for (std::size_t i = 0; i < n; i++) {
    if (timeouts[i] >= zero && (timeout < zero || timeout > timeouts[i])) {
      timeout = timeouts[i];
    }
  }

  std::chrono::milliseconds elapsed = zero;
  auto last = std::chrono::steady_clock::now();

  do {
    int pres = poll(fds, n, (timeout - elapsed) / std::chrono::milliseconds(1));
    if (pres < 0) detail::throw_io_error("Error in select");

    std::array<bool, n> res;
    if (pres > 0) {
      for (std::size_t i = 0; i < n; i++) {
        res[i] = (fds[i].revents & fds[i].events) != 0 &&
                 (!checkers[i] || checkers[i]());
      }
    } else {
      for (std::size_t i = 0; i < n; i++) {
        res[i] = timeouts[i] >= zero && timeouts[i] <= timeout &&
                 (!checkers[i] || checkers[i]());
      }
    }
    for (std::size_t i = 0; i < n; i++) {
      if (res[i]) {
        return res;
      }
    }

    auto now = std::chrono::steady_clock::now();
    elapsed +=
        std::chrono::duration_cast<std::chrono::milliseconds>(now - last);
    last = now;
  } while (true);
}

}  // namespace ash

#endif  // ASH_IO_H_
